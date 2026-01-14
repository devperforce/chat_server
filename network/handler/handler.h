#pragma once

namespace dev::network {

class Session;

class IHandler {
public:
    virtual ~IHandler() = default;
    virtual bool OnHandle(const uint8_t* buf, uint32_t size, std::shared_ptr<Session> session) const = 0;
};

} // namespace dev::network
