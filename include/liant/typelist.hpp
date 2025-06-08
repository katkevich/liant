#pragma once
#include <cstddef>
#include <type_traits>
#include <array>

namespace liant {

template <typename T>
struct TypeIdentity {};

template <typename... Ts>
struct TypeList {
    template <typename T>
    static constexpr bool contains() {
        return (std::is_same_v<T, Ts> || ...);
    }

    template <typename TTypePredicate>
    static constexpr std::ptrdiff_t find(TTypePredicate pred) {
        // call lambda expression which have templated call operator
        constexpr std::array found = { pred.template operator()<Ts>()... };
        for (std::ptrdiff_t i = 0; i < found.size(); ++i) {
            if (found[i]) {
                return i;
            }
        }
        return -1;
    }
};
} // namespace liant