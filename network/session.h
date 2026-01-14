#pragma once

#include <deque>
#include <boost/asio.hpp>
#include "utility/log/logger.h"
#include "network/handler/packet_handler.h"
#include "system/actor.h"

namespace dev::network {

class Session : public system::ActorStrand, public std::enable_shared_from_this<Session> {
public:
    static constexpr int32_t kHeaderTypeSize = sizeof(int32_t);
    static constexpr int32_t kPacketIdTypeSize = sizeof(int32_t);
    static constexpr int32_t kMaxBodyTypeSize = boost::asio::detail::default_max_transfer_size - kHeaderTypeSize - kPacketIdTypeSize;

    using Buffer = std::array<uint8_t, kPacketIdTypeSize + kMaxBodyTypeSize>;

    Session(
        boost::asio::io_context& io_context,
        boost::asio::ip::tcp::socket&& socket,
        const utility::ILogger& logger,
        const PacketHandler& packet_handler
    );

    virtual ~Session();

    void Start();
    void Close();

    virtual void OnConnected() = 0;
    virtual void OnDisconnected(const boost::system::error_code& ec) = 0;
    virtual void OnError(const boost::system::error_code& ec) = 0;

    const utility::ILogger& logger() const;

protected:
    void Send(int32_t packet_id, const uint8_t* packet_body, int32_t packet_body_size);

    boost::asio::ip::tcp::socket socket_;
    const utility::ILogger& logger_;

    struct BufferInfo {
        size_t length;
        Buffer buffer;
    };

    void Send(std::shared_ptr<const BufferInfo> buf_info);
    
private:
    bool OnRecv(int32_t packet_id, const uint8_t* buf, uint32_t size);

    void ReadHeader();
    void ReadBody();

    void Send();

    int32_t header_;
    Buffer data_;

    std::deque<std::shared_ptr<const BufferInfo>> send_queue_;

    const PacketHandler& packet_handler_;
};

} // namespace dev::network
