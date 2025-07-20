#pragma once
#include "liant/container.hpp"
#include "liant/details/container_ptr.hpp"
#include "liant/details/container_slice_settings.hpp"
#include "liant/details/container_slice_vtable.hpp"

#ifndef LIANT_MODULE
#include <cstring>
#include <memory>
#include <type_traits>
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
auto liantPrettyDependencyLazyLookup(liant::TypeIdentity<TContainer>, liant::TypeIdentity<T>) {
    return liant::TypeIdentity<Dependency<T>>{};
}

template <typename TContainer, typename T>
using PrettyDependency =
    typename decltype(liantPrettyDependencyLookup(liant::TypeIdentity<TContainer>{}, liant::TypeIdentity<T>{}))::type;

template <typename TContainer, typename T>
using PrettyDependencyLazy =
    typename decltype(liantPrettyDependencyLazyLookup(liant::TypeIdentity<TContainer>{}, liant::TypeIdentity<T>{}))::type;
} // namespace liant


namespace liant::details {
// common base class for
// - liant::ContainerSlice (owning)
// - liant::ContainerView (non-owning)
// - liant::ContainerSliceLazy (owning, no automatic resolving in ctor - resolve upon request)
// - liant::ContainerViewLazy (non-owning, no automatic resolving in ctor - resolve upon request)
template <typename TTraits, typename... TInterfaces>
class ContainerSliceImpl : public std::conditional_t<TTraits::Resolve == ResolveMode::Ctor,
                               liant::PrettyDependency<ContainerSliceImpl<TTraits, TInterfaces...>, TInterfaces>,
                               liant::PrettyDependencyLazy<ContainerSliceImpl<TTraits, TInterfaces...>, TInterfaces>>... {
    template <typename UBaseContainer, typename... UTypeMappings>
    friend class liant::Container;

    template <typename UTraits, typename... UInterfaces>
    friend class ContainerSliceImpl;

    ContainerSliceVTable<TInterfaces...> vtable;

public:
    template <typename UBaseContainer, typename... UTypeMappings>
    ContainerSliceImpl(const std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>& container)
        : vtable(&vtableForContainer<Container<UBaseContainer, UTypeMappings...>, TInterfaces...>)
        , container(container) {
        if constexpr (TTraits::Resolve == ResolveMode::Ctor) {
            resolveAllChecked();
        }
    }
    template <typename UBaseContainer, typename... UTypeMappings>
    ContainerSliceImpl(std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>&& container)
        : vtable(&vtableForContainer<Container<UBaseContainer, UTypeMappings...>, TInterfaces...>)
        , container(std::move(container)) {
        if constexpr (TTraits::Resolve == ResolveMode::Ctor) {
            resolveAllChecked();
        }
    }

    template <typename UTraits>
    ContainerSliceImpl(const ContainerSliceImpl<UTraits, TInterfaces...>& other)
        : vtable(other.vtable)
        , container(other.container) {
        if constexpr (TTraits::Resolve == ResolveMode::Ctor && UTraits::Resolve == ResolveMode::Lazy) {
            resolveAllChecked();
        }
    }

    template <typename UTraits>
    ContainerSliceImpl(ContainerSliceImpl<UTraits, TInterfaces...>&& other)
        : vtable(std::move(other.vtable))
        , container(std::move(other.container)) {
        if constexpr (TTraits::ResolveMode == ResolveMode::Ctor && UTraits::ResolveMode == ResolveMode::Lazy) {
            resolveAllChecked();
        }
    }

    template <typename UTraits, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl(const ContainerSliceImpl<UTraits, UInterfaces...>& other)
        : vtable(other.vtable.vtable, other.vtable.nestedVTables, TypeList<UInterfaces...>{})
        , container(other.container) {
        if constexpr (TTraits::Resolve == ResolveMode::Ctor && UTraits::Resolve == ResolveMode::Lazy) {
            resolveAllChecked();
        }
    }

    template <typename UTraits, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl(ContainerSliceImpl<UTraits, UInterfaces...>&& other)
        : vtable(other.vtable.vtable, other.vtable.nestedVTables, TypeList<UInterfaces...>{})
        , container(std::move(other.container)) {
        if constexpr (TTraits::ResolveMode == ResolveMode::Ctor && UTraits::ResolveMode == ResolveMode::Lazy) {
            resolveAllChecked();
        }
    }

    template <typename UTraits, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl& operator=(const ContainerSliceImpl<UTraits, UInterfaces...>& other) {
        vtable = ContainerSliceVTable<TInterfaces...>(other.vtable.vtable, other.vtable.nestedVTables, TypeList<UInterfaces...>{});
        container = other.container;

        if constexpr (TTraits::ResolveMode == ResolveMode::Ctor && UTraits::ResolveMode == ResolveMode::Lazy) {
            resolveAllChecked();
        }
        return *this;
    }

    template <typename UTraits, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl& operator=(ContainerSliceImpl<UTraits, UInterfaces...>&& other) {
        vtable = ContainerSliceVTable<TInterfaces...>(other.vtable.vtable, other.vtable.nestedVTables, TypeList<UInterfaces...>{});
        container = std::move(other.container);

        if constexpr (TTraits::ResolveMode == ResolveMode::Ctor && UTraits::ResolveMode == ResolveMode::Lazy) {
            resolveAllChecked();
        }
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

        return vtable.template findRaw<TInterface>(container.asRaw());
    }

    // trying to find already created instance registered 'as TInterface'
    // returned fat 'SharedRef' protects underlying 'Container' from being destroyed so use 'SharedRef' with caution (you don't really want block 'Container' deletion)
    template <typename TInterface>
    SharedPtr<TInterface> find() const {
        return SharedPtr<TInterface>(findRaw<TInterface>(), container.asShared());
    }

    // resolve an instance of type registered 'as TInterface'
    // the unsafe raw reference is being returned here so make sure it doesn't outlive the underlying 'Container'
    //
    // all dependencies will be created automatically, cycles are detected automatically
    // if such instance already exists then just return it, statically casted to 'TInterface'
    template <typename TInterface>
    TInterface& resolveRaw() {
        static_assert(liant::PrintConditional<TypeList<TInterfaces...>::template contains<TInterface>(), TInterface>,
            "Interface you're trying to resolve is missing from ContainerSlice<...> / "
            "ContainerView<...> (search 'liant::Print' in the compilation output for details)");

        return vtable.template resolveRaw<TInterface>(container.asRaw());
    }

    // resolve an instance of type registered 'as TInterface'
    // returned fat 'SharedRef' protects 'Container' from being destroyed so use 'SharedRef' with caution (you don't really want block 'Container' deletion)
    //
    // all dependencies will be created automatically, cycles are detected automatically
    // if such instance already exists then just return it, statically casted to 'TInterface'
    template <typename TInterface>
    SharedRef<TInterface> resolve() {
        return SharedRef<TInterface>(resolveRaw<TInterface>(), container.asShared());
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

    void resolveAllChecked() {
        if (this->container) {
            resolveAll();
        }
    }

    const ContainerSliceImpl* operator->() const {
        return this;
    }

    ContainerSliceImpl* operator->() {
        return this;
    }

protected:
    // underlying container
    ContainerPtr<TTraits::Ownership> container;
};
} // namespace liant::details