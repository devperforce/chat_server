#pragma once

#include "network/handler/packet_handler.h"

namespace dev::chat_server {

class PacketHandlerRegistry {
public:
    explicit PacketHandlerRegistry(utility::ILogger& logger);

    bool Initialize() const;

    const network::PacketHandler& packet_handler() const;
    network::PacketHandler& packet_handler();

private:
    std::unique_ptr<network::PacketHandler> packet_handler_;
};

} // namespace dev::chat_server