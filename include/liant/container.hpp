#pragma once
#include "liant/tuple.hpp"
#include "liant/typelist.hpp"

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

namespace liant {

enum class ItemLifetime { External, DI };

template <ItemLifetime LifetimeV, typename T, typename TInterfacesTypeList, typename TCtorArgsTypeList>
struct TypeMapping;

template <ItemLifetime LifetimeV, typename T, typename... TInterfaces, typename... TCtorArgs>
struct TypeMapping<LifetimeV, T, TypeList<TInterfaces...>, TypeList<TCtorArgs...>> {
    static_assert((std::is_base_of_v<TInterfaces, T> && ...), "Type T must be derived from each of the TInterfaces");

    using Type = T;
    using Interfaces = TypeList<TInterfaces...>;
    using CtorArgs = TypeList<TCtorArgs...>;
    using CtorArgsTuple = std::tuple<TCtorArgs...>;
    static constexpr ItemLifetime Lifetime = LifetimeV;
};


template <typename TTypeMapping>
class RegisteredItem {
public:
    using Mapping = TTypeMapping;

    RegisteredItem() = default;

    template <typename... TCtorArgs>
    RegisteredItem(typename TTypeMapping::Type* item, TCtorArgs&&... ctorArgs)
        : item(item)
        , ctorArgs(std::forward<TCtorArgs>(ctorArgs)...) {}
    RegisteredItem(typename TTypeMapping::Type* item, TTypeMapping::CtorArgsTuple&& ctorArgs)
        : item(item)
        , ctorArgs(std::move(ctorArgs)) {}

    template <typename... TInterfaces>
    auto as() && {
        constexpr ItemLifetime Lifetime = TTypeMapping::Lifetime;
        using Type = TTypeMapping::Type;
        using Interfaces = TypeList<TInterfaces...>;
        using CtorArgs = TTypeMapping::CtorArgs;
        using NextTypeMapping = TypeMapping<Lifetime, Type, Interfaces, CtorArgs>;

        return RegisteredItem<NextTypeMapping>{ item, std::move(ctorArgs) };
    }

    template <typename... TCtorArgs>
    auto bindArgs(TCtorArgs&&... ctorArgs) && {
        constexpr ItemLifetime Lifetime = TTypeMapping::Lifetime;
        using Type = TTypeMapping::Type;
        using Interfaces = TTypeMapping::Interfaces;
        using CtorArgs = TypeList<std::decay_t<TCtorArgs>...>;
        using NextTypeMapping = TypeMapping<Lifetime, Type, Interfaces, CtorArgs>;

        return RegisteredItem<NextTypeMapping>{ item, std::forward<TCtorArgs>(ctorArgs)... };
    }

private:
    template <typename... TTypeMappings>
    friend class Container;

    template <typename TInterface>
    auto get() {
        return static_cast<TInterface*>(item);
    }

    // TDependenciesHolder may be either 'Container<...>' itself or 'Container<...>::DependenciesCtorHook'
    template <typename TInterface, typename TDependenciesHolder, typename... TArgs>
    auto& instantiate(TDependenciesHolder& dependenciesHolder, TArgs&&... args) {
        if constexpr (std::is_constructible_v<typename TTypeMapping::Type, TDependenciesHolder&, TArgs&&...>) {
            item = new TTypeMapping::Type(dependenciesHolder, std::forward<TArgs>(args)...);
        } else {
            item = new TTypeMapping::Type(std::forward<TArgs>(args)...);
        }

        if constexpr (requires { item->postCreate(); }) {
            item->postCreate();
        }

        return static_cast<TInterface&>(*item);
    }

