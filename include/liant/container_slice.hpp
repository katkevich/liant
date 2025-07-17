#pragma once
#include "liant/container_slice_base.hpp"
#include "liant/export_macro.hpp"

#ifndef LIANT_MODULE
#include <memory>
#endif

// clang-format off
LIANT_EXPORT
// clang-format on
namespace liant {

// subset of another type-erased DI container (shared ownership)
template <typename... TInterfaces>
class ContainerSlice : public details::ContainerSliceBase<details::ContainerPtrKind::Shared, TInterfaces...> {
public:
    using details::ContainerSliceBase<details::ContainerPtrKind::Shared, TInterfaces...>::ContainerSliceBase;

    explicit operator bool() const {
        return static_cast<bool>(this->container.inner);
    }

    auto useCount() const {
        return this->container.owner().use_count();
    }
};

template <typename... TInterfaces>
class ContainerSliceWeak {
public:
    template <typename UBaseContainer, typename... UTypeMappings>
    ContainerSliceWeak(std::weak_ptr<Container<UBaseContainer, UTypeMappings...>> weakContainer)
        : weakContainer(std::move(weakContainer)) {}

    ContainerSlice<TInterfaces...> lock() const {
        return ContainerSlice<TInterfaces...>(weakContainer.lock());
    }

private:
    // underlying container
    std::weak_ptr<ContainerBase> weakContainer;
};
} // namespace liant