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

// MSVC compiler (19.38) doesn't like it to be defined inside ContainerSliceImpl class
// So unfortunately, we're forced to move these guts out of ContainerSliceImpl class scope
template <typename TContainer, typename... TInterfaces>
static constexpr std::tuple<VTable<TInterfaces>...> vtablesFor = { VTable<TInterfaces>{
    [](const VTable<TInterfaces>& self, ContainerBase& container) -> TInterfaces* {
        return self.findRaw(static_cast<TContainer&>(container));
    },
    [](const VTable<TInterfaces>& self, ContainerBase& container) -> TInterfaces& {
        return self.resolveRaw(static_cast<TContainer&>(container));
    },
}... };


// This is another layer of indirection after VTable<Interface>
// The thing is that liant::ContainerSlice may be created from either liant::Container or from another liant::ContainerSlice.
// And in each case we have different kinds of vtables:
// - while we're creating ContainerSlice from Container we need to erase Container (i.e. vtable which erases liant::Container methods)
// - while we're creating ContainerSlice from ContainerSlice we need to erase ContainerSlice (i.e. vtable which erases liant::ContainerSlice methods)
// So we need to be able to abstract away the vtables themselves - hence this VTables class.
template <typename... TInterfaces>
class VTables {
    template <typename... UInterfaces>
    friend class VTables;

    template <typename TInterface>
    using VTableGetter = auto (*)(const void* vtablesTupleErased, const void* vtableGettersTupleErased)
        -> const VTable<TInterface>&;

    template <typename VTablesTuple>
    static constexpr std::tuple<VTableGetter<TInterfaces>...> vtableGettersFromContainerVTables = { //
        [](const void* vtablesTupleErased, const void*) -> const VTable<TInterfaces>& {
            return std::get<VTable<TInterfaces>>(*static_cast<const VTablesTuple*>(vtablesTupleErased));
        }...
    };

    template <typename VTableGettersTuple>
    static constexpr std::tuple<VTableGetter<TInterfaces>...> vtableGettersFromSliceVTableGetters = { //
        [](const void* vtablesTupleErased, const void* vtableGettersTupleErased) -> const VTable<TInterfaces>& {
            auto* vtableGettersTuple = static_cast<const VTableGettersTuple*>(vtableGettersTupleErased);
            auto& vtableGetter = std::get<VTableGetter<TInterfaces>>(*vtableGettersTuple);

            // delegate getting the vtable to the "vtableGettersFromContainerVTables"
            return vtableGetter(vtablesTupleErased, nullptr);
        }...
    };

public:
    // Create ContainerSlice vtables from Container vtables
    // "{Ts...} set should be included in {Us...}"
    template <typename... UInterfaces>
    VTables(const std::tuple<VTable<UInterfaces>...>* containerVTables)
        : vtableGetters(std::addressof(vtableGettersFromContainerVTables<std::tuple<VTable<UInterfaces>...>>))
        , vtablesTupleErased(containerVTables) {}


    // Create ContainerSlice vtables from another ContainerSlice vtables
    // "{Ts...} set should be included in {Us...}"
    template <typename... UInterfaces>
    VTables(const VTables<UInterfaces...>& sliceVTables)
        : vtableGetters(std::addressof(vtableGettersFromSliceVTableGetters<std::tuple<VTableGetter<UInterfaces>...>>))
        , vtablesTupleErased(sliceVTables.vtablesTupleErased)
        , vtableGettersTupleErased(sliceVTables.vtableGetters) {}

    // Create ContainerSlice vtables from another ContainerSlice vtables
    // "{Ts...} set should be included in {Us...}"
    template <typename... UInterfaces>
    VTables& operator=(const VTables<UInterfaces...>& sliceVTables) {
        vtableGetters = std::addressof(vtableGettersFromSliceVTableGetters<std::tuple<VTableGetter<UInterfaces>...>>);
        vtablesTupleErased = sliceVTables.vtablesTupleErased;
        vtableGettersTupleErased = sliceVTables.vtableGetters;

        return *this;
    }

    VTables(const VTables&) = default;
    VTables& operator=(const VTables&) = default;

