#pragma once
#include "liant/details/container_slice_impl.hpp"
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
class ContainerSlice : public details::ContainerSliceImpl<details::SharedOwnership, ContainerSlice<TInterfaces...>> {
public:
    using details::ContainerSliceImpl<details::SharedOwnership, ContainerSlice>::ContainerSliceImpl;

    explicit operator bool() const {
        return this->container.operator bool();
    }
    auto useCount() const {
        return this->container.asShared().use_count();
    }
};

// subset of another type-erased DI container (shared ownership, no automatic resolving upon the construction)
template <typename... TInterfaces>
class ContainerSliceLazy
    : public details::ContainerSliceImpl<details::SharedOwnershipLazy, ContainerSliceLazy<TInterfaces...>> {
public:
    using details::ContainerSliceImpl<details::SharedOwnershipLazy, ContainerSliceLazy>::ContainerSliceImpl;

    explicit operator bool() const {
        return this->container.operator bool();
    }
    auto useCount() const {
        return this->container.asShared().use_count();
    }
};

// subset of another type-erased DI container (weak ownership)
template <typename... TInterfaces>
class ContainerSliceWeak : private details::ContainerSliceImpl<details::WeakOwnership, ContainerSliceWeak<TInterfaces...>> {
public:
    using details::ContainerSliceImpl<details::WeakOwnership, ContainerSliceWeak>::ContainerSliceImpl;

    ContainerSlice<TInterfaces...> lock() const {
        return ContainerSlice<TInterfaces...>(*this);
    }
};


// subset of another type-erased DI container (weak ownership, no automatic resolving upon the construction)
template <typename... TInterfaces>
class ContainerSliceWeakLazy
    : private details::ContainerSliceImpl<details::WeakOwnershipLazy, ContainerSliceWeakLazy<TInterfaces...>> {
public:
    using details::ContainerSliceImpl<details::WeakOwnershipLazy, ContainerSliceWeakLazy>::ContainerSliceImpl;

    ContainerSliceLazy<TInterfaces...> lock() const {
        return ContainerSliceLazy<TInterfaces...>(*this);
    }
};
} // namespace liant