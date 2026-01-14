#include "pch.h"
#include "system/actor.h"

namespace dev::system {

// IActor
IActor::~IActor() {
}

// Actor
Actor::Actor(boost::asio::io_context& io_context)
    : io_context_(io_context) {
}

Actor::~Actor() {
}

boost::asio::any_io_executor Actor::io_executor() const {
    return io_context_.get_executor();
}

bool Actor::TestSynchronize() const {
    return io_context_.get_executor().running_in_this_thread();
}

// ActorLogic
ActorLogic::ActorLogic(boost::asio::any_io_executor io_executor)
    : io_executor_(io_executor), thread_id_(std::this_thread::get_id()) {
}

ActorLogic::~ActorLogic() {
}

boost::asio::any_io_executor ActorLogic::io_executor() const {
    return io_executor_;
}

bool ActorLogic::TestSynchronize() const {
    return thread_id_ == std::this_thread::get_id();
}

// ActorStrand
ActorStrand::ActorStrand(boost::asio::any_io_executor io_executor)
    : strand_(boost::asio::make_strand(io_executor)) {
}

ActorStrand::~ActorStrand() {
}

boost::asio::any_io_executor ActorStrand::io_executor() const {
    return strand_;
}

bool ActorStrand::TestSynchronize() const {
    return strand_.running_in_this_thread();
}

} // namespace dev::system
