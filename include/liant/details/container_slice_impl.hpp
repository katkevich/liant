#pragma once
#include "liant/container.hpp"
#include "liant/details/container_ptr.hpp"
#include "liant/details/container_slice_settings.hpp"
#include "liant/details/container_slice_vtable.hpp"
#include "liant/factory.hpp"

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
template <typename TTraits, typename TSelf>
class ContainerSliceImpl;

// common base class for
// - liant::ContainerSlice (owning)
// - liant::ContainerView (non-owning)
// - liant::ContainerSliceLazy (owning, no automatic resolving in ctor - resolve upon request)
// - liant::ContainerViewLazy (non-owning, no automatic resolving in ctor - resolve upon request)
template <typename TTraits, template <typename...> typename TSelf, typename... TInterfaces>
class ContainerSliceImpl<TTraits, TSelf<TInterfaces...>>
    : public std::conditional_t<TTraits::Resolve == ResolveMode::Ctor,
          PrettyDependency<ContainerSliceImpl<TTraits, TSelf<TInterfaces...>>, TInterfaces>,
          PrettyDependencyLazy<ContainerSliceImpl<TTraits, TSelf<TInterfaces...>>, TInterfaces>>... {
    // `Container<...>` should be able to access private `operator->`
    template <typename UBaseContainer, typename... UTypeMappings>
    friend class liant::Container;

    // `FactoryImpl<...>` should be able to access private `ContainerSliceImpl(vtable, container)` ctor
    template <OwnershipKind OwnershipOther, typename U>
    friend class FactoryImpl;

    // should be able to access private members of `ContainerSliceImpl<...>` with different specializations
    template <typename UTraits, typename USelf>
    friend class ContainerSliceImpl;

private:
    template <OwnershipKind OwnershipOther>
    ContainerSliceImpl(const ContainerSliceVTableErased& vtable, const ContainerPtr<OwnershipOther>& container)
        : vtable(vtable)
        , container(container) {
        if constexpr (TTraits::Resolve == ResolveMode::Ctor) {
            resolveAllChecked();
        }
    }

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
    template <typename UBaseContainer, typename... UTypeMappings>
    ContainerSliceImpl(const Container<UBaseContainer, UTypeMappings...>& container)
        : vtable(&vtableForContainer<Container<UBaseContainer, UTypeMappings...>, TInterfaces...>)
        , container(container.shared_from_this()) {
        if constexpr (TTraits::Resolve == ResolveMode::Ctor) {
            resolveAllChecked();
        }
    }

    template <typename UTraits, template <typename...> typename USelf>
    ContainerSliceImpl(const ContainerSliceImpl<UTraits, USelf<TInterfaces...>>& other)
        : vtable(other.vtable)
        , container(other.container) {
        if constexpr (TTraits::Resolve == ResolveMode::Ctor && UTraits::Resolve == ResolveMode::Lazy) {
            resolveAllChecked();
        }
    }

    template <typename UTraits, template <typename...> typename USelf>
    ContainerSliceImpl(ContainerSliceImpl<UTraits, USelf<TInterfaces...>>&& other)
        : vtable(std::move(other.vtable))
        , container(std::move(other.container)) {
        if constexpr (TTraits::Resolve == ResolveMode::Ctor && UTraits::Resolve == ResolveMode::Lazy) {
            resolveAllChecked();
        }
    }

    template <typename UTraits, template <typename...> typename USelf, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl(const ContainerSliceImpl<UTraits, USelf<UInterfaces...>>& other)
        : vtable(other.vtable.vtable, other.vtable.nestedVTables, TypeList<UInterfaces...>{})
        , container(other.container) {
        if constexpr (TTraits::Resolve == ResolveMode::Ctor && UTraits::Resolve == ResolveMode::Lazy) {
            resolveAllChecked();
        }
    }

    template <typename UTraits, template <typename...> typename USelf, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl(ContainerSliceImpl<UTraits, USelf<UInterfaces...>>&& other)
        : vtable(other.vtable.vtable, other.vtable.nestedVTables, TypeList<UInterfaces...>{})
        , container(std::move(other.container)) {
        if constexpr (TTraits::Resolve == ResolveMode::Ctor && UTraits::Resolve == ResolveMode::Lazy) {
            resolveAllChecked();
        }
    }

    template <typename UTraits, template <typename...> typename USelf>
    ContainerSliceImpl& operator=(const ContainerSliceImpl<UTraits, USelf<TInterfaces...>>& other) {
        vtable = other.vtable;
        container = other.container;

        if constexpr (TTraits::Resolve == ResolveMode::Ctor && UTraits::Resolve == ResolveMode::Lazy) {
            resolveAllChecked();
        }
        return *this;
    }

    template <typename UTraits, template <typename...> typename USelf>
    ContainerSliceImpl& operator=(ContainerSliceImpl<UTraits, USelf<TInterfaces...>>&& other) {
        vtable = std::move(other.vtable);
        container = std::move(other.container);

        if constexpr (TTraits::Resolve == ResolveMode::Ctor && UTraits::Resolve == ResolveMode::Lazy) {
            resolveAllChecked();
        }
        return *this;
    }

    template <typename UTraits, template <typename...> typename USelf, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl& operator=(const ContainerSliceImpl<UTraits, USelf<UInterfaces...>>& other) {
        vtable = ContainerSliceVTable<TInterfaces...>(other.vtable.vtable, other.vtable.nestedVTables, TypeList<UInterfaces...>{});
        container = other.container;

        if constexpr (TTraits::Resolve == ResolveMode::Ctor && UTraits::Resolve == ResolveMode::Lazy) {
            resolveAllChecked();
        }
        return *this;
    }

    template <typename UTraits, template <typename...> typename USelf, typename... UInterfaces>
        requires liant::IsSubsetOf<TypeList<TInterfaces...>, TypeList<UInterfaces...>>::value
    ContainerSliceImpl& operator=(ContainerSliceImpl<UTraits, USelf<UInterfaces...>>&& other) {
        vtable = ContainerSliceVTable<TInterfaces...>(other.vtable.vtable, other.vtable.nestedVTables, TypeList<UInterfaces...>{});
        container = std::move(other.container);

        if constexpr (TTraits::Resolve == ResolveMode::Ctor && UTraits::Resolve == ResolveMode::Lazy) {
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
    [[nodiscard]] TInterface* findRaw() const {
        static_assert(liant::PrintConditional<TypeList<TInterfaces...>::template contains<TInterface>(), TInterface>,
            "Interface you're trying to find is missing from ContainerSlice<...> / "
            "ContainerView<...> (search 'liant::Print' in the compilation output for details)");

        return vtable.template findRaw<TInterface>(container.asRaw());
    }

    // trying to find already created instance registered 'as TInterface'
    // returned fat 'SharedRef' protects underlying 'Container' from being destroyed so use 'SharedRef' with caution (you don't really want block 'Container' deletion)
    template <typename TInterface>
    [[nodiscard]] SharedPtr<TInterface> find() const {
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

    template <typename U>
    [[nodiscard]] Factory<U> makeFactory() const {
        return Factory<U>(asSelf());
    }

    template <typename U>
    [[nodiscard]] FactoryView<U> makeFactoryView() const {
        return FactoryView<U>(asSelf());
    }

    template <typename U>
    [[nodiscard]] U make() const {
        return U{ asSelf() };
    }

    template <typename U, typename... TArgs>
    [[nodiscard]] std::shared_ptr<U> makeShared(TArgs&&... args) const {
        return std::make_shared<U>(asSelf(), std::forward<TArgs>(args)...);
    }

    template <typename U, typename... TArgs>
    [[nodiscard]] std::unique_ptr<U> makeUnique(TArgs&&... args) const {
        return std::make_unique<U>(asSelf(), std::forward<TArgs>(args)...);
    }

private:
    const TSelf<TInterfaces...>& asSelf() const {
        return static_cast<const TSelf<TInterfaces...>&>(*this);
    }

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

private:
    ContainerSliceVTable<TInterfaces...> vtable;

protected:
    // underlying container
    ContainerPtr<TTraits::Ownership> container;
};
} // namespace liant::details