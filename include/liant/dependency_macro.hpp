#pragma once

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