    void destroy() {
        if constexpr (requires { item->preDestroy(); }) {
            item->preDestroy();
        }
        delete item;
    }

private:
    TTypeMapping::Type* item{};
    TTypeMapping::CtorArgsTuple ctorArgs{};
};

template <typename T>
auto registerType() {
    return RegisteredItem<TypeMapping<ItemLifetime::DI, T, TypeList<T>, TypeList<>>>{};
}

template <typename T>
auto registerItem(T& item) {
    return RegisteredItem<TypeMapping<ItemLifetime::External, T, TypeList<T>, TypeList<>>>{ std::addressof(item) };
}

template <typename... TTypes>
class Dependencies;

class ContainerBase {
public:
    virtual ~ContainerBase() = default;
};

template <typename... TTypeMappings>
class Container : public ContainerBase {
    using DestroyItemFn = void (*)(Container&);

public:
    Container(RegisteredItem<TTypeMappings>... items)
        : items{ items... } {
        deleters.reserve(sizeof...(TTypeMappings));
    }
    ~Container() {
        // destroy items in the order opposite to the creation order
        for (auto it = deleters.rbegin(); it != deleters.rend(); ++it) {
            (*it)(*this);
        }
    }

    // trying to find already created instance registered 'as TInterface'
    template <typename TInterface>
    TInterface* find() {
        auto& item = findItem<TInterface>();

        return item.template get<TInterface>();
    }

    // resolve an instance of type registered 'as TInterface'
    // all dependencies will be created automatically, cycles are detected automatically
    // if such instance already exists then just return it, statically casted to 'TInterface'
    template <typename TInterface, typename... TArgs>
    TInterface& resolve(TArgs&&... args) {
        auto& item = findItem<TInterface>();
        return instantiateOne<TInterface>(item, TypeList<>{}, std::forward<TArgs>(args)...);
    }

    // resolve all registered instances automatically
    // order is determined automatically, cycles are detected automatically
    // note: your types should satisfy some preconditions:
    // - registered type should be default-constructible
    // - or registered type should be only constructible from 'Dependencies<...>'
    // - or you should provide ctor arguments while calling 'Container::resolve<...>'
    // - or you should provide ctor arguments bindings while configuring DI mappings (bindArgs(...))
    void resolveAll() {
        liant::tuple::forEach(items, [&]<typename TTypeMapping>(RegisteredItem<TTypeMapping>&) {
            instantiateAll(typename TTypeMapping::Interfaces{}, TypeList<>{});
        });
    }

private:
    template <typename TDependenciesChain>
    struct DependenciesCtorHook {
        template <typename... TInterfaces>
        operator Dependencies<TInterfaces...>() {
            container.instantiateAll(TypeList<TInterfaces...>{}, TDependenciesChain{});
            return Dependencies<TInterfaces...>{ container };
        }

        Container& container;
    };

    // TDependenciesChain is there to detect dependencies cycles at compile time
    template <typename TDependenciesChain, typename... TInterfaces>
    void instantiateAll(TypeList<TInterfaces...>, TDependenciesChain dependenciesChain) {
        TypeList<TInterfaces...>::template forEach([&]<typename TInterface>() { //
            instantiateOne<TInterface>(findItem<TInterface>(), dependenciesChain);
        });
    }

