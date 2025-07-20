#pragma once
#include "liant/container.hpp"
#include "liant/details/container_ptr.hpp"
#include "liant/details/container_settings.hpp"

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
    const VTableItem<TInterface>* (*atErased)(NestedVTableErased* nestedVTablesErased, std::size_t idx, VTableErased vtableErased);
};

template <typename... TInterfaces>
struct NestedVTable : public NestedVTableItem<TInterfaces>... {};

template <typename TVTable, typename... TInterfaces>
static constexpr NestedVTable<TInterfaces...> nestedVTableForVTable = { NestedVTableItem<TInterfaces>{
    [](NestedVTableErased*, std::size_t, VTableErased vtableErased) -> const VTableItem<TInterfaces>* {
        const auto* vtable = static_cast<const TVTable*>(vtableErased);
        const auto* vtableItem = static_cast<const VTableItem<TInterfaces>*>(vtable);
        return vtableItem;
    },
}... };

template <typename TNestedVTable, typename... TInterfaces>
static constexpr NestedVTable<TInterfaces...> nestedVTableForNestedVTable = { NestedVTableItem<TInterfaces>{
    [](NestedVTableErased* nestedVTablesErased, std::size_t idx, VTableErased vtableErased) -> const VTableItem<TInterfaces>* {
        const NestedVTableItem<TInterfaces>* nestedVTableItem = static_cast<const TNestedVTable*>(nestedVTablesErased[idx]);
        return nestedVTableItem->atErased(nestedVTablesErased, idx - 1, vtableErased);
    },
}... };

template <typename... TInterfaces>
struct ContainerSliceVTable {
    template <typename... UInterfaces>
    ContainerSliceVTable(const VTable<UInterfaces...>* vtable)
        : vtable(vtable) {
        this->nestedVTables = new NestedVTableErased[2];
        this->nestedVTables[0] = &nestedVTableForVTable<VTable<UInterfaces...>, TInterfaces...>;
        this->nestedVTables[1] = NestedVTableErased{};
    }

    template <typename... UInterfaces>
    ContainerSliceVTable(VTableErased vtable, NestedVTableErased* nestedVTables, TypeList<UInterfaces...>)
        : vtable(vtable) {
        const std::size_t nestedVTablesLen = arrayLength(nestedVTables);
        this->nestedVTables = new NestedVTableErased[nestedVTablesLen + 2];

        std::memcpy(this->nestedVTables, nestedVTables, nestedVTablesLen * sizeof(NestedVTableErased));
        this->nestedVTables[nestedVTablesLen] = &nestedVTableForNestedVTable<NestedVTable<UInterfaces...>, TInterfaces...>;
        this->nestedVTables[nestedVTablesLen + 1] = NestedVTableErased{};
    }
    ContainerSliceVTable(const ContainerSliceVTable& other)
        : vtable(other.vtable) {
        const std::size_t nestedVTablesLen = arrayLength(other.nestedVTables);
        this->nestedVTables = new NestedVTableErased[nestedVTablesLen + 1];

        std::memcpy(this->nestedVTables, other.nestedVTables, nestedVTablesLen * sizeof(NestedVTableErased));
        this->nestedVTables[nestedVTablesLen] = NestedVTableErased{};
    }
    ContainerSliceVTable(ContainerSliceVTable&& other) noexcept
        : vtable(other.vtable)
        , nestedVTables(other.nestedVTables) {
        other.nestedVTables = {};
    }
    ContainerSliceVTable& operator=(ContainerSliceVTable other) noexcept {
        using std::swap;
        swap(this->vtable, other.vtable);
        swap(this->nestedVTables, other.nestedVTables);

        return *this;
    }
    ~ContainerSliceVTable() {
        if (nestedVTables) {
            delete[] nestedVTables;
        }
    }

    template <typename TInterface>
    TInterface* findRaw(liant::ContainerBase* container) const {
        auto idx = arrayLength(nestedVTables);

        const auto* nestedVTable = static_cast<const NestedVTable<TInterfaces...>*>(nestedVTables[idx - 1]);
        const auto* nestedVTableItem = static_cast<const NestedVTableItem<TInterface>*>(nestedVTable);

        const VTableItem<TInterface>* vtableItem = (*nestedVTableItem->atErased)(nestedVTables, idx - 2, vtable);
        TInterface* containerInterface = (*vtableItem->findRawErased)(container);

        return containerInterface;
    }

    template <typename TInterface>
    TInterface& resolveRaw(liant::ContainerBase* container) const {
        auto idx = arrayLength(nestedVTables);

        const auto* nestedVTable = static_cast<const NestedVTable<TInterfaces...>*>(nestedVTables[idx - 1]);
        const auto* nestedVTableItem = static_cast<const NestedVTableItem<TInterface>*>(nestedVTable);

        const VTableItem<TInterface>* vtableItem = (*nestedVTableItem->atErased)(nestedVTables, idx - 2, vtable);
        TInterface& containerInterface = (*vtableItem->resolveRawErased)(container);

        return containerInterface;
    }

    static std::size_t arrayLength(NestedVTableErased* array) noexcept {
        std::size_t length = 0;
        while (array[length]) {
            length++;
        }
        return length;
    }

    VTableErased vtable{};
    NestedVTableErased* nestedVTables{};
};
} // namespace liant::details