    template <typename TInterface>
    const VTable<TInterface>& get() const {
        auto& vtableGetter = std::get<VTableGetter<TInterface>>(*vtableGetters);
        return vtableGetter(vtablesTupleErased, vtableGettersTupleErased);
    }

private:
    const std::tuple<VTableGetter<TInterfaces>...>* vtableGetters{};

    // vtables for erased liant::Container methods
    const void* vtablesTupleErased{};
    // vtable getters of a "parent" liant::ContainerSlice which this liant::ContainerSlice was constructed from
    // under the hood these getters will delegate vtables resolving to the root "vtablesTupleErased"
    const void* vtableGettersTupleErased{};
};

// common base class for liant::ContainerSlice (owning) & liant::ContainerView (non-owning)
template <ContainerPtrKind PtrKind, typename... TInterfaces>
class ContainerSliceImpl : public liant::PrettyDependency<ContainerSliceImpl<PtrKind, TInterfaces...>, TInterfaces>... {
    template <typename UBaseContainer, typename... UTypeMappings>
    friend class liant::Container;

    template <ContainerPtrKind PtrKindOther, typename... UInterfaces>
    friend class ContainerSliceImpl;

    // 'VTables<...>' is another layer of type-erasure indirection
    // 'VTable<TInterface>' erases liant::Container interface
    // 'VTables<TInterfaces...>' erases vtables themselves (it could be Container vtable or ContainerSlice vtable under the hood)
    VTables<TInterfaces...> vtables{};

public:
    template <typename UBaseContainer, typename... UTypeMappings>
    ContainerSliceImpl(const std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>& container)
        : vtables(std::addressof(vtablesFor<Container<UBaseContainer, UTypeMappings...>, TInterfaces...>))
        , container(container) {
        resolveAll();
    }
    template <typename UBaseContainer, typename... UTypeMappings>
    ContainerSliceImpl(std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>&& container)
        : vtables(std::addressof(vtablesFor<Container<UBaseContainer, UTypeMappings...>, TInterfaces...>))
        , container(std::move(container)) {
        resolveAll();
    }

    template <ContainerPtrKind PtrKindOther, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl(const ContainerSliceImpl<PtrKindOther, UInterfaces...>& other)
        : vtables(other.vtables)
        , container(other.container) {}

    template <ContainerPtrKind PtrKindOther, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl(ContainerSliceImpl<PtrKindOther, UInterfaces...>&& other)
        : vtables(other.vtables)
        , container(std::move(other.container)) {}

    template <ContainerPtrKind PtrKindOther, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl& operator=(const ContainerSliceImpl<PtrKindOther, UInterfaces...>& other) {
        vtables = other.vtables;
        container = other.container;
        return *this;
    }

    template <ContainerPtrKind PtrKindOther, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl& operator=(ContainerSliceImpl<PtrKindOther, UInterfaces...>&& other) {
        vtables = std::move(other.vtables);
        container = std::move(other.container);
        return *this;
    }

    ContainerSliceImpl(const ContainerSliceImpl&) = default;
    ContainerSliceImpl(ContainerSliceImpl&&) = default;
    ContainerSliceImpl& operator=(const ContainerSliceImpl&) = default;
    ContainerSliceImpl& operator=(ContainerSliceImpl&&) = default;

    // trying to find already created instance registered 'as TInterface'
    // the unsafe raw pointer is being returned here so make sure it doesn't outlive the underlying 'Container'
    template <typename TInterface>
    TInterface* findRaw() const {
        static_assert(liant::PrintConditional<TypeList<TInterfaces...>::template contains<TInterface>(), TInterface>,
            "Interface you're trying to find is missing from ContainerSlice<...> / "
            "ContainerView<...> (search 'liant::Print' in the compilation output for details)");

        const VTable<TInterface>& vtableItem = vtables.template get<TInterface>();
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
        const VTable<TInterface>& vtableItem = vtables.template get<TInterface>();
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

    const ContainerSliceImpl* operator->() const {
        return this;
    }

    ContainerSliceImpl* operator->() {
        return this;
    }

protected:
    // underlying container
    ContainerPtr<PtrKind> container;
};
} // namespace liant::details