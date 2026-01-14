#include "pch.h"
#include "chat_server/user_detail/user_handler.h"

#include "network/handler/packet_handler.h"
#include "protobuf/handler/register_handler.h"
#include "protobuf/generated/chatting.pb.h"
#include "utility/log/static_logger.h"
#include "chat_server/session/chat_session.h"

namespace dev::chat_server {
using namespace utility;

static bool OnPing(std::shared_ptr<ChatSession> session, std::shared_ptr<const chat::PingRes> res) {
    BOOST_ASSERT(session->TestSynchronize());

    LOG_DEBUG("[UserHandler] OnPing: {}", res->GetTypeName());
    session->RecvPing();
    return true;
}

bool UserHandler::Register(network::PacketHandler& packet_handler) {
    bool result = true;

    result &= RegisterHandler(packet_handler, OnPing);
  
    return result;
}

} // namespace dev::chat_server
