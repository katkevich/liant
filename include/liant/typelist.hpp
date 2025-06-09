#pragma once
#include <array>
#include <cstddef>
#include <type_traits>

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

    template <typename TTypeFn>
    static constexpr void forEach(TTypeFn fn) {
        // call lambda expression which have templated call operator
        (fn.template operator()<Ts>(), ...);
    }
};

template <typename TTypeList, typename... Us>
struct TypeListAppend;

template <typename... Ts, typename... Us>
struct TypeListAppend<TypeList<Ts...>, Us...> {
    using type = TypeList<Ts..., Us...>;
};

template <typename TTypeList, typename... Us>
using TypeListAppendT = typename TypeListAppend<TTypeList, Us...>::type;

template <typename... Ts>
static constexpr bool AlwaysFalsePrint = false;

template <bool Condition, typename... Ts>
static constexpr bool ConditionalPrint = Condition;
} // namespace liant