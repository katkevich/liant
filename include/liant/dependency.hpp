#pragma once
#include "liant/container.hpp"
#include "liant/typelist.hpp"
#include <memory>

namespace liant {

// default 'Dependency' implementation without custom 'pretty' getters
template <typename T>
class Dependency {
    template <typename... TTypes>
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
using PrettyDependency = decltype(liantPrettyDependencyLookup(liant::TypeIdentity<T>{}))::type;


// subset of dependencies from DI root container
// each of you dependencies will have an instance of this class, thereby referring to its own dependencies
template <typename... TTypes>
class Dependencies : public PrettyDependency<TTypes>... {
public:
    using Interfaces = TypeList<TTypes...>;
    using interfaces_type = Interfaces;

    template <typename UBaseContainer, typename... UTypeMappings>
    Dependencies(Container<UBaseContainer, UTypeMappings...>& container)
        : PrettyDependency<TTypes>{ container.template resolveRaw<TTypes>() }... {}

    template <typename TType>
    auto& get() {
        static_assert(TypeList<TTypes...>::template contains<TType>(),
            "type you're trying to get is not registered in DI container");
        return *static_cast<PrettyDependency<TType>*>(this)->dependency;
    }

    template <typename TType>
    const auto& get() const {
        static_assert(TypeList<TTypes...>::template contains<TType>(),
            "type you're trying to get is not registered in DI container");
        return *static_cast<const PrettyDependency<TType>*>(this)->dependency;
    }
};

} // namespace liant

// no need to open liant:: namespace in order to define custom Pretty dependency
// 'liantPrettyDependencyLookup' uses ADL in order to find correct Pretty dependency customization point
#define LIANT_DEPENDENCY(Type, getterPrettyName)                          \
    class LiantPrettyDependency_##Type : public liant::Dependency<Type> { \
        template <typename... UTypes>                                     \
        friend class Dependencies;                                        \
                                                                          \
        using Dependency<Type>::Dependency;                               \
                                                                          \
    public:                                                               \
        Type& getterPrettyName() {                                        \
            return dependency;                                            \
        }                                                                 \
        const Type& getterPrettyName() const {                            \
            return dependency;                                            \
        }                                                                 \
    };                                                                    \
                                                                          \
    /* exploit ADL lookup */                                              \
    auto liantPrettyDependencyLookup(liant::TypeIdentity<Type>) {         \
        return liant::TypeIdentity<LiantPrettyDependency_##Type>{};       \
    }
