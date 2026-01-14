#pragma once

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

#include "utility/log/logger.h"
#include "network/handler/packet_handler.h"

namespace dev::network {

class Server : boost::noncopyable {
public:
    Server(
        boost::asio::io_context& io_context,
        const boost::asio::ip::tcp::endpoint& endpoint,
        const utility::ILogger& logger,
        const PacketHandler& packet_handler
    );

    void Start();

    boost::asio::io_context& io_context() const;

protected:
    virtual void OnAccepted(boost::asio::ip::tcp::socket&& socket) = 0;
    virtual void OnError(const boost::system::error_code& ec) = 0;

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    const utility::ILogger& logger_;
    const PacketHandler& packet_handler_;
};

} // namespace dev::network
