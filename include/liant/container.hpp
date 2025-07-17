#pragma once
#include "liant/export_macro.hpp"
#include "liant/ptr.hpp"
#include "liant/tuple.hpp"
#include "liant/typelist.hpp"

#ifndef LIANT_MODULE
#include <concepts>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>
#endif

// clang-format off
LIANT_EXPORT
// clang-format on
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

    // snake_case aliases
    using type = Type;
    using interfaces_type = Interfaces;
    using ctor_args_type = CtorArgs;
    using ctor_args_tuple_type = CtorArgsTuple;
    static constexpr ItemLifetime lifetime_value = Lifetime;
};


template <typename TTypeMapping>
class RegisteredItem {
public:
    using Mapping = TTypeMapping;
    using mapping_type = Mapping;

    RegisteredItem()
        requires std::default_initializable<typename TTypeMapping::CtorArgsTuple>
    = default;

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
    template <typename TBaseContainer, typename... TTypeMappings>
    friend class Container;

    template <typename TInterface>
    auto get() const {
        return static_cast<TInterface*>(item);
    }

    template <typename TInterface, typename TContainerSliceCtorHook, typename... TArgs>
    auto& instantiate(TContainerSliceCtorHook& hook, TArgs&&... args) {
        item = new TTypeMapping::Type{ hook, std::forward<TArgs>(args)... };

        if constexpr (requires { item->postCreate(); }) {
            item->postCreate();
        }

        return static_cast<TInterface&>(*item);
    }

    template <typename TInterface, typename... TArgs>
    auto& instantiateTrivial(TArgs&&... args) {
        item = new TTypeMapping::Type{ std::forward<TArgs>(args)... };

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
    TTypeMapping::CtorArgsTuple ctorArgs;
};

template <typename T>
auto registerInstanceOf() {
    return RegisteredItem<TypeMapping<ItemLifetime::DI, T, TypeList<T>, TypeList<>>>{};
}

template <typename T>
auto registerInstance(T& item) {
    return RegisteredItem<TypeMapping<ItemLifetime::External, T, TypeList<T>, TypeList<>>>{ std::addressof(item) };
}


template <typename... TInterfaces>
class ContainerSlice;

template <typename... TInterfaces>
class ContainerSliceWeak;

template <typename... TInterfaces>
class ContainerView;


using EmptyDependenciesChain = TypeList<>;
class EmptyContainer;

class ContainerBase : public std::enable_shared_from_this<ContainerBase> {
public:
    virtual ~ContainerBase() = default;
    virtual void resolveAll() = 0;
};

template <typename TBaseContainer, typename... TTypeMappings>
class Container : public ContainerBase {
    using DestroyItemFn = void (*)(Container&);

    template <typename UBaseContainer, typename... UTypeMappings>
    friend class Container;

    using AllInterfaces = TypeListMergeT<typename TTypeMappings::Interfaces...>;

    static constexpr auto DuplicateIndex = AllInterfaces::findDuplicate();
    static_assert(DuplicateIndex == -1 || liant::Print<decltype(AllInterfaces::template at<DuplicateIndex>())>,
        "Cannot register same interface multiple times "
        "(search 'liant::Print' in the compilation output for details).");

public:
    using RegisteredItems = TypeList<RegisteredItem<TTypeMappings>...>;

    Container(TBaseContainer baseContainer, RegisteredItem<TTypeMappings>... items)
        : items{ items... }
        , baseContainer(std::move(baseContainer)) {
        deleters.reserve(sizeof...(TTypeMappings));
    }
    ~Container() {
        // destroy items in the order opposite to the creation order
        for (auto i = deleters.size(); i > 0; --i) {
            deleters[i - 1](*this);
        }
    }

    // trying to find already created instance registered 'as TInterface'
    // the unsafe raw pointer is being returned here so make sure it doesn't outlive the 'Container' itself
    template <typename TInterface>
    TInterface* findRaw() const {
        return findInternal<TInterface>();
    }

