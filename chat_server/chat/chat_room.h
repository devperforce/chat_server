#pragma once

#include <map>
#include <boost/circular_buffer.hpp>
#include "system/actor.h"
#include "protobuf/generated/chatting.pb.h"
#include "chat_server/content/content_type.h"
#include "chat_server/user_detail/user.h"

namespace dev::chat_server {

class ChatRoom : public system::ActorLogic {
public:
    explicit ChatRoom(
        boost::asio::any_io_executor io_executor,
    
        int32_t room_id
    );

    void Broadcast(std::string nickname, std::shared_ptr<const chat::ChatBroadcastReq> chat_broadcast_req);

    void Join(std::shared_ptr<User> user);

    void Leave(const User* user);

    void GetChatHistory(std::shared_ptr<User> user) const;

    int32_t room_id() const;

    void NotifyAllUser(const std::shared_ptr<const google::protobuf::Message>& noti) const;

    void NotifySpecificUser(UserUid::Type user_uid, const std::shared_ptr<const google::protobuf::Message>& noti) const;

    const std::map<UserUid::Type, std::shared_ptr<User>>& users() const ;

private:
    std::map<UserUid::Type, std::shared_ptr<User>> users_;
    boost::circular_buffer<chat::NicknameChatMsgInfo> nickname_msg_infos_;
    
    int32_t room_id_;
};

} // namespace dev::chat_server
