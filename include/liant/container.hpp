#pragma once
#include "liant/missing_dependency_policy.hpp"
#include "liant/tuple.hpp"
#include "liant/typelist.hpp"

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

namespace liant {

enum class ItemLifetime { External, DI };

template <ItemLifetime LifetimeV, typename T, typename... TInterfaces>
struct TypeMapping {
    static_assert((std::is_base_of_v<TInterfaces, T> && ...), "type T must be derived from each of the TInterfaces");

    using Type = T;
    using Interfaces = TypeList<TInterfaces...>;
    static constexpr ItemLifetime Lifetime = LifetimeV;
};


template <typename TTypeMapping>
class RegisteredItem {
public:
    using Mapping = TTypeMapping;

    RegisteredItem() = default;
    RegisteredItem(typename TTypeMapping::Type* item)
        : item(item) {}

    template <typename... TInterfaces>
    auto as() const {
        return RegisteredItem<TypeMapping<TTypeMapping::Lifetime, typename TTypeMapping::Type, TInterfaces...>>{ item };
    }

private:
    template <typename TMissingDependencyPolicy, typename... TTypeMappings>
    friend class Container;

    template <typename TInterface>
    auto get() {
        return static_cast<TInterface*>(item);
    }

    // TDependenciesHolder may be either 'Container<...>' itself or 'Container<...>::DependenciesCtorHook'
    template <typename TInterface, typename TDependenciesHolder, typename... TArgs>
    auto& instantiate(TDependenciesHolder& dependenciesHolder, TArgs&&... args) {
        if constexpr (std::is_constructible_v<typename TTypeMapping::Type, TDependenciesHolder&, TArgs&&...>) {
            item = new typename TTypeMapping::Type(dependenciesHolder, std::forward<TArgs>(args)...);
        } else {
            item = new typename TTypeMapping::Type(std::forward<TArgs>(args)...);
        }
        return static_cast<TInterface&>(*item);
    }

    void destroy() {
        delete item;
    }

private:
    typename TTypeMapping::Type* item{};
};

template <typename T>
auto registerType() {
    return RegisteredItem<TypeMapping<ItemLifetime::DI, T, T>>{};
}

template <typename T>
auto registerItem(T& item) {
    return RegisteredItem<TypeMapping<ItemLifetime::External, T, T>>{ std::addressof(item) };
}

template <typename... TTypes>
class Dependencies;

class ContainerBase {
public:
    virtual ~ContainerBase() = default;
};

template <typename TMissingDependencyPolicy, typename... TTypeMappings>
class Container : public ContainerBase {
    using DestroyItemFn = void (*)(Container&);

public:
    Container(TMissingDependencyPolicy missingDependencyPolicy, RegisteredItem<TTypeMappings>... items)
        : items{ items... }
        , missingDependencyPolicy(missingDependencyPolicy) {
        deleters.reserve(sizeof...(TTypeMappings));
    }
    ~Container() {
        // destroy items in the order opposite to the creation order
        for (auto it = deleters.rbegin(); it != deleters.rend(); ++it) {
            (*it)(*this);
        }
    }

    // trying to find already created 'TInterface' instance
    template <typename TInterface>
    TInterface* find() {
        auto& item = findItemOf<TInterface>();

        return item.template get<TInterface>();
    }

    // trying to find already created 'TInterface' instance
    // if haven't found then MissingDependencyPolicy kicks in (terminate or throw or custom handler)
    template <typename TInterface>
    TInterface& findChecked() {
        auto& item = findItemOf<TInterface>();

        if (TInterface* interface = item.template get<TInterface>()) {
            return *interface;
        } else {
            return missingDependencyPolicy.template handleMissingItem<TInterface>(*this);
        }
    }

    // create an instance of type registered as 'TInterface'
    // if such instance already exists then just return it statically casted to 'TInterface'
    template <typename TInterface, typename... TArgs>
    TInterface& create(TArgs&&... args) {
        auto& item = findItemOf<TInterface>();
        return instantiate(TypeIdentity<TInterface>{}, item, std::forward<TArgs>(args)...);
    }