    // trying to find already created instance registered 'as TInterface'
    // returned fat 'SharedRef' protects 'Container' from being destroyed so use 'SharedRef' with caution (you don't really want block 'Container' deletion)
    template <typename TInterface>
    SharedPtr<TInterface> find() const {
        return SharedPtr<TInterface>(findInternal<TInterface>(), Container::shared_from_this());
    }

    // resolve an instance of type registered 'as TInterface'
    // the unsafe raw reference is being returned here so make sure it doesn't outlive the 'Container' itself
    //
    // all dependencies will be created automatically, cycles are detected automatically
    // if such instance already exists then just return it, statically casted to 'TInterface'
    // if current container and base container both have same 'TInterface' registered then 'TInterface' from current container will be used
    // if current container doesn't have 'TInterface' registered then we will try to resolve it from the base container
    template <typename TInterface, typename... TArgs>
    TInterface& resolveRaw(TArgs&&... args) {
        return resolveInternal<TInterface, EmptyDependenciesChain>(std::forward<TArgs>(args)...);
    }

    // resolve an instance of type registered 'as TInterface'
    // returned fat 'SharedRef' protects 'Container' from being destroyed so use 'SharedRef' with caution (you don't really want block 'Container' deletion)
    //
    // all dependencies will be created automatically, cycles are detected automatically
    // if such instance already exists then just return it, statically casted to 'TInterface'
    // if current container and base container both have same 'TInterface' registered then 'TInterface' from current container will be used
    // if current container doesn't have 'TInterface' registered then we will try to resolve it from the base container
    template <typename TInterface, typename... TArgs>
    SharedRef<TInterface> resolve(TArgs&&... args) {
        return SharedRef<TInterface>(resolveInternal<TInterface, EmptyDependenciesChain>(std::forward<TArgs>(args)...),
            Container::shared_from_this());
    }

    // resolve all registered instances automatically
    // order is determined automatically, cycles are detected automatically
    // base container is being resolved as well
    // note: your types should satisfy some preconditions:
    // - registered type should be default-constructible
    // - or registered type should be only constructible from 'ContainerSlice<...>' / 'ContainerView<...>'
    // - or you should provide ctor arguments while calling 'Container::resolve<...>'
    // - or you should provide ctor arguments bindings while configuring DI mappings (bindArgs(...))
    virtual void resolveAll() final {
        baseContainer->resolveAll();

        liant::tuple::forEach(items, [&]<typename TTypeMapping>(RegisteredItem<TTypeMapping>&) {
            instantiateAll<EmptyDependenciesChain>(typename TTypeMapping::Interfaces{});
        });
    }

private:
    // while creating non-trivial dependency you need to create its dependencies first
    // and those dependencies should create theirs dependencies first and so on
    // 'ContainerSliceCtorHook' allow us to build this recursive initialization chain and create the 'leafs' of this
    // dependencies tree first and from there go up all the way to the roots
    template <typename TDependenciesChain>
    struct ContainerSliceCtorHook {
        template <typename... TInterfaces>
        operator ContainerSlice<TInterfaces...>() {
            return ContainerSlice<TInterfaces...>{ std::static_pointer_cast<Container>(container.shared_from_this()) };
        }

        template <typename... TInterfaces>
        operator ContainerSliceWeak<TInterfaces...>() {
            return ContainerSliceWeak<TInterfaces...>{ std::static_pointer_cast<Container>(container.shared_from_this()) };
        }

        template <typename... TInterfaces>
        operator ContainerView<TInterfaces...>() {
            return ContainerView<TInterfaces...>{ std::static_pointer_cast<Container>(container.shared_from_this()) };
        }

        Container& container;
    };

    template <typename TInterface, typename TDependenciesChain, typename... TArgs>
    TInterface& resolveInternal(TArgs&&... args) {
        if constexpr (constexpr std::ptrdiff_t itemIndex = findItemIndex<TInterface>(); itemIndex != -1) {
            // if TInterface exists in current container then try to use it
            return instantiateOne<TInterface, TDependenciesChain>(getItem<itemIndex>(), std::forward<TArgs>(args)...);
        } else {
            // otherwise delegate to the base container which may possibly have TInterface
            return baseContainer->template resolveRaw<TInterface>(std::forward<TArgs>(args)...);
        }
    }

