#include "pch.h"
#include "chat_server/session/chat_session.h"

#include "protobuf/generated/chatting.pb.h"

namespace dev::chat_server {

ChatSession::ChatSession(
    boost::asio::io_context& io_context,
    boost::asio::ip::tcp::socket&& socket,
    const utility::ILogger& logger,
    const network::PacketHandler& packet_handler
) : ProtobufSession(io_context, std::move(socket), logger, packet_handler) {
}

const UserInfo* ChatSession::user_info() const {
    return user_info_.get();
}

void ChatSession::set_user_info(std::unique_ptr<UserInfo> user_info) {
    user_info_ = std::move(user_info);
}

void ChatSession::OnConnected() {
    logger_.LogError("[ChatSession] OnConnected");
    auto req = std::make_shared<chat::PingReq>();
    Send(req);
}

void ChatSession::OnDisconnected(const boost::system::error_code& ec) {
    logger_.LogError("[ChatSession] OnDisconnected, {}", ec.what());
}

void ChatSession::OnError(const boost::system::error_code& ec) {
    logger_.LogError("[ChatSession] OnError, {}", ec.what());
}


} // namespace dev::chat_server
