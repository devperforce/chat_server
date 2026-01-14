#pragma once

#include <boost/asio.hpp>
#include <google/protobuf/message.h>

#include "network/session.h"
#include "utility/log/logger.h"
#include "network/handler/packet_handler.h"

namespace dev::protobuf {

class ProtobufSession : public network::Session {
public:
    ProtobufSession(
        boost::asio::io_context& io_context,
        boost::asio::ip::tcp::socket&& socket,
        const utility::ILogger& logger,
        const network::PacketHandler& packet_handler
    );

    void Send(std::shared_ptr<const google::protobuf::Message> msg);

};

} // namespace dev::protobuf
