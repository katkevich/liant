#pragma once
#include "liant/missing_dependency_policy.hpp"
#include "liant/typelist.hpp"

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

namespace liant {

template <typename T, typename... TInterfaces>
struct TypeMapping {
    static_assert((std::is_base_of_v<TInterfaces, T> && ...), "type T must be derived from each of the TInterfaces");

    using Type = T;
    using Interfaces = TypeList<TInterfaces...>;
};


template <typename TTypeMapping>
class RegisteredInstance {
public:
    using Mapping = TTypeMapping;

    RegisteredInstance() = default;
    RegisteredInstance(typename TTypeMapping::Type* instance)
        : instance(instance) {}

    template <typename... TInterfaces>
    auto as() const {
        return RegisteredInstance<TypeMapping<typename TTypeMapping::Type, TInterfaces...>>{ instance };
    }

private:
    template <typename TMissingDependencyPolicy, typename... TTypeMappings>
    friend class Container;

    template <typename TInterface>
    auto get() {
        return static_cast<TInterface*>(instance);
    }

    template <typename TInterface, typename TDIContainer, typename... TArgs>
    auto& create(TDIContainer& diContainer, TArgs&&... args) {
        if constexpr (std::is_constructible_v<typename TTypeMapping::Type, TDIContainer&, TArgs&&...>) {
            instance = new typename TTypeMapping::Type(diContainer, std::forward<TArgs>(args)...);
        } else {
            instance = new typename TTypeMapping::Type(std::forward<TArgs>(args)...);
        }
        return static_cast<TInterface&>(*instance);
    }

    void destroy() {
        delete instance;
    }

private:
    typename TTypeMapping::Type* instance{};
};

template <typename T>
auto registerType() {
    return RegisteredInstance<TypeMapping<T, T>>{};
}

template <typename T>
auto registerInstance(T& instance) {
    return RegisteredInstance<TypeMapping<T, T>>{ std::addressof(instance) };
}

class ContainerBase {
public:
    virtual ~ContainerBase() = default;
};

template <typename TMissingDependencyPolicy, typename... TTypeMappings>
class Container : public ContainerBase {
    using DestroyInstanceFn = void (*)(Container&);

public:
    Container(TMissingDependencyPolicy missingDependencyPolicy, RegisteredInstance<TTypeMappings>... instances)
        : instances{ instances... }
        , missingDependencyPolicy(missingDependencyPolicy) {
        deleters.reserve(sizeof...(TTypeMappings));
    }
    ~Container() {
        // destroy instances in the order opposite to the creation order
        for (auto it = deleters.rbegin(); it != deleters.rend(); ++it) {
            (*it)(*this);
        }
    }

    template <typename TInterface>
    TInterface* resolve() {
        auto& instance = findInstanceOf<TInterface>();

        return instance.template get<TInterface>();
    }

    template <typename TInterface>
    TInterface& resolveChecked() {
        auto& instance = findInstanceOf<TInterface>();

        if (TInterface* interface = instance.template get<TInterface>()) {
            return *interface;
        } else {
            return missingDependencyPolicy.template handleMissingInstance<TInterface>(*this);
        }
    }

    template <typename TInterface, typename... TArgs>
    TInterface& create(TArgs&&... args) {
        auto& instance = findInstanceOf<TInterface>();

        if (TInterface* existingInterface = instance.template get<TInterface>()) {
            return *existingInterface;
        } else {
            deleters.push_back(makeDeleter<TInterface>());
            return instance.template create<TInterface>(*this, std::forward<TArgs>(args)...);
        }
    }

    void createAll() {}

private:
    template <typename TInterface>
    auto& findInstanceOf() {
        using RegisteredInstancesTypeList = TypeList<RegisteredInstance<TTypeMappings>...>;

        constexpr std::ptrdiff_t instanceIndex = RegisteredInstancesTypeList::find([]<typename TRegisteredInstance>() {
            return TRegisteredInstance::Mapping::Interfaces::template contains<TInterface>();
        });

        static_assert(instanceIndex != -1,
            "you're trying to create or resolve an interface which isn't registered within DI container");

        return std::get<static_cast<std::size_t>(instanceIndex)>(instances);
    }

    template <typename TInterface>
    DestroyInstanceFn makeDeleter() {
        return +[](Container& self) {
            auto& instance = self.findInstanceOf<TInterface>();
            instance.destroy();
        };
    }

private:
    std::tuple<RegisteredInstance<TTypeMappings>...> instances;
    TMissingDependencyPolicy missingDependencyPolicy;
    std::vector<DestroyInstanceFn> deleters;
};

template <typename TMissingDependencyPolicy, typename... TTypeMappings>
auto makeContainer(TMissingDependencyPolicy missingDependencyPolicy, RegisteredInstance<TTypeMappings>... items) {
    static_assert(liant::missing_dependency_policy::is_policy_v<TMissingDependencyPolicy>,
        "first argument to liant::makeContainer(...) should be Missing Dependency Policy\nexpecting something like:\n"
        "liant::makeContainer(liant::missing_dependency_policy::Terminate, ...)\n"
        "                     ^ here we are using 'terminate' policy\n");
    return std::make_shared<liant::Container<TMissingDependencyPolicy, TTypeMappings...>>(missingDependencyPolicy, items...);
}
} // namespace liant