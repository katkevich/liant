# Liant C++ DI Library

[GitHub repo](https://github.com/Katkevich/liant)

The `liant` C++ Dependency Injection (DI) library provides a robust and flexible way to manage object creation and dependencies in your application. It promotes loose coupling and improves testability by allowing you to define how your types are created and where their dependencies come from.

## Creating a Container
The `liant::makeContainer` function is the primary entry point. It allows you to define the initial set of services and their mappings.

### 1. Registering a Type as Itself
The container will manage the lifetime of an instance of `MyType`, and it will be resolved when exactly `MyType` is requested.
```c++
auto container = liant::makeContainer(
    liant::registerInstanceOf<MyType>()
);
```

### 2. Registering a Type as an Interface
The container will instantiate and manage the lifetime of an instance of `MyService`. `MyService` will be instantiated when `IMyService` is requested. 
```c++
struct MyService : public IMyService {
    MyService() { /* ... */ }
};

auto container = liant::makeContainer(
    liant::registerInstanceOf<MyService>().as<IMyService>()
);
```

### 3. Registering a Type as multiple Interfaces
The container will instantiate and manage the lifetime of an instance of `MyService`. `MyService` will be instantiated when `IMyService` or `IMyServiceExtra` is requested.
```c++
struct MyService : public IMyService, public IMyServiceExtra {
    MyService() { /* ... */ }
};

auto container = liant::makeContainer(
    liant::registerInstanceOf<MyService>().as<IMyService1, IMyService2>()
);
```

### 4. Binding Constructor Arguments
You can bind specific arguments that will be passed to the constructor of your type when the container instantiates it.
```c++
struct MyService {
    MyService(std::string s, int i) { /* ... */ }
};

auto container = liant::makeContainer(
    liant::registerInstanceOf<MyService>().bindArgs("hello", 123)
);
```

### 5. Combining Interface Mapping and Argument Binding
You can chain `as()` and `bindArgs()` to fully configure your type registration.
```c++
struct MyServiceImpl : public IMyService {
    MyServiceImpl(std::string config) { /* ... */ }
};

auto container = liant::makeContainer(
    liant::registerInstanceOf<MyServiceImpl>().as<IMyService>().bindArgs("config_value")
);
```

### 6. Registering an Existing Instance
Use this when you have an object whose lifetime is managed externally, and you just want the container to provide access to it.
```c++
MyGlobalObject* existing_object = ...;

auto container = liant::makeContainer(
    liant::registerInstance(existing_object).as<IService>()
);
```

## Hierarchical Containers
`liant` supports nesting containers to create logical scopes and share common services. When a dependency is requested, the child container is checked first, then its request is delegated to the base container. This promotes modularity and allows for overriding dependencies. 

1. ### Using `std::shared_ptr<liant::Container>` as a Base
The child container will inherit all dependencies registered in the `base_container`.
```c++
// 'base_container' here is std::shared_ptr<liant::Container<...>>
auto base_container = liant::makeContainer(
    liant::registerInstanceOf<BaseService>()
);
auto child_container = liant::makeContainer(
    liant::baseContainer(base_container),
    liant::registerInstanceOf<ChildService>() // ChildService is specific to this `child_container`
);
```
2. ### Using `liant::ContainerSlice` or `liant::ContainerView` (or their lazy variants) as a Base
This allows the child container to inherit a *filtered view* of dependencies from the base container, exposing only a specific subset of its capabilities. This could be userful when your dependency itself introduces another container with *narrower scope*.
```c++
// Assume 'base_container' is an already created std::shared_ptr<liant::Container<...>>

// Create a slice that only exposes ILogger and IConfig from the base container
liant::ContainerSlice<ILogger, IConfig> base_slice(base_container);

auto specialized_child_container = liant::makeContainer(
    base_slice, // Only ILogger and IConfig are visible from the base here
    liant::registerInstanceOf<SpecializedService>()
);
```

## Consuming Dependencies
Components declare their dependencies by taking a `liant::ContainerSlice` or `liant::ContainerView` (or their lazy and weak variants) in their constructor. This allows them to pull other dependencies from the container as needed. Each type offers different ownership and resolution strategies.

1. ### `liant::ContainerView<TInterfaces...>`
   * Ownership: non-owning
   * Dependencies Resolution: eager (on construction)
```c++
// construct 'ContainerSlice' from 'Container'
template <typename UBaseContainer, typename... UTypeMappings>
ContainerView(const std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>& container); // (1)

template <typename UBaseContainer, typename... UTypeMappings>
ContainerView(std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>&& container); // (2)

template <typename UBaseContainer, typename... UTypeMappings>
ContainerView(const Container<UBaseContainer, UTypeMappings...>& container); // (3)

// construct 'ContainerView' from other 'ContainerSlice/ContainerView/ContainerSliceLazy/ContainerViewLazy'
// 'UInterfaces...' here should be a superset of `TInterfaces...'
template <typename... UInterfaces>
ContainerView(const ContainerSlice<UInterfaces...>& slice);

template <typename... UInterfaces>
ContainerView(const ContainerView<UInterfaces...>& view);

template <typename... UInterfaces>
ContainerView(const ContainerSliceLazy<UInterfaces...>& slice);

template <typename... UInterfaces>
ContainerView(const ContainerViewLazy<UInterfaces...>& view);

template <typename... UInterfaces>
ContainerView(ContainerSlice<UInterfaces...>&& slice);

template <typename... UInterfaces>
ContainerView(ContainerView<UInterfaces...>&& view);

template <typename... UInterfaces>
ContainerView(ContainerSliceLazy<UInterfaces...>&& slice);

template <typename... UInterfaces>
ContainerView(ContainerViewLazy<UInterfaces...>&& view);
```
Note: use `liant::ContainerView<...>` as a member in you Type for holding your type's dependencies. **DON'T** use `liant::ContainerSlice<...>` member for that purpose otherwise you will get circular references and Container won't get destroyed. 

2. ### `liant::ContainerSlice<TInterfaces...>`
   * Ownership: shared
   * Dependencies Resolution: eager (on construction)
```c++
// construct 'ContainerSlice' from 'Container'
template <typename UBaseContainer, typename... UTypeMappings>
ContainerSlice(const std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>& container); // (1)

template <typename UBaseContainer, typename... UTypeMappings>
ContainerSlice(std::shared_ptr<Container<UBaseContainer, UTypeMappings...>>&& container); // (2)

template <typename UBaseContainer, typename... UTypeMappings>
ContainerSlice(const Container<UBaseContainer, UTypeMappings...>& container); // (3)

// construct 'ContainerSlice' from other 'ContainerSlice/ContainerView/ContainerSliceLazy/ContainerViewLazy'
// 'UInterfaces...' here should be a superset of `TInterfaces...'
template <typename... UInterfaces>
ContainerSlice(const ContainerSlice<UInterfaces...>& slice);

template <typename... UInterfaces>
ContainerSlice(const ContainerView<UInterfaces...>& view);

template <typename... UInterfaces>
ContainerSlice(const ContainerSliceLazy<UInterfaces...>& slice);

template <typename... UInterfaces>
ContainerSlice(const ContainerViewLazy<UInterfaces...>& view);

template <typename... UInterfaces>
ContainerSlice(ContainerSlice<UInterfaces...>&& slice);

template <typename... UInterfaces>
ContainerSlice(ContainerView<UInterfaces...>&& view);

template <typename... UInterfaces>
ContainerSlice(ContainerSliceLazy<UInterfaces...>&& slice);

template <typename... UInterfaces>
ContainerSlice(ContainerViewLazy<UInterfaces...>&& view);
```
Note: use `liant::ContainerSlice<...>` if you need to share the ownership of the Container with somebody else. **DON'T** use `liant::ContainerSlice<...>` as a member in your type for holding your type's dependencies otherwise you will get circular references and Container won't get destroyed.

3. ### `liant::ContainerViewLazy<TInterfaces...>`
   * Ownership: non-owning
   * Resolution: lazy (on access i.e. on `slice->resolve<T>()/slice->resolveAll()`)

Constructors are basically the same as in `liant::ContainerView<...>`. The difference is that `liant::ContainerViewLazy<...>` DO NOT instantiate all its dependencies in its constructor whereas `liant::ContainerView<...>` does. 

You may need lazy variants if, for instance, an instantiation of a particular dependency is heavy but optional and can be avoided. Or, lets say, you have two distinct "branches" of dependencies within single Container and you want to instantiate only one of these branches conditionally.

4. ### `liant::ContainerSliceLazy<TInterfaces...>`
   * Ownership: shared
   * Resolution: lazy (on access i.e. on `slice->resolve<T>()/slice->resolveAll()`)

Constructors are basically the same as in `liant::ContainerSlice<...>`. The difference is that `liant::ContainerSliceLazy<...>` DO NOT instantiate all its dependencies in its constructor whereas `liant::ContainerSlice<...>` does. 

5. ### `liant::ContainerSliceWeak` and `liant::ContainerSliceWeakLazy`
   * Ownership: weak

Provide Weak Ownership to break circular dependencies. Use their `.lock()` method to temporarily gain shared ownership and access dependencies.

## Container/View/Slice API

All `liant::Container`, `liant::ContainerSlice`, `liant::ContainerView` (and their lazy variants) provide a same API for resolving and finding dependencies.
```c++
template <typename TInterface>
TInterface* findRaw() const;

template <typename TInterface>
liant::SharedPtr<TInterface> find() const;

template <typename TInterface, typename... TArgs>
TInterface& resolveRaw(TArgs&&... args);

template <typename TInterface, typename... TArgs>
liant::SharedRef<TInterface> resolve(TArgs&&... args);

virtual void resolveAll();
```

1. `resolveRaw<T>()`

    Resolves an instance of `T`. If it doesn't exist, it's created. Returns a raw reference.


2. `resolve<T>()`

    Resolves an instance of `T`. If it doesn't exist, it's created. Returns a smart reference (`liant::SharedRef`) that extends the container's lifetime.


3. `findRaw<T>()`

    Finds an *already created* instance of `T`. Returns `nullptr` if not found. Returns a raw pointer.


4. `find<T>()`

    Finds an *already created* instance of `T`. Returns an empty smart pointer if not found. Returns a smart pointer (`liant::SharedPtr`) that extends the container's lifetime.


5. `resolveAll()`

    Eagerly instantiates all registered dependencies in the container, slices, views, and their bases. Ideal for application startup.

## `LIANT_DEPENDENCY` Macro
`#include "liant/dependency_macro.hpp`

This utility macro provides "pretty" getter methods for your dependencies, making your code more readable.

Before:
```c++
struct ILogger {
    void log(std::string){ /* ... */ }
};
// Accessing dependencies via resolve
class MyService {
public:
    MyService(liant::ContainerSlice<ILogger> di) {
        di.resolveRaw<ILogger>().log("init");
    }
};
```

After:
```c++
struct ILogger {
    void log(std::string){ /* ... */ }
};
// Define a pretty getter
LIANT_DEPENDENCY(ILogger, logger)

class MyService {
public:
    MyService(liant::ContainerSlice<ILogger> di) {
        di.logger().log("init");
    }
};
```

## Smart Pointers
`liant` provides custom smart pointers (`liant::SharedRef`, `liant::SharedPtr`, `liant::WeakPtr`) designed to interact with the DI container's lifetime management. These pointers hold a pointer to the managed dependency *and* a smart pointer to the `liant::ContainerBase` that owns the dependency. This ensures that the container, and thus the dependency, remains alive as long as any of these smart pointers refer to it.

1. `liant::SharedRef`

   A non-nullable shared "fat" reference. It holds a raw pointer to a dependency and a `std::shared_ptr` to the owning container. It guarantees that the referenced dependency is valid and that its owning container will not be destroyed while `liant::SharedRef` instances exist.


2. `liant::SharedPtr`

   A nullable shared "fat" pointer. Similar to `liant::SharedRef`, it maintains a `std::shared_ptr` to the owning container, preventing its destruction. However, unlike `SharedRef`, it can be null if the underlying dependency is not present or has been reset.


3. `liant::WeakPtr`

   A weak "fat" pointer that does not contribute to the reference count of the owning container. It can be used to break circular dependencies. You must call its `lock()` method to get a `liant::SharedPtr` (and thus shared ownership) before accessing the underlying dependency. If the container or dependency has been destroyed, `lock()` will return an empty `liant::SharedPtr`. 

