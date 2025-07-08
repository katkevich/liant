#pragma once
#include "liant/export_macro.hpp"

#ifndef LIANT_MODULE
#include <memory>
#endif

// clang-format off
LIANT_EXPORT
// clang-format on
namespace liant {

// subset of another type-erased DI container
template <typename... TInterfaces>
class ContainerSlice {
    template <typename TInterface>
    struct VTable {
        auto (*findRawErased)(const VTable<TInterface>& self, ContainerBase& container) -> TInterface*;
        auto (*resolveRawErased)(const VTable<TInterface>& self, ContainerBase& container) -> TInterface&;

        template <typename TContainer>
        TInterface* findRaw(TContainer& container) const {
            return container.template findRaw<TInterface>();
        }

        template <typename TContainer>
        TInterface& resolveRaw(TContainer& container) const {
            return container.template resolveRaw<TInterface>();
        }
    };

    const std::tuple<VTable<TInterfaces>...>* vtable{};

    template <typename TContainer>
    static constexpr std::tuple<VTable<TInterfaces>...> vtableFor = { VTable<TInterfaces>{
        +[](const VTable<TInterfaces>& self, ContainerBase& container) -> TInterfaces* {
            return self.findRaw(static_cast<TContainer&>(container));
        },
        +[](const VTable<TInterfaces>& self, ContainerBase& container) -> TInterfaces& {
            return self.resolveRaw(static_cast<TContainer&>(container));
        },
    }... };

public:
    template <typename UBaseContainer, typename... UTypeMappings>
    ContainerSlice(const std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>& container)
        : vtable(std::addressof(vtableFor<Container<UBaseContainer, UTypeMappings...>>))
        , container(container) {}


    const ContainerSlice* operator->() const {
        return this;
    }

    ContainerSlice* operator->() {
        return this;
    }

    template <typename TInterface>
    TInterface* findRaw() const {
        static_assert(liant::PrintConditional<TypeList<TInterfaces...>::template contains<TInterface>(), TInterface>,
            "You're trying to find an interface which isn't specified in 'TInterfaces...' list of 'ContainerSlice' "
            "(search 'liant::Print' in the compilation output for details)");

        const VTable<TInterface>& vtableItem = std::get<VTable<TInterface>>(*vtable);
        return (*vtableItem.findRawErased)(vtableItem, *container);
    }

    template <typename TInterface, typename... TArgs>
    TInterface& resolveRaw() {
        static_assert(liant::PrintConditional<TypeList<TInterfaces...>::template contains<TInterface>(), TInterface>,
            "You're trying to resolve an interface which isn't specified in 'TInterfaces...' list of 'ContainerSlice' "
            "(search 'liant::Print' in the compilation output for details)");

        static_assert(liant::PrintConditional<sizeof...(TArgs) == 0, TInterface>,
            "Cannot resolve an interface from a 'liant::Container<...>' through 'liant::ContainerSlice<...>' using "
            "explicit arguments. The reason is that 'liant::ContainerSlice<...>' erases 'liant::Container<...>' type "
            "under the hood which means that information about concrete 'Types' behinds 'Interfaces' within "
            "'liant::Container<...>' is erased as well. Obviously you cannot instantiate a 'Type' using "
            "'resolveRaw<Interface>(TArgs...)' without knowing this 'Type'");

        const VTable<TInterface>& vtableItem = std::get<VTable<TInterface>>(*vtable);
        return (*vtableItem.resolveRawErased)(vtableItem, *container);
    }

    void resolveAll() {
        container->resolveAll();
    }

private:
    std::shared_ptr<ContainerBase> container;
};

} // namespace liant