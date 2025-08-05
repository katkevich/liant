#pragma once
#include "liant/details/container_ptr.hpp"
#include "liant/details/container_slice_vtable.hpp"

#ifndef LIANT_MODULE
#include <memory>
#endif

namespace liant::details {

template <OwnershipKind OwnershipOther, typename U>
struct FactoryVTable {
    U (*make)(const ContainerSliceVTableErased& sliceVTable, const ContainerPtr<OwnershipOther>& container);
    std::unique_ptr<U> (*makeUnique)(const ContainerSliceVTableErased& sliceVTable, const ContainerPtr<OwnershipOther>& container);
    std::shared_ptr<U> (*makeShared)(const ContainerSliceVTableErased& sliceVTable, const ContainerPtr<OwnershipOther>& container);
};

template <OwnershipKind OwnershipOther, typename U, typename UContainerSlice>
static constexpr FactoryVTable<OwnershipOther, U> factoryVTableFor = {
    [](const ContainerSliceVTableErased& sliceVTable, const ContainerPtr<OwnershipOther>& container) -> U {
        return U{ UContainerSlice(sliceVTable, container) };
    },
    [](const ContainerSliceVTableErased& sliceVTable, const ContainerPtr<OwnershipOther>& container) -> std::unique_ptr<U> {
        return std::make_unique<U>(UContainerSlice(sliceVTable, container));
    },
    [](const ContainerSliceVTableErased& sliceVTable, const ContainerPtr<OwnershipOther>& container) -> std::shared_ptr<U> {
        return std::make_shared<U>(UContainerSlice(sliceVTable, container));
    },
};

template <OwnershipKind Ownership, typename T>
class FactoryImpl {
public:
    template <typename UContainerSlice>
    explicit FactoryImpl(const UContainerSlice& slice)
        : vtable(&factoryVTableFor<Ownership, T, UContainerSlice>)
        , sliceVTable(slice.vtable)
        , container(slice.container) {}

    FactoryImpl(const FactoryImpl&) = default;
    FactoryImpl(FactoryImpl&&) = default;
    FactoryImpl& operator=(const FactoryImpl&) = default;
    FactoryImpl& operator=(FactoryImpl&&) = default;

    [[nodiscard]] T make() const {
        return (*vtable->make)(sliceVTable, container);
    }

    [[nodiscard]] std::shared_ptr<T> makeShared() const {
        return (*vtable->makeShared)(sliceVTable, container);
    }

    [[nodiscard]] std::unique_ptr<T> makeUnique() const {
        return (*vtable->makeUnique)(sliceVTable, container);
    }

private:
    const FactoryVTable<Ownership, T>* vtable{};
    ContainerSliceVTableErased sliceVTable;
    ContainerPtr<Ownership> container{};
};
} // namespace liant::details

// clang-format off
LIANT_EXPORT
// clang-format on
namespace liant {
template <typename T>
class Factory : public details::FactoryImpl<details::OwnershipKind::Shared, T> {
public:
    using details::FactoryImpl<details::OwnershipKind::Shared, T>::FactoryImpl;
};

template <typename T>
class FactoryView : public details::FactoryImpl<details::OwnershipKind::RawRef, T> {
public:
    using details::FactoryImpl<details::OwnershipKind::RawRef, T>::FactoryImpl;
};
} // namespace liant
