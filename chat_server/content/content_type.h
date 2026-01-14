#pragma once

#include <format>

namespace dev::chat_server {

class UserUid {
public:
    using Type = uint64_t;
    inline static const std::string Name = "userUid";

    explicit UserUid(Type value) : value_(value) {}
    operator Type() const { return value_; }

private:
    Type value_;
};

} // namespace dev::chat_server

template <>
struct std::formatter<dev::chat_server::UserUid> : std::formatter<dev::chat_server::UserUid::Type> {
    auto format(const dev::chat_server::UserUid& uid, auto& ctx) const {
        return formatter<dev::chat_server::UserUid::Type>::format(static_cast<dev::chat_server::UserUid::Type>(uid), ctx);
    }
};