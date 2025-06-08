#pragma once
#include <exception>
#include <stdexcept>
#include <type_traits>

namespace liant::missing_dependency_policy {

template <typename T, typename = void>
struct is_policy : std::false_type {};

template <typename T>
struct is_policy<T, std::void_t<typename T::IsPolicy>> : std::true_type {};

template<typename T>
static constexpr bool is_policy_v = is_policy<T>::value;

struct TerminatePolicy {
    using IsPolicy = void;

    template <typename TInterface, typename TContainer>
    TInterface& handleMissingInstance(TContainer&) const {
        std::terminate();
    }
};

struct ThrowPolicy {
    using IsPolicy = void;

    template <typename TInterface, typename TContainer>
    TInterface& handleMissingInstance(TContainer&) const {
        throw std::runtime_error("missing dependency");
    }
};

template <typename TCallback>
struct HandlerPolicy {
    using IsPolicy = void;

    HandlerPolicy(TCallback callback)
        : callback(callback) {}

    template <typename TInterface, typename TContainer>
    TInterface& handleMissingInstance(TContainer& container) const {
        return callback.template operator()<TInterface, TContainer>(container);
    }

    TCallback callback;
};

// note:
// 'TInterface&' object which is being returned from 'callback()' must outlive DI container
// DI container DO NOT manage the lifetime of this returned object
template <typename TCallback>
auto handler(TCallback callback) {
    return HandlerPolicy<TCallback>(callback);
}

static constexpr TerminatePolicy Terminate;
static constexpr ThrowPolicy Throw;
} // namespace liant::missing_dependency_policy