    // create all registered instances automatically
    // order is determined automatically, cycles are detected automatically
    // note: your types should satisfy some preconditions:
    // - registered type should be default-constructible, or
    // - registered type should be constructible from 'Dependencies<...>'
    void createAll() {
        liant::tuple::forEach(items, [&]<typename TTypeMapping>(RegisteredItem<TTypeMapping>&) {
            instantiateAll(typename TTypeMapping::Interfaces{}, TypeList<>{});
        });
    }

private:
    template <typename TInterface, typename TRegisteredItem, typename... TArgs>
    TInterface& instantiate(TypeIdentity<TInterface>, TRegisteredItem& item, TArgs&&... args) {
        if (TInterface* existingInterface = item.template get<TInterface>()) {
            return *existingInterface;
        } else {
            TInterface& interface = item.template instantiate<TInterface>(*this, std::forward<TArgs>(args)...);
            // order matters coz 'item.instantiate<TInterface>' may recursisevly instantiate its own dependencies (see 'instantiateAll' mechanism)
            deleters.push_back(makeDeleter<TInterface>());

            return interface;
        }
    }

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
    void instantiateAll(TypeList<TInterfaces...>, TDependenciesChain) {
        TypeList<TInterfaces...>::template forEach([&]<typename TInterface>() {
            static_assert(!TDependenciesChain::template contains<TInterface>(), "detected cycle in you dependencies");
            using DependenciesChainNext = TypeListAppendT<TDependenciesChain, TInterface>;

            auto& item = findItemOf<TInterface>();
            using Item = std::remove_reference_t<decltype(item)>;

            if constexpr (Item::Mapping::Lifetime == ItemLifetime::DI) {
                if constexpr (std::is_constructible_v<typename Item::Mapping::Type, Container&>) {
                    // hook into dependencies creation logic and ensure dependencies of those dependencies are created first
                    DependenciesCtorHook<DependenciesChainNext> dependenciesCtorHook{ *this };
                    item.template instantiate<TInterface>(dependenciesCtorHook);
                } else if constexpr (std::is_default_constructible_v<typename Item::Mapping::Type>) {
                    // item depends on nothing (it is a leaf of a dependencies tree) so it can be created right away
                    item.template instantiate<TInterface>(*this);
                } else {
                    static_assert(AlwaysFalsePrint<typename Item::Mapping::Type>,
                        "type you've registered within DI container doesn't have default ctor or ctor which accepts "
                        "only liant::Dependencies<...>");
                }
            }
        });
    }

    template <typename TInterface>
    auto& findItemOf() {
        using RegisteredItemsTypeList = TypeList<RegisteredItem<TTypeMappings>...>;

        constexpr std::ptrdiff_t itemIndex = RegisteredItemsTypeList::find([]<typename TRegisteredItem>() {
            return TRegisteredItem::Mapping::Interfaces::template contains<TInterface>();
        });

        static_assert(ConditionalPrint<itemIndex != -1, TInterface>,
            "you're trying to create or resolve an interface which isn't registered within DI container");

        return std::get<static_cast<std::size_t>(itemIndex)>(items);
    }

    template <typename TInterface>
    DestroyItemFn makeDeleter() {
        return +[](Container& self) {
            auto& item = self.findItemOf<TInterface>();
            item.destroy();
        };
    }

private:
    std::tuple<RegisteredItem<TTypeMappings>...> items;
    TMissingDependencyPolicy missingDependencyPolicy;
    std::vector<DestroyItemFn> deleters;
};

template <typename TMissingDependencyPolicy, typename... TTypeMappings>
auto makeContainer(TMissingDependencyPolicy missingDependencyPolicy, RegisteredItem<TTypeMappings>... items) {
    static_assert(liant::missing_dependency_policy::is_policy_v<TMissingDependencyPolicy>,
        "first argument to liant::makeContainer(...) should be Missing Dependency Policy\nexpecting something like:\n"
        "liant::makeContainer(liant::missing_dependency_policy::Terminate, ...)\n"
        "                     ^ here we are using 'terminate' policy\n");
    return std::make_shared<liant::Container<TMissingDependencyPolicy, TTypeMappings...>>(missingDependencyPolicy, items...);
}
} // namespace liant