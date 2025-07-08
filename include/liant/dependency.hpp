#pragma once
#include "liant/export_macro.hpp"
#include "liant/container.hpp"
#include "liant/typelist.hpp"

#ifndef LIANT_MODULE
#include <memory>
#endif

// clang-format off
LIANT_EXPORT
// clang-format on
namespace liant {

// default 'Dependency' implementation without custom 'pretty' getters
template <typename T>
class Dependency {
    template <typename... TInterfaces>
    friend class Dependencies;

public:
    using Interface = T;
    using interface_type = Interface;

    T* operator->() {
        return std::addressof(dependency);
    }

    const T* operator->() const {
        return std::addressof(dependency);
    }

protected:
    Dependency(T& dependency)
        : dependency(dependency) {}

    T& dependency;
};

// default "lookup" just returns 'Dependency<T>' as is
// custom lookups are generated using LIANT_DEPENDENCY(...) macro and will be found via ADL
template <typename T>
auto liantPrettyDependencyLookup(liant::TypeIdentity<T>) {
    return TypeIdentity<Dependency<T>>{};
}

template <typename T>
using PrettyDependency = typename decltype(liantPrettyDependencyLookup(liant::TypeIdentity<T>{}))::type;


// subset of dependencies from DI root container
// each of you dependencies will have an instance of this class, thereby referring to its own dependencies
template <typename... TInterfaces>
class Dependencies : public PrettyDependency<TInterfaces>... {
public:
    using Interfaces = TypeList<TInterfaces...>;
    using interfaces_type = Interfaces;

    template <typename UBaseContainer, typename... UTypeMappings>
    Dependencies(Container<UBaseContainer, UTypeMappings...>& container)
        : PrettyDependency<TInterfaces>{ container.template resolveRaw<TInterfaces>() }... {}

    template <typename TInterface>
    auto& get() {
        static_assert(TypeList<TInterfaces...>::template contains<TInterface>(),
            "interface you're trying to get is not registered in DI container");
        return *static_cast<PrettyDependency<TInterface>*>(this)->dependency;
    }

    template <typename TInterface>
    const auto& get() const {
        static_assert(TypeList<TInterfaces...>::template contains<TInterface>(),
            "interface you're trying to get is not registered in DI container");
        return *static_cast<const PrettyDependency<TInterface>*>(this)->dependency;
    }
};

} // namespace liant