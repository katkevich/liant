#pragma once
#include "liant/export_macro.hpp"

#ifndef LIANT_MODULE
#include <array>
#include <cstddef>
#include <type_traits>
#endif

// clang-format off
LIANT_EXPORT
// clang-format on
namespace liant {

struct Nothing {};

template <typename T>
struct TypeIdentity {
    using type = T;
};


template <typename... Ts>
struct TypeList {
    template <typename T>
    static constexpr bool contains() {
        return (std::is_same_v<T, Ts> || ...);
    }

    template <typename... Us>
    static constexpr bool containsAll() {
        return (TypeList<Ts...>::template contains<Us>() && ...);
    }

    template <typename TTypePredicate>
    static constexpr std::ptrdiff_t find(TTypePredicate pred) {
        if constexpr (sizeof...(Ts) > 0) {
            constexpr std::array found = { pred.template operator()<Ts>()... };
            for (std::size_t i = 0; i < found.size(); ++i) {
                if (found[i]) {
                    return static_cast<std::ptrdiff_t>(i);
                }
            }
        }
        return -1;
    }

    static constexpr std::ptrdiff_t findDuplicate() {
        return find([]<typename T>() { return ((std::is_same_v<T, Ts> ? 1 : 0) + ... + 0) >= 2; });
    }

    template <typename TTypeFn>
    static constexpr void forEach(TTypeFn fn) {
        // call lambda expression which have templated call operator
        (fn.template operator()<Ts>(), ...);
    }


    template <std::ptrdiff_t Index>
    static constexpr auto at() {
        if constexpr (Index == -1 || sizeof...(Ts) == 0) {
            return Nothing{};
        } else {
            return atImpl<Index, Ts...>();
        }
    }

private:
    template <std::ptrdiff_t Index, typename U, typename... Us>
    static constexpr auto atImpl() {
        if constexpr (Index == 0) {
            return TypeIdentity<U>{};
        } else if (sizeof...(Us) > 0) {
            return atImpl<Index - 1, Us...>();
        } else {
            return Nothing{};
        }
    }
};

template <typename TSubset, typename TSet>
struct IsSubsetOf;

template <typename... Ts, typename... Us>
struct IsSubsetOf<TypeList<Ts...>, TypeList<Us...>> : std::bool_constant<TypeList<Us...>::template containsAll<Ts...>()> {};


template <typename TTypeList, typename... Us>
struct TypeListAppend;

template <typename... Ts, typename... Us>
struct TypeListAppend<TypeList<Ts...>, Us...> {
    using type = TypeList<Ts..., Us...>;
};

template <typename TTypeList, typename... Us>
using TypeListAppendT = typename TypeListAppend<TTypeList, Us...>::type;


template <typename... TTypeLists>
struct TypeListMerge {
    using type = TypeList<>;
};

template <typename... Ts>
struct TypeListMerge<TypeList<Ts...>> {
    using type = TypeList<Ts...>;
};

template <typename... Ts, typename... Us, typename... TTypeListsTail>
struct TypeListMerge<TypeList<Ts...>, TypeList<Us...>, TTypeListsTail...> {
    using type = TypeListMerge<TypeList<Ts..., Us...>, TTypeListsTail...>::type;
};

template <typename... TTypeLists>
using TypeListMergeT = typename TypeListMerge<TTypeLists...>::type;
} // namespace liant

namespace liant {
template <typename... Ts>
static constexpr bool Print = false;

template <bool Condition, typename... Ts>
static constexpr bool PrintConditional = Condition;
} // namespace liant