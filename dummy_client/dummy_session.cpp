#include "pch.h"
#include "dummy_client/dummy_session.h"

#include "generated/chatting.pb.h"

namespace dev::dummy_client {

DummySession::DummySession(
    boost::asio::io_context& io_context,
    boost::asio::ip::tcp::socket&& socket,
    const utility::ILogger& logger,
    const network::PacketHandler& packet_handler
) : ProtobufSession(io_context, std::move(socket), logger, packet_handler) {
}

void DummySession::Connect(
    boost::asio::ip::tcp::endpoint endpoint
) {
    socket_.async_connect(endpoint, [&, self = shared_from_this()](const boost::system::error_code& ec) {
        if (ec) {
            logger_.LogError("fail to connect");
            return;
        }
        logger_.LogDebug("success to connect");
        Start();
    });
}

void DummySession::OnConnected() {
    logger_.LogDebug("OnConnected");
}

void DummySession::OnDisconnected(const boost::system::error_code& ec) {
    logger_.LogDebug("OnDisconnected. ec: {}", ec.what());
}

void DummySession::OnError(const boost::system::error_code& ec) {
    logger_.LogDebug("OnError. ec: {}", ec.what());
}

} // namespace dev::dummy_client
