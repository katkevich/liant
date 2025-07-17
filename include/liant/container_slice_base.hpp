#pragma once
#include "liant/container.hpp"

#ifndef LIANT_MODULE
#include <memory>
#endif


// clang-format off
LIANT_EXPORT
// clang-format on
namespace liant {

template <typename T>
class Dependency {};

template <typename TContainer, typename T>
auto liantPrettyDependencyLookup(liant::TypeIdentity<TContainer>, liant::TypeIdentity<T>) {
    return liant::TypeIdentity<Dependency<T>>{};
}

template <typename TContainer, typename T>
using PrettyDependency =
    typename decltype(liantPrettyDependencyLookup(liant::TypeIdentity<TContainer>{}, liant::TypeIdentity<T>{}))::type;
} // namespace liant


namespace liant::details {
enum class ContainerPtrKind { Shared, RawRef };

template <ContainerPtrKind Kind>
struct ContainerPtr;

template <>
struct ContainerPtr<ContainerPtrKind::RawRef> {
    liant::ContainerBase* inner{};

    ContainerPtr() = default;
    ContainerPtr(const ContainerPtr&) = default;
    ContainerPtr(ContainerPtr&&) = default;
    ContainerPtr& operator=(const ContainerPtr&) = default;
    ContainerPtr& operator=(ContainerPtr&&) = default;

    template <ContainerPtrKind PtrKindOther>
        requires(PtrKindOther == ContainerPtrKind::Shared)
    ContainerPtr(const ContainerPtr<PtrKindOther>& containerPtr)
        : inner(containerPtr.inner.get()) {}

    ContainerPtr(std::shared_ptr<ContainerBase> container)
        : inner(container.get()) {}

    ContainerBase* raw() const {
        return inner;
    }
    std::shared_ptr<ContainerBase> owner() const {
        return inner->shared_from_this();
    }
};

template <>
struct ContainerPtr<ContainerPtrKind::Shared> {
    std::shared_ptr<ContainerBase> inner{};

    ContainerPtr() = default;
    ContainerPtr(const ContainerPtr&) = default;
    ContainerPtr(ContainerPtr&&) = default;
    ContainerPtr& operator=(const ContainerPtr&) = default;
    ContainerPtr& operator=(ContainerPtr&&) = default;

    template <ContainerPtrKind PtrKindOther>
        requires(PtrKindOther == ContainerPtrKind::RawRef)
    ContainerPtr(const ContainerPtr<PtrKindOther>& containerPtr)
        : inner(containerPtr.owner()) {}

    ContainerPtr(std::shared_ptr<ContainerBase> container)
        : inner(std::move(container)) {}

    ContainerBase* raw() const {
        return inner.get();
    }
    const std::shared_ptr<ContainerBase>& owner() const {
        return inner;
    }
};

template <typename TInterface>
struct VTable {
    auto (*findRawErased)(const VTable<TInterface>& self, liant::ContainerBase& container) -> TInterface*;
    auto (*resolveRawErased)(const VTable<TInterface>& self, liant::ContainerBase& container) -> TInterface&;

    template <typename TContainer>
    TInterface* findRaw(TContainer& container) const {
        return container.template findRaw<TInterface>();
    }

    template <typename TContainer>
    TInterface& resolveRaw(TContainer& container) const {
        return container.template resolveRaw<TInterface>();
    }
};

// MSVC compiler (19.38) doesn't like it to be defined inside ContainerSliceBase class
// So unfortunately, we're forced to move these guts out of ContainerSliceBase class scope
template <typename TContainer, typename... TInterfaces>
static constexpr std::tuple<VTable<TInterfaces>...> vtableFor = { VTable<TInterfaces>{
    [](const VTable<TInterfaces>& self, ContainerBase& container) -> TInterfaces* {
        return self.findRaw(static_cast<TContainer&>(container));
    },
    [](const VTable<TInterfaces>& self, ContainerBase& container) -> TInterfaces& {
        return self.resolveRaw(static_cast<TContainer&>(container));
    },
}... };

// common base class for liant::ContainerSlice (owning) & liant::ContainerView (non-owning)
template <ContainerPtrKind PtrKind, typename... TInterfaces>
class ContainerSliceBase : public liant::PrettyDependency<ContainerSliceBase<PtrKind, TInterfaces...>, TInterfaces>... {
    template <typename UBaseContainer, typename... UTypeMappings>
    friend class liant::Container;

    template <ContainerPtrKind PtrKindOther, typename... UInterfaces>
    friend class ContainerSliceBase;

