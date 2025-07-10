[![Standard](https://img.shields.io/badge/standard-C%2B%2B20-blue.svg?logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://mit-license.org/)
[![CMake: 3.30+](https://img.shields.io/badge/CMake-3.30%2B-blue)](https://cmake.org/)
# Liant - C++ DI Container Library

`liant` is a C++ Dependency Injection (DI) container library. It simplifies object lifecycle management and dependency resolution, allowing for cleaner, more maintainable code.

Beyond its core DI features, `liant` also serves as a good example of how to build, distribute, and consume a C++ library using modern CMake, including C++20 Modules and C++23 `import std` integration.

## Quick example

```c++
import std;
import liant;

struct Interface1 {};
struct Type1 : Interface1 {};
struct Type2 {
    // injection via ctor ('liant::ContainerView<...>' should be first argument)
    Type2(liant::ContainerView<Interface1> di)
        : di(di) {}
    liant::ContainerView<Interface1> di;
};
struct Type3 {
    // injection via aggregate initialization ('liant::ContainerView<...>' should be first field)
    liant::ContainerView<Interface1, Type2> di;
    std::string arg;

    void postCreate() const { std::println("Type3::postCreate"); }
    void preDestroy() const { std::println("Type3::preDestroy"); }
};

int main()
{
    auto container = liant::makeContainer(
        liant::registerInstanceOf<Type1>().as<Interface1>(),
        liant::registerInstanceOf<Type2>(),
        liant::registerInstanceOf<Type3>().bindArgs("Hello, World!")
    );
    // instantiate all at once automatically
    container->resolveAll();

    liant::SharedPtr<Type3> type3Instance = container->find<Type3>();

    std::println("Type3::arg: {0}", type3Instance->arg);
}
```

## Features

* Injection through constructors or aggregate initialization.
* Detects circular dependencies **at compile time**.
* Dependencies are resolved and injected lazily and automatically.
* Initialization and destruction order management. `postCreate`/`preDestroy` customization points.
* Ability to instantiate all dependencies at once, automatically resolving their order, or to instantiate them one by one.
* You can "include" one container (or its slice) as a base for another, so that dependencies from base container are reused by child container.
* Bind concrete types to interfaces or simply register types as-is.
* **Multiple Distribution Formats:**
    * **Header-only version:** For traditional `#include`-based usage.
    * **C++20 Module version:** `import liant` support (standard library goes into the Global Module Fragment).
    * **C++23 Module version:** `import std` support.
* **Nice Compile-Time Diagnostics:** provides clear error messages to help you quickly resolve compilation errors if you misuse API.

## How to build & install

### Prerequisites

* Git
* CMake 3.30+
* Ninja 1.11+
* C++ toolchain:
    * **C++23 `liant_module` target (with `import std`)** is only working with **Clang 20+** and **libc++-20** for now. You will need to configure your cmake build with libc++ standard library like `-DCMAKE_CXX_FLAGS="-stdlib=libc++"`.
    * **C++20 header-only `liant` target** & **`liant_module_nostd` target** should work with any reasonably conforming C++20 compiler (GCC 13+, Clang 18+, MSVC 17.10+).

### Building and Installing from Source

We will use **llvm-20** toolchain and **CMake 4.0.3** 

1.  Clone the repository:

    ```bash
    git clone https://github.com/Katkevich/liant.git
    cd liant
    ```

2.  Create a build directory and configure CMake:
    ```bash
    mkdir build
    cd build
    # -DCMAKE_CXX_FLAGS="-stdlib=libc++" - this one is neccesary here if you want C++23 'import std' support
    # -DCMAKE_INSTALL_PREFIX=../install  - set the installation directory to 'liant/install' for test purposes (don't pollute /usr/local while testing all of this)
    cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-20 -DCMAKE_CXX_COMPILER=clang++-20 -DCMAKE_CXX_FLAGS="-stdlib=libc++" -DCMAKE_INSTALL_PREFIX=../install
    ```
3.  Build the library:
    ```bash
    cmake --build .
    ```

4.  Install the library:
    ```bash
    cmake --install .
    ```

Installation is done. Now you can find `liant` package using CMake `find_package` command in config mode.

## Using `liant` library

`liant` package provides 3 targets:
-  `liant::liant` - traditional header-only library
-  `liant::liant_module` - C++23 module with `import std` support
-  `liant::liant_module_nostd` - C++20 module with standard library being in Global Module Fragment.

You can choose which one to link against based on your project's needs.


### Creating `liant_example` project

1. Create `liant_example` folder next to the `liant` library folder as follows:

    ```
    ├── liant
    │   ├── build
    │   ├── install
    |   ├── ...
    |   ├── CMakeLists.txt
    |
    ├── liant_example
    |   ├── build
    |   ├── main.cpp
    |   ├── CMakeLists.txt
    ```
1. Create **liant_example/main.cpp** (copy **Quick Example** code) and **liant_example/CMakeLists.txt**:

- Since `import std` is still experimental feature in CMake, we must enable it explicitly
- Since our target is using standard module, we must set `CXX_MODULE_STD` property for our target
- `CXX_EXTENSIONS` should be `ON` because of these CMake issues:
    - https://gitlab.kitware.com/cmake/cmake/-/issues/25916
    - https://gitlab.kitware.com/cmake/cmake/-/issues/25539
    ```cmake
    cmake_minimum_required(VERSION 3.30)

    # CMake support for 'import std' is experimental still
    # To enable it you must set 'CMAKE_EXPERIMENTAL_CXX_IMPORT_STD' UUID
    # Each CMake version may have it's own unique UUID for each experimental feature
    #
    # CMake 4.0.3:  "d0edc3af-4c50-42ea-a356-e2862fe7a444" <<< we are using this one
    # CMake 3.31.8: "d0edc3af-4c50-42ea-a356-e2862fe7a444"
    #
    # to find UUID for your CMake version look your version CMake sources (/Help/dev/experimental.rst)
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

    project(liant_example)

    # Explicitly specify the liant library installation path (path to the `liant-config.cmake` file to be more precise)
    #
    # If you'd installed the library in the default location (hadn't specified `-DCMAKE_INSTALL_PREFIX=../install` during CMake configuration) then you would not need to explicitly specify the path to the library here.
    # `find_package(liant CONFIG REQUIRED)` would just be enough
    find_package(liant CONFIG REQUIRED PATHS ../liant/install/lib/cmake/liant)

    add_executable(liant_example)
    target_compile_features(liant_example PUBLIC cxx_std_23)
    set_target_properties(liant_example PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
        CXX_MODULE_STD ON
    )
    target_sources(liant_example PRIVATE main.cpp)
    # link with C++23 liant_module target
    target_link_libraries(liant_example PRIVATE liant::liant_module)
    ```

### Building & running `liant_example`
1. Build `liant_example`:

    ```bash
    cd ../../
    mkdir -p liant_example/build
    cd liant_example/build

    cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-20 -DCMAKE_CXX_COMPILER=clang++-20 -DCMAKE_CXX_FLAGS="-stdlib=libc++"

    cmake --build .
    ```

1. Run `liant_example`
    ```bash
    ./liant_example

    # prints:
    #
    # Type3::postCreate
    # Type3::arg: Hello, World!
    # Type3::preDestroy
    ```