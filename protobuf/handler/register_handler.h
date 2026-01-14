#pragma once

#include "network/handler/packet_handler.h"
#include "protobuf/handler/handler.h"
#include "protobuf/generated/packet_id.pb.h"

namespace dev::protobuf {

template <typename Session, typename Message>
static bool RegisterHandler(
    network::PacketHandler& packet_handler,
    bool(*callback)(std::shared_ptr<Session>, std::shared_ptr<const Message>)
) {
    static_assert(
        std::is_base_of_v<network::Session, Session>,
        "Session must derive from network::Session"
        );

    static_assert(
        std::is_base_of_v<google::protobuf::Message, Message>,
        "Message must derive from google::protobuf::Message"
        );

    const auto* desc = Message::descriptor();
    if (desc == nullptr) {
        return false;
    }

    const auto& options = desc->options();
    if (!options.HasExtension(type_enum)) {
        return false;
    }

    const PacketId packet_id = options.GetExtension(type_enum);

    auto handler = std::make_unique<ProtobufHandler<Session, Message>>(callback);
    return packet_handler.Register(std::to_underlying(packet_id), std::move(handler));
}

} // namespace dev::protobuf
