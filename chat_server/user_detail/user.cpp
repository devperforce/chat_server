#include "pch.h"
#include "chat_server/user_detail/user.h"

#include "protobuf/generated/chatting.pb.h"
#include "chat_server/user_detail/user_repository.h"
#include "chat_server/chat/chat_room.h"
#include "utility/log/static_logger.h"

namespace dev::chat_server {

std::shared_ptr<User> User::Create(
    boost::asio::any_io_executor io_executor,
    std::shared_ptr<ChatSession> session,
    UserUid::Type user_uid,
    const std::string& nickname
) {
    auto user = std::make_shared<User>(io_executor, session, user_uid, nickname);
    return user;
}

User::User(
    boost::asio::any_io_executor io_executor,
    std::shared_ptr<ChatSession> session,
    UserUid::Type user_uid,
    const std::string& nickname
) : ActorLogic(io_executor),
    session_(session),
    user_uid_(user_uid),
    nickname_(nickname) {
}

User::~User() {
    utility::LOG_INFO("[User] ~User. user_uid: {}", user_uid_);
}

std::shared_ptr<ChatSession> User::session() const {
    return session_;
}

UserUid::Type User::user_uid() const {
    return user_uid_;
}

const std::string& User::nickname() const {
    return nickname_;
}

ChatRoom* User::chat_room() const {
    return chat_room_;
}

void User::OnDisconnected() {
    BOOST_ASSERT(TestSynchronize());

    session_ = nullptr;
    Close();
}

void User::Send(std::shared_ptr<const google::protobuf::Message> msg) const {
    if (session_ != nullptr) {
        session_->Send(msg);
    }
}

void User::OnJoinChatRoom(ChatRoom* chat_room) {
    BOOST_ASSERT(TestSynchronize());

    auto res = std::make_shared<chat::ChatJoinRes>();
    res->set_result(chat::ChatJoinRes::Result::ChatJoinRes_Result_kSuccess);
    res->set_room_id(chat_room->room_id());

    chat_room_ = chat_room;

    Send(res);
}

void User::Close() {
    BOOST_ASSERT(TestSynchronize());

    if (finalized_) {
        utility::LOG_INFO("[User] Already finalized. user_uid: {}", user_uid_);
        return;
    }

    finalized_ = true;

    utility::LOG_INFO("[User] OnDisconnected. user_uid: {}", user_uid_);
    if (chat_room_ != nullptr) {
        chat_room_->Leave(this);
    }

    chat_room_ = nullptr;

    auto user = UserRepository::GetInstance().Get(user_uid_);
    if (user.get() == this) {
        UserRepository::GetInstance().Remove(user_uid_);
    }

    if (session_ == nullptr) {
        return;
    }

    CONTROLLER(session_).Post([](std::shared_ptr<ChatSession> chat_session) {
        chat_session->set_user(nullptr);
        chat_session->Close();
    });
}

} // namespace dev::chat_server
