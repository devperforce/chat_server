#include "pch.h"
#include "chat_server/chat/chat_room.h"

#include "utility/log/static_logger.h"
#include "chat_server/session/chat_session.h"
#include "chat_server/chat/chat_room_selector.h"

namespace dev::chat_server {

static constexpr int32_t kMaxMsgSize = 20;

ChatRoom::ChatRoom(
    boost::asio::any_io_executor io_executor,
    int32_t room_id)
    : ActorLogic(io_executor),
      room_id_(room_id) {
    nickname_msg_infos_.set_capacity(kMaxMsgSize);
}

static std::shared_ptr<chat::ChatBroadcastNoti> CreateBroadcastNoti(
    std::string&& nickname,
    const chat::ChatBroadcastReq& req
) {
    auto noti = std::make_shared<chat::ChatBroadcastNoti>();
    auto* nickname_msg_info = noti->mutable_nickname_msg_info();
   
    nickname_msg_info->set_nickname(std::move(nickname));
    nickname_msg_info->mutable_msg_info()->CopyFrom(req.chat_msg_info());
    return noti;
}

void ChatRoom::Broadcast(std::string nickname, std::shared_ptr<const chat::ChatBroadcastReq> chat_broadcast_req) {
    BOOST_ASSERT(TestSynchronize());

    const auto noti = CreateBroadcastNoti(std::move(nickname), *chat_broadcast_req);

    // 히스토리 저장
    nickname_msg_infos_.push_back(noti->nickname_msg_info());
    utility::LOG_INFO("[ChatRoom] Broadcast. nickname_msg_infos size: {}", std::ssize(nickname_msg_infos_));

    for (const auto& user : users_ | std::ranges::views::values) {
        if (user != nullptr) {
            user->Send(noti);
        }
    }
}

void ChatRoom::Join(std::shared_ptr<User> user) {
    BOOST_ASSERT(TestSynchronize());

    const auto user_uid = user->user_uid();
    users_.insert_or_assign(user_uid, user);

    utility::LOG_INFO("[ChatRoom] Join. user_count: {}", std::ssize(users_));

    user->OnJoinChatRoom(this);
}

void ChatRoom::Leave(const User* user) {
    BOOST_ASSERT(TestSynchronize());

    const auto user_uid = user->user_uid();
    const auto it = users_.find(user_uid);
    if (it != users_.end() && it->second.get() == user) {
        users_.erase(it);
        utility::LOG_INFO("[ChatRoom] Leave. users size: {}", std::ssize(users_));
    }
}

void ChatRoom::GetChatHistory(std::shared_ptr<User> user) const {
    BOOST_ASSERT(TestSynchronize());

    auto res = std::make_shared<chat::ChatHistoryRes>();
    for (const auto& nickname_msg_info : nickname_msg_infos_) {
        auto add_msg_info = res->add_nickname_msg_infos();
        add_msg_info->CopyFrom(nickname_msg_info);
    }
    user->Send(res);
}

void ChatRoom::NotifyAllUser(const std::shared_ptr<const google::protobuf::Message>& noti) const {
    BOOST_ASSERT(TestSynchronize());

    for (std::shared_ptr<User> user : users_ | std::ranges::views::values) {
        user->Send(noti);
    }
}

void ChatRoom::NotifySpecificUser(UserUid::Type user_uid, const std::shared_ptr<const google::protobuf::Message>& noti) const {
    BOOST_ASSERT(TestSynchronize());

    const auto it = users_.find(user_uid);
    if (it == users_.end() || it->second == nullptr) {
        return;
    }

    it->second->Send(noti);
}

int32_t ChatRoom::room_id() const {
    BOOST_ASSERT(TestSynchronize());

    return room_id_;
}

const std::map<UserUid::Type, std::shared_ptr<User>>& ChatRoom::users() const {
    return users_;
}

} // namespace dev::chat_server
