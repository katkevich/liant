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
    template <typename TBaseContainer, typename... TTypeMappings>
    friend class Container;

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

    // trying to find already created instance registered 'as TInterface'
    // the unsafe raw pointer is being returned here so make sure it doesn't outlive the underlying 'Container'
    template <typename TInterface>
    TInterface* findRaw() const {
        static_assert(liant::PrintConditional<TypeList<TInterfaces...>::template contains<TInterface>(), TInterface>,
            "You're trying to find an interface which isn't specified in 'TInterfaces...' list of 'ContainerSlice' "
            "(search 'liant::Print' in the compilation output for details)");

        const VTable<TInterface>& vtableItem = std::get<VTable<TInterface>>(*vtable);
        return (*vtableItem.findRawErased)(vtableItem, *container);
    }

    // trying to find already created instance registered 'as TInterface'
    // returned fat 'SharedRef' protects underlying 'Container' from being destroyed so use 'SharedRef' with caution (you don't really want block 'Container' deletion)
    template <typename TInterface>
    SharedPtr<TInterface> find() const {
        return SharedPtr<TInterface>(findRaw<TInterface>(), container);
    }

    // resolve an instance of type registered 'as TInterface'
    // returned fat 'SharedRef' protects 'Container' from being destroyed so use 'SharedRef' with caution (you don't really want block 'Container' deletion)
    //
    // all dependencies will be created automatically, cycles are detected automatically
    // if such instance already exists then just return it, statically casted to 'TInterface'
    template <typename TInterface>
    SharedRef<TInterface> resolve() {
        static_assert(liant::PrintConditional<TypeList<TInterfaces...>::template contains<TInterface>(), TInterface>,
            "You're trying to resolve an interface which isn't specified in 'TInterfaces...' list of 'ContainerSlice' "
            "(search 'liant::Print' in the compilation output for details)");

        return SharedRef<TInterface>(std::addressof(resolveRaw<TInterface>()), container);
    }

    void resolveAll() {
        container->resolveAll();
    }

private:
    // resolve an instance of type registered 'as TInterface'
    // the unsafe raw reference is being returned here so make sure it doesn't outlive the underlying 'Container'
    //
    // all dependencies will be created automatically, cycles are detected automatically
    // if such instance already exists then just return it, statically casted to 'TInterface'
    template <typename TInterface, typename... TArgs>
    TInterface& resolveRaw(TArgs&&...) {
        static_assert(liant::PrintConditional<sizeof...(TArgs) == 0, TInterface>,
            "Cannot resolve an interface from a 'liant::Container<...>' through 'liant::ContainerSlice<...>' using "
            "explicit arguments (ContainerSlice::resolveRaw(ExplicitArguments...)). The reason is that "
            "'liant::ContainerSlice<...>' erases 'liant::Container<...>' type under the hood which means that "
            "information about concrete 'Types' behinds 'Interfaces' within "
            "'liant::Container<...>' is erased as well. Obviously you cannot instantiate a 'Type' using "
            "'resolveRaw<Interface>(TArgs...)' without knowing this 'Type'");

        const VTable<TInterface>& vtableItem = std::get<VTable<TInterface>>(*vtable);
        return (*vtableItem.resolveRawErased)(vtableItem, *container);
    }

    const ContainerSlice* operator->() const {
        return this;
    }

    ContainerSlice* operator->() {
        return this;
    }

private:
    // underlying container
    std::shared_ptr<ContainerBase> container;
};

} // namespace liant