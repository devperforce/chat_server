#pragma once

#include "system/actor.h"
#include "chat_server/session/chat_session.h"
#include "chat_server/content/content_type.h"

namespace dev::chat_server {

class ChatRoom;

class User final : public system::ActorLogic, public std::enable_shared_from_this<User> {
public:
    static std::shared_ptr<User> Create(
        boost::asio::any_io_executor io_executor,
        std::shared_ptr<ChatSession> session,
        UserUid::Type user_uid,
        const std::string& nickname
    );

    User(boost::asio::any_io_executor io_executor, std::shared_ptr<ChatSession> session, UserUid::Type user_uid, const std::string& nickname);
    ~User();

    std::shared_ptr<ChatSession> session() const;
    UserUid::Type user_uid() const;
    const std::string& nickname() const;

    ChatRoom* chat_room() const;

    void OnDisconnected();

    void Send(std::shared_ptr<const google::protobuf::Message> msg) const;

    void OnJoinChatRoom(ChatRoom* chat_room);

    void Close();

private:
    std::shared_ptr<ChatSession> session_;
    UserUid::Type user_uid_;
    std::string nickname_;

    ChatRoom* chat_room_ = nullptr;

    bool finalized_ = false;
};

} // namespace dev::chat_server
