#include "pch.h"
#include "chat_server/chat/chat_handler.h"

#include "network/handler/packet_handler.h"
#include "protobuf/handler/register_handler.h"
#include "protobuf/generated/chatting.pb.h"
#include "utility/log/static_logger.h"
#include "chat_server/chat_server.h"
#include "chat_server/session/chat_session.h"
#include "chat_server/chat/chat_room_selector.h"
#include "chat_server/chat/chat_room.h"
#include "chat_server/user_detail/user.h"
#include "chat_server/user_detail/user_repository.h"

namespace dev::chat_server {
using namespace utility;

static bool OnLogin(std::shared_ptr<ChatSession> session, std::shared_ptr<const chat::LoginReq> req) {
    BOOST_ASSERT(session->TestSynchronize());
    LOG_INFO("[ChatHandler] OnLogin. req: {}", req->DebugString());
    
    boost::asio::post(session->chat_server().chat_room_selector().io_executor(), [session, req] {
        const auto user_uid = req->user_uid();
        const auto new_user = User::Create(session->chat_server().chat_room_selector().io_executor(), session, user_uid, req->nickname());

        // 중복 로그인 기존 채널에서 나가게 해준다
        auto old_user = UserRepository::GetInstance().Get(user_uid);
        if (old_user != nullptr) {
            old_user->Close();
        }

        UserRepository::GetInstance().Add(new_user);

        std::shared_ptr<ChatSession> new_session = session;
        CONTROLLER(new_session).Post([new_user](std::shared_ptr<ChatSession> chat_session) {
            chat_session->set_user(new_user);
            auto res = std::make_shared<chat::LoginRes>();
            chat_session->Send(res);
        });
    });
    
    return true;
}

static bool OnChatJoin(std::shared_ptr<ChatSession> session, std::shared_ptr<const chat::ChatJoinReq> req) {
    BOOST_ASSERT(session->TestSynchronize());

    session->logger().LogDebug("[ChatJoinReq]: {}", req->DebugString());
    auto user = session->user();
    if (user == nullptr) {
        session->logger().LogError("[OnChatJoin] User is null.");
        return false;
    }

    auto& chat_room_selector = session->chat_server().chat_room_selector();
    CONTROLLER(chat_room_selector).Post([user] (ChatRoomSelector& chat_room_selector) {
        // 기존 유저가 있다면 채팅 채널에서 내보내기
        auto chat_room = chat_room_selector.GetChatRoom();
        if (chat_room == nullptr) {
            auto res = std::make_shared<chat::ChatJoinRes>();
            res->set_result(chat::ChatJoinRes::Result::ChatJoinRes_Result_kError);
            user->Send(res);
            return;
        }
        chat_room->Join(user);
    });

    return true;
}

static bool OnChatHistory(std::shared_ptr<ChatSession> session, std::shared_ptr<const chat::ChatHistoryReq> req) {
    BOOST_ASSERT(session->TestSynchronize());

    auto user = session->user();
    if (user == nullptr) {
        session->logger().LogError("[OnChatJoin] User is null.");
        return false;
    }

    //BOOST_ASSERT(user->TestSynchronize());

    CONTROLLER(user).Post([](std::shared_ptr<User> user) {
        auto chat_room = user->chat_room();
        if (chat_room == nullptr) {
            return;
        }

        chat_room->GetChatHistory(user);
    });

    return true;
}

static bool OnChatBroadcast(std::shared_ptr<ChatSession> session, std::shared_ptr<const chat::ChatBroadcastReq> req) {
    LOG_INFO("[ChatHandler] OnChatBroadcast. req: {}", req->GetTypeName());

    auto user = session->user();
    if (user == nullptr) {
        session->logger().LogError("[OnChatJoin] User is null.");
        return false;
    }

    CONTROLLER(*user).Post([req](User& user) {
        auto chat_room = user.chat_room();
        if (chat_room == nullptr) {
            return;
        }

        chat_room->Broadcast(user.nickname(), req);
    });

    return true;
}

bool ChatHandler::Register(network::PacketHandler& packet_handler) {
    bool result = true;

    result &= RegisterHandler(packet_handler, OnLogin);
    result &= RegisterHandler(packet_handler, OnChatJoin);
    result &= RegisterHandler(packet_handler, OnChatHistory);
    result &= RegisterHandler(packet_handler, OnChatBroadcast);

    return result;
}

} // namespace dev::chat_server
