#include "pch.h"
#include "dummy_client/public_chat/chat_handler.h"

#include "network/handler/packet_handler.h"
#include "protobuf/handler/register_handler.h"
#include "protobuf/protobuf_session.h"
#include "protobuf/generated/chatting.pb.h"
#include "dummy_client/dummy_session.h"

namespace dev::dummy_client {

static bool OnPing(std::shared_ptr<DummySession> session, std::shared_ptr<const chat::PingReq> req) {
    session->logger().LogDebug("OnPing");

    auto res = std::make_shared<chat::PingRes>();
    session->Send(res);

    return true;
}

bool ChatHandler::Register(network::PacketHandler& packet_handler) {
    bool result = true;

    result &= protobuf::RegisterHandler(packet_handler, OnPing);
     
    return result;
}

} // namespace dev::dummy_client
