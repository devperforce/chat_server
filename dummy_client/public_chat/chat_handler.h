#pragma once

#include "network/handler/packet_handler.h"


namespace dev::dummy_client {

class ChatHandler {
public:
    static bool Register(network::PacketHandler& packet_handler);
};

} // namespace dev::dummy_client
