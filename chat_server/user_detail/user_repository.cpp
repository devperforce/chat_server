#include "pch.h"
#include "chat_server/user_detail/user_repository.h"

#include "utility/log/static_logger.h"
#include "chat_server/user_detail/user.h"

namespace dev::chat_server {

UserRepository::UserRepository() : thread_id_(std::this_thread::get_id()) {
}

void UserRepository::Add(std::shared_ptr<User> user) {
    BOOST_ASSERT(TestSynchronize());

    const auto user_uid = user->user_uid();
    if (users_.contains(user_uid)) {
        users_.erase(user_uid);
        utility::LOG_INFO("[UserRepository] Erase old user. user_uid: {}, user_count: {}", user_uid, std::ssize(users_));
    }
    users_.emplace(user_uid, user);
    utility::LOG_INFO("[UserRepository] Add. user_uid: {}, user_count: {}", user_uid, std::ssize(users_));
}

void UserRepository::Remove(UserUid::Type user_uid) {
    BOOST_ASSERT(TestSynchronize());

    const size_t num_erased = users_.erase(user_uid);
    if (num_erased == 0) {
        utility::LOG_INFO("[UserRepository] Not found user. user_uid: {}", user_uid);
    } else {
        utility::LOG_INFO("[UserRepository] Remove. user_count: {}", std::ssize(users_));
    }
}

std::shared_ptr<User> UserRepository::Get(UserUid::Type user_uid) const {
    BOOST_ASSERT(TestSynchronize());

    const auto it = users_.find(user_uid);
    if (it != users_.end()) {
        BOOST_ASSERT(it->second != nullptr);
        return it->second;
    }
    return nullptr;
}

size_t UserRepository::GetUserCount() const {
    return users_.size();
}

bool UserRepository::TestSynchronize() const {
    return thread_id_ == std::this_thread::get_id();
}

} // namespace dev::chat_server