    const std::tuple<VTable<TInterfaces>...>* vtable{};

public:
    template <typename UBaseContainer, typename... UTypeMappings>
    ContainerSliceBase(const std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>& container)
        : vtable(std::addressof(vtableFor<Container<UBaseContainer, UTypeMappings...>, TInterfaces...>))
        , container(container) {
        resolveAll();
    }
    template <typename UBaseContainer, typename... UTypeMappings>
    ContainerSliceBase(std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>&& container)
        : vtable(std::addressof(vtableFor<Container<UBaseContainer, UTypeMappings...>, TInterfaces...>))
        , container(std::move(container)) {
        resolveAll();
    }

    template <ContainerPtrKind PtrKindOther>
    ContainerSliceBase(const ContainerSliceBase<PtrKindOther, TInterfaces...>& containerSlice)
        : vtable(containerSlice.vtable)
        , container(containerSlice.container) {}

    ContainerSliceBase(const ContainerSliceBase&) = default;
    ContainerSliceBase(ContainerSliceBase&&) = default;
    ContainerSliceBase& operator=(const ContainerSliceBase&) = default;
    ContainerSliceBase& operator=(ContainerSliceBase&&) = default;

    // trying to find already created instance registered 'as TInterface'
    // the unsafe raw pointer is being returned here so make sure it doesn't outlive the underlying 'Container'
    template <typename TInterface>
    TInterface* findRaw() const {
        static_assert(liant::PrintConditional<TypeList<TInterfaces...>::template contains<TInterface>(), TInterface>,
            "Interface you're trying to find is missing from ContainerSlice<...> / "
            "ContainerView<...> (search 'liant::Print' in the compilation output for details)");

        const VTable<TInterface>& vtableItem = std::get<VTable<TInterface>>(*vtable);
        return (*vtableItem.findRawErased)(vtableItem, *container.raw());
    }

    // trying to find already created instance registered 'as TInterface'
    // returned fat 'SharedRef' protects underlying 'Container' from being destroyed so use 'SharedRef' with caution (you don't really want block 'Container' deletion)
    template <typename TInterface>
    SharedPtr<TInterface> find() const {
        return SharedPtr<TInterface>(findRaw<TInterface>(), container.owner());
    }

    // resolve an instance of type registered 'as TInterface'
    // the unsafe raw reference is being returned here so make sure it doesn't outlive the underlying 'Container'
    //
    // all dependencies will be created automatically, cycles are detected automatically
    // if such instance already exists then just return it, statically casted to 'TInterface'
    template <typename TInterface>
    TInterface& resolveRaw() {
        const VTable<TInterface>& vtableItem = std::get<VTable<TInterface>>(*vtable);
        return (*vtableItem.resolveRawErased)(vtableItem, *container.raw());
    }

    // resolve an instance of type registered 'as TInterface'
    // returned fat 'SharedRef' protects 'Container' from being destroyed so use 'SharedRef' with caution (you don't really want block 'Container' deletion)
    //
    // all dependencies will be created automatically, cycles are detected automatically
    // if such instance already exists then just return it, statically casted to 'TInterface'
    template <typename TInterface>
    SharedRef<TInterface> resolve() {
        static_assert(liant::PrintConditional<TypeList<TInterfaces...>::template contains<TInterface>(), TInterface>,
            "Interface you're trying to resolve is missing from 'liant::ContainerSlice<...>' / "
            "'liant::ContainerView<...>' (search 'liant::Print' in the compilation output for details)");

        return SharedRef<TInterface>(resolveRaw<TInterface>(), container.owner());
    }

    void resolveAll() {
        TypeList<TInterfaces...>::forEach([&]<typename TInterface>() { //
            resolveRaw<TInterface>();
        });
    }

private:
    template <typename TInterface, typename TArg, typename... TArgs>
    TInterface& resolveRaw(TArg&&, TArgs&&...) {
        static_assert(liant::Print<TInterface>,
            "Cannot resolve an interface from a 'liant::Container<...>' through 'liant::ContainerSlice<...>' using "
            "explicit arguments (ContainerSlice::resolveRaw(ExplicitArguments...)). The reason is that "
            "'liant::ContainerSlice<...>' erases 'liant::Container<...>' type under the hood which means that "
            "concrete 'Types' behind 'Interfaces' within 'liant::Container<...>' are erased as well. Hence "
            "cannot instantiate a 'Type' using 'resolveRaw<Interface>(TArgs...)' without knowing this 'Type'");

        return this->resolveRaw();
    }

    const ContainerSliceBase* operator->() const {
        return this;
    }

    ContainerSliceBase* operator->() {
        return this;
    }

protected:
    // underlying container
    ContainerPtr<PtrKind> container;
};
} // namespace liant::details