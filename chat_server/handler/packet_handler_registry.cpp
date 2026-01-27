#include "pch.h"
#include "chat_server/handler/packet_handler_registry.h"
#include "chat_server/chat/chat_handler.h"
#include "chat_server/user_detail/user_handler.h"

namespace dev::chat_server {

PacketHandlerRegistry::PacketHandlerRegistry(utility::ILogger& logger)
    : packet_handler_(std::make_unique<network::PacketHandler>(logger)) {
}

bool PacketHandlerRegistry::Initialize() const {
    // 유저 핸들러 등록
    if (!UserHandler::Register(*packet_handler_)) {
        return false;
    }
    // 채팅 핸들러 등록
    if (!ChatHandler::Register(*packet_handler_)) {
        return false;
    }
    return true;
}

const network::PacketHandler& PacketHandlerRegistry::packet_handler() const {
    return *packet_handler_;
}

network::PacketHandler& PacketHandlerRegistry::packet_handler() {
    return *packet_handler_;
}

} // namespace dev::chat_server