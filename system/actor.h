#pragma once

#include <boost/noncopyable.hpp>
#include "system/type.h"

namespace dev::system {

class IActor : public boost::noncopyable {
public:
    virtual ~IActor();
    virtual boost::asio::any_io_executor io_executor() const = 0;

protected:
    virtual bool TestSynchronize() const = 0;
};

class Actor : public IActor {
public:
    explicit Actor(boost::asio::io_context& io_executor);
    virtual ~Actor();

    boost::asio::any_io_executor io_executor() const override;

    bool TestSynchronize() const override;

private:
    boost::asio::io_context& io_context_;
};

class ActorLogic : public IActor {
public:
    explicit ActorLogic(boost::asio::any_io_executor);
    virtual ~ActorLogic();

    boost::asio::any_io_executor io_executor() const override;

    bool TestSynchronize() const override;

private:
    std::thread::id thread_id_;
    boost::asio::any_io_executor io_executor_;
};

class ActorStrand : public IActor {
public:
    explicit ActorStrand(boost::asio::any_io_executor io_executor);
    virtual ~ActorStrand();

    boost::asio::any_io_executor io_executor() const override;

    bool TestSynchronize() const override;

protected:
    boost::asio::strand<boost::asio::any_io_executor> strand_;
};

template <typename Type>
class Controller {
public:
    using _Type = std::remove_cvref_t<Type>;

    Controller(Type& actor) : actor_{ actor } {
        static_assert(is_smart_ptr<Type>::value, "Type is not smart_pointer");
    }

    Controller(Type&& actor) noexcept : actor_{ std::move(actor) } {
        static_assert(is_smart_ptr<std::remove_cvref_t<Type>>::value, "Type is not smart_pointer");
    }

    template <typename Func>
    void Post(Func&& fn) {
        using Actor = typename _Type::element_type;

        static_assert(std::is_base_of_v<IActor, Actor>,
            "Type is not derived from IActor");

        std::shared_ptr<Actor> actor = nullptr;

        if constexpr (is_shared_ptr<_Type>::value) {
            actor = actor_;
        } else if constexpr (is_unique_ptr<_Type>::value) {
            actor = std::move(actor_);
        } else if constexpr (is_weak_ptr<_Type>::value) {
            actor = actor_.lock();
            if (!actor) {
                return;
            }
        } else {
            static_assert([] { return false; }(), "Unsupported pointer type");
        }

        auto callback =
            [actor, f = std::forward<Func>(fn)]() mutable {
            BOOST_ASSERT_MSG(actor->TestSynchronize(),
                "Running in other thread");

            if constexpr (std::is_invocable_v<Func, Actor&>) {
                f(*actor);
            } else if constexpr (std::is_invocable_v<Func, std::shared_ptr<Actor>>) {
                f(actor);
            } else {
                static_assert(
                    std::is_invocable_v<Func, Actor&> ||
                    std::is_invocable_v<Func, std::shared_ptr<Actor>>,
                    "Func must be callable with Actor& or std::shared_ptr<Actor>"
                    );
            }
        };

        boost::asio::post(actor->io_executor(), std::move(callback));
    }

    template <typename Derived, typename Func>
    void Post(Func&& fn) {
        using Base = typename _Type::element_type;

        static_assert(std::is_base_of_v<typename _Type::element_type, Derived>, "Class Derived should be base for class Type");

        std::shared_ptr<Base> actor = nullptr;

        if constexpr (is_shared_ptr<_Type>::value) {
            actor = actor_;
        } else if constexpr (is_unique_ptr<_Type>::value) {
            actor = std::move(actor_);
        } else if constexpr (is_weak_ptr<_Type>::value) {
            actor = actor_.lock();
            if (!actor) {
                return;
            }
        } else {
            static_assert([] { return false; }(), "Unsupported pointer type");
        }

        auto callback =
            [actor, f = std::forward<Func>(fn)]() mutable {
            BOOST_ASSERT_MSG(actor->TestSynchronize(),
                "Running in other thread");

            f(*static_cast<Derived*>(actor.get()));
        };

        boost::asio::post(actor->io_executor(), std::move(callback));
    }

    Type actor_ = nullptr;
};

template <typename Type>
class Controller<Type&> {
public:
    using Actor = std::remove_cvref_t<Type>;

    explicit Controller(Type& actor) noexcept
        : actor_(actor) {
    }

    template <typename Func>
    void Post(Func&& fn) {
        static_assert(!is_smart_ptr<Actor>::value,
            "Controller<T&> does not support smart pointer types");

        auto callback =
            [actor = std::addressof(actor_),
            f = std::forward<Func>(fn)]() mutable {
            f(*actor);
            };

        boost::asio::post(actor_.io_executor(), std::move(callback));
    }

private:
    Actor& actor_;
};
} // namespace ACtor

#ifndef CONTROLLER
#define CONTROLLER(x) system::Controller<decltype(x)>(x)
#endif
