#pragma once

#include "network/handler/packet_handler.h"

namespace dev::chat_server {

class UserHandler {
public:
    static bool Register(network::PacketHandler& packet_handler);
};

} // namespace dev::chat_server