    template <typename TInterface, typename TRegisteredItem, typename TDependenciesChain, typename... TArgs>
    TInterface& instantiateOne(TRegisteredItem& item, TDependenciesChain, TArgs&&... args) {
        // ensure items are only instantiated once
        if (TInterface* existingInterface = item.template get<TInterface>()) {
            return *existingInterface;
        }

        static_assert(!TDependenciesChain::template contains<TInterface>(), "Detected cycle in your dependencies");
        using DependenciesChainNext = TypeListAppendT<TDependenciesChain, TInterface>;

        if constexpr (TRegisteredItem::Mapping::Lifetime == ItemLifetime::DI) {
            // use directly provided ctor arguments instead of the binded ones
            if constexpr (sizeof...(TArgs) > 0) {
                if constexpr (std::is_constructible_v<typename TRegisteredItem::Mapping::Type, Container&, TArgs&&...>) {
                    // hook into dependencies creation logic and ensure dependencies of those dependencies are created first
                    DependenciesCtorHook<DependenciesChainNext> dependenciesCtorHook{ *this };
                    return instantiate<TInterface>(item, dependenciesCtorHook, std::forward<TArgs>(args)...);
                } else if constexpr (std::is_constructible_v<typename TRegisteredItem::Mapping::Type, TArgs&&...>) {
                    // item depends on nothing from DI container (it is a leaf of a dependencies tree) so it can be created right away
                    return instantiate<TInterface>(item, *this, std::forward<TArgs>(args)...);
                } else {
                    static_assert(AlwaysFalsePrint<typename TRegisteredItem::Mapping::Type>,
                        "Cannot create an instance of a type you've registered within DI container. "
                        "Either you've called 'Container::createAll' but one of the dependencies requires some extra "
                        "ctor arguments or you've called 'Container::create' and provided wrong ctor arguments");
                }
            }
            // no directly provided ctor arguments - use binded arguments instead
            else {
                return std::apply(
                    [this, &item]<typename... UArgs>(UArgs&&... args) -> TInterface& {
                        if constexpr (std::is_constructible_v<typename TRegisteredItem::Mapping::Type, Container&, UArgs&&...>) {
                            // hook into dependencies creation logic and ensure dependencies of those dependencies are created first
                            DependenciesCtorHook<DependenciesChainNext> dependenciesCtorHook{ *this };
                            return instantiate<TInterface>(item, dependenciesCtorHook, std::forward<UArgs>(args)...);
                        } else if constexpr (std::is_constructible_v<typename TRegisteredItem::Mapping::Type, UArgs&&...>) {
                            // item depends on nothing from DI container (it is a leaf of a dependencies tree) so it can be created right away
                            return instantiate<TInterface>(item, *this, std::forward<UArgs>(args)...);
                        } else {
                            static_assert(AlwaysFalsePrint<typename TRegisteredItem::Mapping::Type>,
                                "Cannot create an instance of a type you've registered within DI container. "
                                "You've provided wrong ctor arguments during DI container bindings setup.");
                        }
                    },
                    // it is safe to move these args coz RegisteredItem may be instantiated only once
                    std::move(item.ctorArgs));
            }
        } else {
            // External items are always initialized
            return *item.template get<TInterface>();
        }
    }

    // TDependenciesHolder may be either 'Container<...>' itself or 'Container<...>::DependenciesCtorHook'
    template <typename TInterface, typename TRegisteredItem, typename TDependenciesHolder, typename... TArgs>
    TInterface& instantiate(TRegisteredItem& item, TDependenciesHolder& dependenciesHolder, TArgs&&... args) {
        TInterface& interface = item.template instantiate<TInterface>(dependenciesHolder, std::forward<TArgs>(args)...);
        // order matters coz 'item.instantiate<TInterface>' may recursisevly instantiate its own dependencies (see 'instantiateAll' mechanism)
        deleters.push_back(makeDeleter<TInterface>());

        return interface;
    }

    template <typename TInterface>
    auto& findItem() {
        using RegisteredItemsTypeList = TypeList<RegisteredItem<TTypeMappings>...>;

        constexpr std::ptrdiff_t itemIndex = RegisteredItemsTypeList::find([]<typename TRegisteredItem>() {
            return TRegisteredItem::Mapping::Interfaces::template contains<TInterface>();
        });

        static_assert(ConditionalPrint<itemIndex != -1, TInterface>,
            "You're trying to create or resolve an interface which isn't registered within DI container");

        return std::get<static_cast<std::size_t>(itemIndex)>(items);
    }

    template <typename TInterface>
    DestroyItemFn makeDeleter() {
        return +[](Container& self) {
            auto& item = self.findItem<TInterface>();
            item.destroy();
        };
    }

private:
    std::tuple<RegisteredItem<TTypeMappings>...> items;
    std::vector<DestroyItemFn> deleters;
};

template <typename... TTypeMappings>
auto makeContainer(RegisteredItem<TTypeMappings>... items) {
    return std::make_shared<liant::Container<TTypeMappings...>>(items...);
}
} // namespace liant