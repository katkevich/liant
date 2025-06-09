#pragma once
#include "liant/container.hpp"
#include "liant/typelist.hpp"

namespace liant {

template <typename T>
class Dependency {
    template <typename... TTypes>
    friend class Dependencies;

    Dependency(T& dependency)
        : dependency(dependency) {}

    T& dependency;
};

// subset of dependencies from DI root container
// each of you dependencies will have an instance of this class, thereby referring to its own dependencies
template <typename... TTypes>
class Dependencies : public Dependency<TTypes>... {
public:
    using Interfaces = TypeList<TTypes...>;

    template <typename... UTypeMappings>
    Dependencies(Container<UTypeMappings...>& container)
        : Dependency<TTypes>{ container.template findChecked<TTypes>() }... {}

    template <typename TType>
    auto& get() {
        static_assert(TypeList<TTypes...>::template contains<TType>(),
            "type you're trying to get is not registered in DI container");
        return *static_cast<Dependency<TType>*>(this)->dependency;
    }

    template <typename TType>
    const auto& get() const {
        static_assert(TypeList<TTypes...>::template contains<TType>(),
            "type you're trying to get is not registered in DI container");
        return *static_cast<const Dependency<TType>*>(this)->dependency;
    }
};

} // namespace liant

#define LIANT_DEPENDENCY(Type, getterMethodName) \
    namespace liant {                            \
                                                 \
    template <>                                  \
    class Dependency<Type> {                     \
        template <typename... TTypes>            \
        friend class Dependencies;               \
                                                 \
        Dependency(Type& dependency)             \
            : dependency(dependency) {}          \
                                                 \
        Type& dependency;                        \
                                                 \
    public:                                      \
        Type& getterMethodName() {               \
            return dependency;                   \
        }                                        \
        const Type& getterMethodName() const {   \
            return dependency;                   \
        }                                        \
    };                                           \
    }
