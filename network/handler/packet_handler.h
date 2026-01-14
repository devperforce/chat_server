#pragma once

#include <array>
#include <boost/noncopyable.hpp>
#include "utility/log/logger.h"
#include "network/handler/handler.h"

namespace dev::network {

class Session;

class PacketHandler : boost::noncopyable {
public:
    static constexpr int32_t kMaxPacketId = 5000;
    explicit PacketHandler(utility::ILogger& logger);
    //~PacketHandler() = default;

    bool Register(int32_t packet_id, std::unique_ptr<IHandler> handler);
    const IHandler* GetHandler(int32_t index) const;

    utility::ILogger& logger () const;

protected:
    utility::ILogger& logger_;
    std::array<std::unique_ptr<IHandler>, kMaxPacketId> handlers_;
};

} // namespace dev::network
