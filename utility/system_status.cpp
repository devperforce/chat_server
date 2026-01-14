#include "pch.h"
#include "utility/system_status.h"

namespace dev::utility {

bool PortInUse(boost::asio::ip::port_type port) {
    using namespace boost::asio;
    using ip::tcp;

    io_context io;

    boost::system::error_code ec;
    tcp::acceptor acceptor(io);

    acceptor.open(tcp::v4(), ec);
    if (ec) {
        return ec == boost::asio::error::address_in_use;
    }

    acceptor.bind({ tcp::v4(), port }, ec);
    if (ec) {
        return ec == boost::asio::error::address_in_use;
    }

    return false; // port is free
}

} // namespace dev::utility
