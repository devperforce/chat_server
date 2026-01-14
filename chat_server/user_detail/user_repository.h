#pragma once

#include "system/singleton.h"
#include "chat_server/session/chat_session.h"
#include "chat_server/content/content_type.h"

namespace dev::chat_server {

class User;

class UserRepository : public system::Singleton<UserRepository> {
public:
    explicit UserRepository();

    void Add(std::shared_ptr<User> user);
    void Remove(UserUid::Type user_uid);
    std::shared_ptr<User> Get(UserUid::Type user_uid) const;
    
    size_t GetUserCount() const;

private:
    bool TestSynchronize() const;

    std::thread::id thread_id_;
    std::unordered_map<UserUid::Type, std::shared_ptr<User>> users_;
};

} // namespace dev::chat_server
