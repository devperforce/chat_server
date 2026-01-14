#include "pch.h"

#include "network/session.h"
#include "network/handler/handler.h"

namespace dev::network {

Session::Session(
    boost::asio::io_context& io_context,
    boost::asio::ip::tcp::socket&& socket,
    const utility::ILogger& logger,
    const PacketHandler& packet_handler)
    : ActorStrand(io_context.get_executor()),
    socket_(std::move(socket)),
    logger_(logger),
    packet_handler_(packet_handler) {
}

Session::~Session() {
    Close();
}

void Session::Start() {
    OnConnected();
    ReadHeader();
}

void Session::Close() {
    if (!socket_.is_open()) {
        return;
    }

    boost::system::error_code ec;
    socket_.shutdown(boost::asio::socket_base::shutdown_both, ec);
    if (ec) {
        logger_.LogWarning("[Session] shutdown error. ec: {}", ec.what());
    }

    socket_.close(ec);
    if (ec) {
        logger_.LogWarning("[Session] close error. ec: {}", ec.what());
    }
}

void Session::ReadHeader() {
    boost::asio::async_read(socket_,
        boost::asio::buffer(&header_, kHeaderTypeSize),
        boost::asio::bind_executor(strand_, [this, self = shared_from_this()](const boost::system::error_code& ec, std::size_t /*length*/) {
            if (ec) {
                OnDisconnected(ec);
                return;
            }

            ReadBody();
        }));
}

void Session::ReadBody() {
    boost::asio::async_read(socket_,
        boost::asio::buffer(data_, header_),
        boost::asio::bind_executor(strand_, [this, self = shared_from_this()](const boost::system::error_code& ec, std::size_t /*length*/) {
            if (ec) {
                OnDisconnected(ec);
                return;
            }

            int32_t packet_id = std::numeric_limits<int32_t>::max();
            // Buffer protocol;
            std::memcpy(&packet_id, data_.data(), kPacketIdTypeSize);

            const uint32_t packet_body_size = header_ - kPacketIdTypeSize;
            if (packet_body_size >= kPacketIdTypeSize + kMaxBodyTypeSize) {
                logger_.LogError("Unable to read body. protocol_length: {}", packet_body_size);
                Close();
                return;
            }

            if (!OnRecv(packet_id, data_.data() + kPacketIdTypeSize, packet_body_size)) {
                Close();
                return;
            }

            ReadHeader();
        }));
}

void Session::Send(std::shared_ptr<const BufferInfo> buf_info) {
    boost::asio::post(strand_, [this, self = shared_from_this(), buf_info] {
        const bool write_in_progress = !send_queue_.empty();
        send_queue_.push_back(buf_info);
            if (!write_in_progress) {
                Send();
            }
        });
}

void Session::Send() {
    const auto buf_info = send_queue_.front();
    boost::asio::async_write(socket_,
        boost::asio::buffer(buf_info->buffer, buf_info->length),
        [this, self = shared_from_this()](const boost::system::error_code& ec, std::size_t /*length*/) {
            if (ec) {
                OnError(ec);
                return;
            }

            boost::asio::post(strand_, [this, self] {
                send_queue_.pop_front();
                if (!send_queue_.empty()) {
                    Send();
                }
            });
        });
}

void Session::Send(int32_t packet_id, const uint8_t* packet_body, int32_t packet_body_size) {
    const uint32_t header_length = kPacketIdTypeSize + packet_body_size;
    const auto max_packet_size = kHeaderTypeSize + kPacketIdTypeSize + packet_body_size;

    auto buf_info = std::make_shared<BufferInfo>();
    if (buf_info->buffer.size() < max_packet_size) {
        logger_.LogError("[Session] Unable to send because buffer size is less than length. packet_id: {}, buff_size: {}, max_packet_size: {}",
            packet_id, buf_info->buffer.size(), max_packet_size); 
        return;
    }

    std::memcpy(buf_info->buffer.data(), &header_length, kHeaderTypeSize);
    std::memcpy(buf_info->buffer.data() + kHeaderTypeSize, &packet_id, kPacketIdTypeSize);
    std::memcpy(buf_info->buffer.data() + kHeaderTypeSize + kPacketIdTypeSize, packet_body, packet_body_size);
    buf_info->length = max_packet_size;

    Send(buf_info);
}

bool Session::OnRecv(int32_t packet_id, const uint8_t* buf, uint32_t size) {
    const IHandler* handler = packet_handler_.GetHandler(packet_id);
    if (handler == nullptr) {
        logger_.LogError("Fail to find handler. protocol_id: {}, size: {}", packet_id, size);
        return false;
    }

    if (!handler->OnHandle(buf, size, shared_from_this())) {
        logger_.LogError("Unable to parse from array. protocol_id: {}, size: {}", packet_id, size);
        return false;
    }

    return true;
}

const utility::ILogger& Session::logger() const {
    return logger_;
}

} // namespace dev::network
