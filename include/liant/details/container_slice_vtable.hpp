#pragma once
#include "liant/container.hpp"
#include "liant/details/container_ptr.hpp"
#include "liant/details/container_slice_settings.hpp"
#include "liant/details/dynamic_array.hpp"

#ifndef LIANT_MODULE
#include <cstring>
#endif

namespace liant::details {

template <typename TInterface>
struct VTableItem {
    TInterface* (*findRawErased)(liant::ContainerBase* container);
    TInterface& (*resolveRawErased)(liant::ContainerBase* container);
};

template <typename... TInterfaces>
struct VTable : public VTableItem<TInterfaces>... {};

template <typename TContainer, typename... TInterfaces>
static constexpr VTable<TInterfaces...> vtableForContainer = { VTableItem<TInterfaces>{
    [](liant::ContainerBase* container) -> TInterfaces* {
        return static_cast<TContainer*>(container)->template findRaw<TInterfaces>();
    },
    [](liant::ContainerBase* container) -> TInterfaces& {
        return static_cast<TContainer*>(container)->template resolveRaw<TInterfaces>();
    },
}... };

using VTableErased = const void*;
using NestedVTableErased = const void*;

template <typename TInterface>
struct NestedVTableItem {
    auto (*atErased)(const liant::DynamicArray<NestedVTableErased>& nestedVTablesErased, std::size_t idx, VTableErased vtableErased)
        -> const VTableItem<TInterface>*;
};

template <typename... TInterfaces>
struct NestedVTable : public NestedVTableItem<TInterfaces>... {};

template <typename TVTable, typename... TInterfaces>
static constexpr NestedVTable<TInterfaces...> nestedVTableForVTable = { NestedVTableItem<TInterfaces>{
    [](const liant::DynamicArray<NestedVTableErased>&, std::size_t, VTableErased vtableErased) -> const VTableItem<TInterfaces>* {
        const auto* vtable = static_cast<const TVTable*>(vtableErased);
        const auto* vtableItem = static_cast<const VTableItem<TInterfaces>*>(vtable);
        return vtableItem;
    },
}... };

template <typename TNestedVTable, typename... TInterfaces>
static constexpr NestedVTable<TInterfaces...> nestedVTableForNestedVTable = { NestedVTableItem<TInterfaces>{
    [](const liant::DynamicArray<NestedVTableErased>& nestedVTablesErased, std::size_t idx, VTableErased vtableErased) -> const VTableItem<TInterfaces>* {
        const NestedVTableItem<TInterfaces>* nestedVTableItem = static_cast<const TNestedVTable*>(nestedVTablesErased[idx]);
        return nestedVTableItem->atErased(nestedVTablesErased, idx - 1, vtableErased);
    },
}... };

template <typename... TInterfaces>
struct ContainerSliceVTable {
    template <typename... UInterfaces>
    ContainerSliceVTable(const VTable<UInterfaces...>* vtable)
        : vtable(vtable)
        , nestedVTables(&nestedVTableForVTable<VTable<UInterfaces...>, TInterfaces...>) {}

    template <typename... UInterfaces>
    ContainerSliceVTable(VTableErased vtable, const liant::DynamicArray<NestedVTableErased>& nestedVTables, TypeList<UInterfaces...>)
        : vtable(vtable)
        , nestedVTables(nestedVTables, &nestedVTableForNestedVTable<NestedVTable<UInterfaces...>, TInterfaces...>) {}

    ContainerSliceVTable(const ContainerSliceVTable&) = default;
    ContainerSliceVTable(ContainerSliceVTable&&) = default;
    ContainerSliceVTable& operator=(const ContainerSliceVTable&) = default;
    ContainerSliceVTable& operator=(ContainerSliceVTable&&) = default;

    template <typename TInterface>
    TInterface* findRaw(liant::ContainerBase* container) const {
        const auto nestedVTablesLen = nestedVTables.size();

        const auto* nestedVTable = static_cast<const NestedVTable<TInterfaces...>*>(nestedVTables[nestedVTablesLen - 1]);
        const auto* nestedVTableItem = static_cast<const NestedVTableItem<TInterface>*>(nestedVTable);

        const VTableItem<TInterface>* vtableItem = (*nestedVTableItem->atErased)(nestedVTables, nestedVTablesLen - 2, vtable);
        TInterface* containerInterface = (*vtableItem->findRawErased)(container);

        return containerInterface;
    }

    template <typename TInterface>
    TInterface& resolveRaw(liant::ContainerBase* container) const {
        const auto nestedVTablesLen = nestedVTables.size();

        const auto* nestedVTable = static_cast<const NestedVTable<TInterfaces...>*>(nestedVTables[nestedVTablesLen - 1]);
        const auto* nestedVTableItem = static_cast<const NestedVTableItem<TInterface>*>(nestedVTable);

        const VTableItem<TInterface>* vtableItem = (*nestedVTableItem->atErased)(nestedVTables, nestedVTablesLen - 2, vtable);
        TInterface& containerInterface = (*vtableItem->resolveRawErased)(container);

        return containerInterface;
    }

    VTableErased vtable{};
    liant::DynamicArray<NestedVTableErased> nestedVTables;
};
} // namespace liant::details