#pragma once

// Provides a getter method with pretty name for your Interface while accessing it from DI container object
//
// LIANT_DEPENDENCY(HttpClient, httpClient)
// ...
// liant::ContainerView<HttpClient, ...> di;
// di.httpClient();
//
// Note: LIANT_DEPENDENCY macro can be used from your namespace as well - no need to open liant:: namespace
#define LIANT_DEPENDENCY(Interface, getterPrettyName)                                                   \
    template <typename TContainer>                                                                      \
    class LiantPrettyDependency_##Interface : public liant::Dependency<Interface> {                     \
    public:                                                                                             \
        Interface& getterPrettyName##Raw() {                                                            \
            return static_cast<TContainer*>(this)->template resolveRaw<Interface>();                    \
        }                                                                                               \
        const Interface* getterPrettyName##Raw() const {                                                \
            return static_cast<const TContainer*>(this)->template findRaw<Interface>();                 \
        }                                                                                               \
        liant::SharedRef<Interface> getterPrettyName() {                                                \
            return static_cast<TContainer*>(this)->template resolve<Interface>();                       \
        }                                                                                               \
        liant::SharedPtr<Interface> getterPrettyName() const {                                          \
            return static_cast<const TContainer*>(this)->template find<Interface>();                    \
        }                                                                                               \
    };                                                                                                  \
                                                                                                        \
    /* exploit ADL lookup */                                                                            \
    template <typename TContainer>                                                                      \
    auto liantPrettyDependencyLookup(liant::TypeIdentity<TContainer>, liant::TypeIdentity<Interface>) { \
        return liant::TypeIdentity<LiantPrettyDependency_##Interface<TContainer>>{};                    \
    }