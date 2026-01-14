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
    try {
        acceptor_.async_accept(
            [this](const boost::system::error_code& ec, boost::asio::ip::tcp::socket socket) {
                if (ec) {
                    OnError(ec);
                } else {
                    //socket.set_option(boost::asio::ip::tcp::no_delay(true));
                    //socket.set_option(boost::asio::socket_base::send_buffer_size(65536));
                    //socket.set_option(boost::asio::socket_base::receive_buffer_size(65536));
                    OnAccepted(std::move(socket));
                }

                Start();
            });
    } catch (const std::exception& e) {
        logger_.LogError("[Server] Unable to async accept. exception: {}", e.what());
        auto timer = std::make_shared<boost::asio::steady_timer>(
            io_context_, std::chrono::seconds(1)
        );

        timer->async_wait([this, timer](const boost::system::error_code& /*ec*/) {
            Start();
        });
    }
}

boost::asio::io_context& Server::io_context() const {
    return io_context_;
}

} // namespace dev::network
