#pragma once
#include <tuple>

namespace liant::tuple {
template <typename TFn, typename... Ts>
void forEach(std::tuple<Ts...>& tpl, TFn fn) {
    [&]<std::size_t... Is>(std::index_sequence<Is...>) { //
        (fn(std::get<Is>(tpl)), ...);
    }(std::index_sequence_for<Ts...>{});
}
} // namespace liant::tuple