#pragma once

#include <boost/asio.hpp>
#include "utility/log/logger.h"
#include "network/handler/packet_handler.h"
#include "content/content_type.h"

#include "protobuf/protobuf_session.h"

namespace dev::chat_server {

struct UserInfo {
    content::UserUid::Type user_uid;
    std::string nickname;
};

class ChatSession final : public protobuf::ProtobufSession {
public:
    ChatSession(
        boost::asio::io_context& io_context,
        boost::asio::ip::tcp::socket&& socket,
        const utility::ILogger& logger,
        const network::PacketHandler& packet_handler
    );

    const UserInfo* user_info() const;
    void set_user_info(std::unique_ptr<UserInfo> user_info);

private:
    void OnConnected() override;
    void OnDisconnected(const boost::system::error_code& ec) override;
    void OnError(const boost::system::error_code& ec) override;

    std::unique_ptr<UserInfo> user_info_;
};

} // namespace dev::chat_server
