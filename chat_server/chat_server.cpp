#include "pch.h"
#include "chat_server/chat_server.h"
#include "chat_server/chat/chat_room_selector.h"

namespace dev::chat_server {

ChatServer::ChatServer(
    const NetworkDependency& network_dependency,
    const database::DatabaseService& database_service,
    ChatRoomSelector& chat_room_selector)
    : Server(network_dependency.io_context, network_dependency.endpoint, network_dependency.logger, network_dependency.packet_handler),
    database_service_(database_service),
    chat_room_selector_(chat_room_selector) {
}

void ChatServer::OnAccepted(boost::asio::ip::tcp::socket&& socket) {
    auto session = std::make_shared<ChatSession>(*this, std::move(socket), logger_, packet_handler_);
    session->Start();
}

void ChatServer::OnError(const boost::system::error_code& ec) {

}

ChatRoomSelector& ChatServer::chat_room_selector() const {
    return chat_room_selector_;
}

const database::DatabaseService& ChatServer::database_service() const {
    return database_service_;
}

} // namespace dev::chat_server
