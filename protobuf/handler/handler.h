#pragma once

#include "network/handler/handler.h"

namespace dev::protobuf {

template <typename Session, typename Message>
class ProtobufHandler : public network::IHandler {
public:
    using Callback = bool(*)(std::shared_ptr<Session> session, std::shared_ptr<const Message>);

    explicit ProtobufHandler(Callback callback) : callback_(callback) {
    }

    virtual ~ProtobufHandler() = default;

    bool OnHandle(const uint8_t* buf, uint32_t size, std::shared_ptr<network::Session> session) const override {
        //const boost::asio::const_buffer buffer(buf, size);
        auto msg = std::make_shared<Message>();
        const bool result = msg->ParseFromArray(buf, size);
        if(!result) {
            return false;
        }
        return callback_(std::static_pointer_cast<Session>(session), msg);
    }

private:
    Callback callback_ = nullptr;
};

} // namespace dev::protobuf
