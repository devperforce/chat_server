#include "pch.h"
#include "protobuf/protobuf_session.h"

#include "network/session.h"
#include "protobuf/generated/packet_id.pb.h"

namespace dev::protobuf {

ProtobufSession::ProtobufSession(
    boost::asio::io_context& io_context,
    boost::asio::ip::tcp::socket&& socket,
    const utility::ILogger& logger,
    const network::PacketHandler& packet_handler
) : Session(io_context, std::move(socket), logger, packet_handler) {
}

void ProtobufSession::Send(std::shared_ptr<const google::protobuf::Message> msg) {
    const auto buffer_info = std::make_shared<BufferInfo>();

    const int32_t msg_size = static_cast<int32_t>(msg->ByteSizeLong());
    if (kMaxBodyTypeSize <= msg_size) {
        logger_.LogError("[Send] msg size: {}", msg_size);
        return;
    }
    buffer_info->length = kHeaderTypeSize + kPacketIdTypeSize + msg_size;
    const uint32_t header_length = kPacketIdTypeSize + msg_size;
    const uint32_t protocol_id = msg->GetDescriptor()->options().GetExtension(type_enum);

    std::memcpy(buffer_info->buffer.data(), &header_length, kHeaderTypeSize);
    std::memcpy(buffer_info->buffer.data() + kHeaderTypeSize, &protocol_id, kPacketIdTypeSize);
    msg->SerializeToArray(buffer_info->buffer.data() + kHeaderTypeSize + kPacketIdTypeSize, msg_size);

    Session::Send(buffer_info);
}

} // namespace dev::protobuf
