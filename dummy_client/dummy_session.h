#pragma once

#include <boost/asio.hpp>
#include "utility/log/logger.h"
#include "network/handler/packet_handler.h"
#include "protobuf/protobuf_session.h"

namespace dev::dummy_client {

class DummySession final : public protobuf::ProtobufSession {
public:
    DummySession(
        boost::asio::io_context& io_context,
        boost::asio::ip::tcp::socket&& socket,
        const utility::ILogger& logger,
        const network::PacketHandler& packet_handler
    );

    void Connect(
        boost::asio::ip::tcp::endpoint endpoint
    );

private:
    void OnConnected() override;
    void OnDisconnected(const boost::system::error_code& ec) override;
    void OnError(const boost::system::error_code& ec) override;
};

} // namespace dev::dummy_client
