#include "pch.h"
#include "network/server.h"

namespace dev::network {

Server::Server(
    boost::asio::io_context& io_context,
    const boost::asio::ip::tcp::endpoint& endpoint,
    const utility::ILogger& logger,
    const PacketHandler& packet_handler
) : io_context_(io_context),
    acceptor_(io_context, endpoint),
    logger_(logger),
    packet_handler_(packet_handler) {
}

void Server::Start() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                OnAccepted(std::move(socket));
            } else if (ec == boost::asio::error::operation_aborted) {
                logger_.LogInfo("[Server] Accept operation aborted (shutdown).");
            } else {
                logger_.LogError("[Server] Accept error: {}", ec.message());
            }

            if (acceptor_.is_open()) {
                Start();
            } else {
                logger_.LogInfo("[Server] Acceptor is closed. Accept loop ended.");
            }
        });
}

boost::asio::io_context& Server::io_context() const {
    return io_context_;
}

} // namespace dev::network
