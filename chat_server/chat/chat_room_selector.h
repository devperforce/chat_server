#pragma once

#include "system/actor.h"

namespace dev::chat_server {

class ChatRoom;

class ChatRoomSelector : public system::Actor {
public:
    explicit ChatRoomSelector(boost::asio::io_context& io_executor);
    virtual ~ChatRoomSelector();

    virtual ChatRoom* GetChatRoom() = 0;
    virtual bool IsAvailable(const ChatRoom& chat_room) const = 0;

    const std::vector<std::unique_ptr<ChatRoom>>& chat_rooms() const;

protected:
    std::vector<std::unique_ptr<ChatRoom>> chat_rooms_;
};

class MaxCapacitySelector final : public ChatRoomSelector {
public:
    struct Setting {
        uint64_t max_room_count = 0;
        uint64_t max_capacity_per_room = 0;
    };

    MaxCapacitySelector(boost::asio::io_context& io_executor, const Setting& setting);
    virtual ~MaxCapacitySelector();

    ChatRoom* GetChatRoom() override;
    bool IsAvailable(const ChatRoom& chat_room) const override;

private:
    Setting setting_;
};

} // namespace dev::chat_server