    // TDependenciesChain is there to detect dependencies cycles at compile time
    template <typename TDependenciesChain, typename... TInterfaces>
    void instantiateAll(TypeList<TInterfaces...>) {
        TypeList<TInterfaces...>::forEach([&]<typename TInterface>() { //
            resolveInternal<TInterface, TDependenciesChain>();
        });
    }

    template <typename TInterface, typename TDependenciesChain, typename TRegisteredItem, typename... TArgs>
    TInterface& instantiateOne(TRegisteredItem& item, TArgs&&... args) {
        // ensure items are only instantiated once
        if (TInterface* existingInterface = item.template get<TInterface>()) {
            return *existingInterface;
        }

        static_assert(!TDependenciesChain::template contains<TInterface>(), "Detected cycle in your dependencies");
        using DependenciesChainNext = TypeListAppendT<TDependenciesChain, TInterface>;

        if constexpr (TRegisteredItem::Mapping::Lifetime == ItemLifetime::DI) {
            // use directly provided ctor arguments instead of the binded ones
            if constexpr (sizeof...(TArgs) > 0) {
                if constexpr (std::is_constructible_v<typename TRegisteredItem::Mapping::Type, std::shared_ptr<Container>, TArgs&&...>) {
                    // hook into dependencies (basically liant::ContainerSlice) creation logic and ensure dependencies of those dependencies are created first
                    ContainerSliceCtorHook<DependenciesChainNext> hook{ *this };
                    return instantiate<TInterface>(item, hook, std::forward<TArgs>(args)...);
                } else if constexpr (std::is_constructible_v<typename TRegisteredItem::Mapping::Type, TArgs&&...>) {
                    // item depends on nothing from DI container (it is a leaf of a dependencies tree) so it can be created right away
                    return instantiateTrivial<TInterface>(item, std::forward<TArgs>(args)...);
                } else {
                    static_assert(liant::Print<typename TRegisteredItem::Mapping::Type>,
                        "Cannot create an instance of a type you've registered within DI container. "
                        "Either you've called 'Container::createAll' but one of the dependencies requires some extra "
                        "ctor arguments or you've called 'Container::create' and provided wrong ctor arguments "
                        "(search 'liant::Print' in the compilation output for details)");
                }
            }
            // no directly provided ctor arguments - use binded arguments instead
            else {
                return std::apply(
                    [this, &item]<typename... UArgs>(UArgs&&... args) -> TInterface& {
                        if constexpr (std::is_constructible_v<typename TRegisteredItem::Mapping::Type, std::shared_ptr<Container>, UArgs&&...>) {
                            // hook into dependencies (basically liant::ContainerSlice) creation logic and ensure dependencies of those dependencies are created first
                            ContainerSliceCtorHook<DependenciesChainNext> hook{ *this };
                            return instantiate<TInterface>(item, hook, std::forward<UArgs>(args)...);
                        } else if constexpr (std::is_constructible_v<typename TRegisteredItem::Mapping::Type, UArgs&&...>) {
                            // item depends on nothing from DI container (it is a leaf of a dependencies tree) so it can be created right away
                            return instantiateTrivial<TInterface>(item, std::forward<UArgs>(args)...);
                        } else {
                            static_assert(liant::Print<typename TRegisteredItem::Mapping::Type, UArgs...>,
                                "Cannot create an instance of a type you've registered within DI container. "
                                "You've provided wrong ctor arguments during DI container bindings setup "
                                "(search 'liant::Print' in the compilation output for details).");
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

    // TContainerSliceCtorHook may be either 'Container<...>' itself or 'Container<...>::ContainerSliceCtorHook'
    template <typename TInterface, typename TRegisteredItem, typename TContainerSliceCtorHook, typename... TArgs>
    TInterface& instantiate(TRegisteredItem& item, TContainerSliceCtorHook& hook, TArgs&&... args) {
        TInterface& interface = item.template instantiate<TInterface>(hook, std::forward<TArgs>(args)...);
        // order matters coz 'item.instantiate<TInterface>' may recursisevly instantiate its own dependencies (see 'instantiateAll' mechanism)
        deleters.push_back(makeDeleter<TInterface>());

        return interface;
    }

    template <typename TInterface, typename TRegisteredItem, typename... TArgs>
    TInterface& instantiateTrivial(TRegisteredItem& item, TArgs&&... args) {
        TInterface& interface = item.template instantiateTrivial<TInterface>(std::forward<TArgs>(args)...);
        // order matters coz 'item.instantiate<TInterface>' may recursisevly instantiate its own dependencies (see 'instantiateAll' mechanism)
        deleters.push_back(makeDeleter<TInterface>());

        return interface;
    }

    template <typename TInterface>
    static constexpr std::ptrdiff_t findItemIndex() {
        constexpr std::ptrdiff_t itemIndex = RegisteredItems::find([]<typename TRegisteredItem>() {
            return TRegisteredItem::Mapping::Interfaces::template contains<TInterface>();
        });
        return itemIndex;
    }

    template <std::ptrdiff_t ItemIndex>
    auto& getItem() {
        return std::get<static_cast<std::size_t>(ItemIndex)>(items);
    }

    template <std::ptrdiff_t ItemIndex>
    const auto& getItem() const {
        return std::get<static_cast<std::size_t>(ItemIndex)>(items);
    }

    template <typename TInterface>
    auto* findInternal() const {
        if constexpr (constexpr std::ptrdiff_t itemIndex = findItemIndex<TInterface>(); itemIndex != -1) {
            return getItem<itemIndex>().template get<TInterface>();
        } else {
            return baseContainer->template findRaw<TInterface>();
        }
    }

    template <typename TInterface>
    DestroyItemFn makeDeleter() {
        return +[](Container& self) {
            auto& item = self.getItem<findItemIndex<TInterface>()>();
            item.destroy();
        };
    }

private:
    std::tuple<RegisteredItem<TTypeMappings>...> items;
    std::vector<DestroyItemFn> deleters;
    TBaseContainer baseContainer;
};

class EmptyContainer {
public:
    using RegisteredItems = TypeList<>;
    using registered_items_type = RegisteredItems;

    const EmptyContainer* operator->() const {
        return this;
    }

    EmptyContainer* operator->() {
        return this;
    }

    void resolveAll() {
        // do nothing
    }

    template <typename TInterface, typename TDependenciesChain, typename... TArgs>
    TInterface& resolveInternal(TArgs&&...) {
        static_assert(liant::Print<TInterface>,
            "You're trying to create or resolve an interface which isn't registered within DI container "
            "(search 'liant::Print' in the compilation output for details)");
    }

    template <typename TInterface>
    constexpr std::ptrdiff_t findItemIndex() {
        return -1;
    }

    template <typename TInterface>
    TInterface* findRaw() const {
        static_assert(liant::Print<TInterface>,
            "You're trying to find an interface which isn't registered within DI container "
            "(search 'liant::Print' in the compilation output for details)");
    }
};

template <typename TBaseContainer, typename... TTypeMappings>
auto baseContainer(const std::shared_ptr<Container<TBaseContainer, TTypeMappings...>>& container) {
    return container;
}

template <typename... TInterfaces>
auto baseContainer(const ContainerSlice<TInterfaces...>& containerSlice) {
    return containerSlice;
}

template <typename... TTypeMappings>
auto makeContainer(RegisteredItem<TTypeMappings>... items) {
    return std::make_shared<liant::Container<EmptyContainer, TTypeMappings...>>(EmptyContainer{}, items...);
}

template <typename TBaseContainer, typename... TTypeMappings>
auto makeContainer(const std::shared_ptr<TBaseContainer>& baseContainer, RegisteredItem<TTypeMappings>... items) {
    return std::make_shared<liant::Container<std::shared_ptr<TBaseContainer>, TTypeMappings...>>(baseContainer, items...);
}

template <typename... TBaseTypes, typename... TTypeMappings>
auto makeContainer(const ContainerSlice<TBaseTypes...>& baseContainer, RegisteredItem<TTypeMappings>... items) {
    return std::make_shared<liant::Container<ContainerSlice<TBaseTypes...>, TTypeMappings...>>(baseContainer, items...);
}
} // namespace liant