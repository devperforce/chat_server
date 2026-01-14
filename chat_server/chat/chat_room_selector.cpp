#include "pch.h"
#include "chat_server/chat/chat_room_selector.h"

#include "utility/log.h"
#include "chat_server/chat/chat_room.h"

namespace dev::chat_server {

// ChatRoomSelector
ChatRoomSelector::ChatRoomSelector(
    boost::asio::io_context& io_context
) : Actor(io_context) {
}

ChatRoomSelector::~ChatRoomSelector() {
}

const std::vector<std::unique_ptr<ChatRoom>>& ChatRoomSelector::chat_rooms() const {
    BOOST_ASSERT(TestSynchronize());
    LOG_DEBUG("ChatRoomSelector::chat_rooms");

    return chat_rooms_;
}

// MaxCapacitySelector
MaxCapacitySelector::MaxCapacitySelector(
    boost::asio::io_context& io_context,
    const Setting& setting
) : ChatRoomSelector(io_context), setting_(setting) {
}

MaxCapacitySelector::~MaxCapacitySelector() {
}

ChatRoom* MaxCapacitySelector::GetChatRoom() {
    BOOST_ASSERT(TestSynchronize());
    LOG_DEBUG("MaxCapacitySelector::GetChatRoom()");

    uint64_t total_user_count = 0;
    for (const auto& chat_room : chat_rooms_) {
        if (!IsAvailable(*chat_room)) {
            total_user_count += chat_room->users().size();
            continue;
        }
        return chat_room.get();
    }

    if (setting_.max_room_count * setting_.max_capacity_per_room <= total_user_count) {
        return nullptr;
    }

    chat_rooms_.emplace_back(std::make_unique<ChatRoom>(io_executor(), static_cast<int32_t>(std::ssize(chat_rooms_))));
    ChatRoom* last_chat = chat_rooms_.back().get();
    return last_chat;
}

bool MaxCapacitySelector::IsAvailable(const ChatRoom& chat_room) const {
    return chat_room.users().size() < setting_.max_capacity_per_room;
}

} // namespace dev::chat_server
