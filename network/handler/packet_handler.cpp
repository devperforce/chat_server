#include "pch.h"
#include "network/handler/packet_handler.h"

namespace dev::network {

PacketHandler::PacketHandler(utility::ILogger& logger)
    : logger_(logger) {
    std::fill(std::begin(handlers_), std::end(handlers_), nullptr);
}


utility::ILogger& PacketHandler::logger() const {
    return logger_;
}

bool PacketHandler::Register(int32_t packet_id, std::unique_ptr<IHandler> handler) {
    if (std::ssize(handlers_) <= packet_id) {
        return false;
    }
    if (handlers_[packet_id] != nullptr) {
        return false;
    }
    handlers_[packet_id] = std::move(handler);
    return true;
}

const IHandler* PacketHandler::GetHandler(int32_t packet_id) const {
    if (std::ssize(handlers_) <= packet_id || packet_id < 0) {
        return nullptr;
    }
    if (handlers_[packet_id] == nullptr) {
        return nullptr;
    }
    return handlers_[packet_id].get();
}

} // namespace dev::network
