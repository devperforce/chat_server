#pragma once

#include "network/server.h"
#include "network/handler/packet_handler.h"
#include "chat_server/session/chat_session.h"

namespace dev::chat_server {

class ChatRoomSelector;

class ChatServer final : public network::Server {
public:
    struct NetworkDependency {
        boost::asio::io_context& io_context;
        const boost::asio::ip::tcp::endpoint& endpoint;
        const utility::ILogger& logger;
        const network::PacketHandler& packet_handler;
    };
    ChatServer(
        const NetworkDependency& network_dependency,
        ChatRoomSelector& chat_room_selector
    );

    ChatRoomSelector& chat_room_selector() const;

private:
    void OnAccepted(boost::asio::ip::tcp::socket&& socket) override;
    void OnError(const boost::system::error_code& ec) override;

    std::unordered_map<int64_t /* uid */, std::shared_ptr<ChatSession>> sessions_;

    boost::asio::any_io_executor logic_executor_;

    ChatRoomSelector& chat_room_selector_;
};

} // namespace dev::chat_server
