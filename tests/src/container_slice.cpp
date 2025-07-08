#include "data.hpp"
#include "liant/liant.hpp"
#include <doctest/doctest.h>

/*
TrivialType1
NonTrivialDerived<1> : Interface<1>
TrivialType2
TrivialType34

DerivedType1 : Interface1
    - TrivialType1
DegivedType2 : Interface2
    - Interface1
    - TrivialType2
DerivedType34 : Interface3, Interface4
    - Interface1
    - Interface2
    - TrivialType34
*/

namespace liant::test {
using namespace linked;

TEST_CASE(
    "ensure 'resolveAll' on current container resolves all types from both current container and base container "
    "slice") {
    // clang-format off
    liant::ContainerSlice<TrivialType1, Interface1, Interface<1>> slice(liant::makeContainer(
        liant::registerInstanceOf<TrivialType1>(),
        liant::registerInstanceOf<NonTrivialDerived<1>>().as<Interface<1>>().bindArgs(111),
        liant::registerInstanceOf<DerivedType1>().as<Interface1>()
    ));
    auto container = liant::makeContainer(
        liant::baseContainer(slice),
        liant::registerInstanceOf<TrivialType2>(),
        liant::registerInstanceOf<TrivialType34>(),
        liant::registerInstanceOf<DerivedType2>().as<Interface2>(),
        liant::registerInstanceOf<DerivedType34>().as<Interface3, Interface4>()
    );
    // clang-format on
    container->resolveAll();

    REQUIRE(container->find<TrivialType1>());
    REQUIRE(container->find<Interface<1>>());
    REQUIRE(container->find<TrivialType2>());
    REQUIRE(container->find<TrivialType34>());
    REQUIRE(container->find<Interface1>());
    REQUIRE(container->find<Interface2>());
    REQUIRE(container->find<Interface3>());
    REQUIRE(container->find<Interface4>());
}

TEST_CASE("ensure 'resolveAll' on base container slice resolves only types from the base container slice") {
    // clang-format off
    liant::ContainerSlice<TrivialType1, Interface1> slice(liant::makeContainer(
        liant::registerInstanceOf<TrivialType1>(),
        liant::registerInstanceOf<DerivedType1>().as<Interface1>()
    ));
    auto container = liant::makeContainer(
        liant::baseContainer(slice),
        liant::registerInstanceOf<TrivialType2>(),
        liant::registerInstanceOf<TrivialType34>(),
        liant::registerInstanceOf<DerivedType2>().as<Interface2>(),
        liant::registerInstanceOf<DerivedType34>().as<Interface3, Interface4>()
    );
    // clang-format on
    slice->resolveAll();

    REQUIRE(container->find<TrivialType1>());
    REQUIRE(container->find<Interface1>());
    REQUIRE(!container->find<TrivialType2>());
    REQUIRE(!container->find<TrivialType34>());
    REQUIRE(!container->find<Interface2>());
    REQUIRE(!container->find<Interface3>());
    REQUIRE(!container->find<Interface4>());

    container->resolveAll();

    REQUIRE(container->find<TrivialType1>());
    REQUIRE(container->find<TrivialType2>());
    REQUIRE(container->find<TrivialType34>());
    REQUIRE(container->find<Interface1>());
    REQUIRE(container->find<Interface2>());
    REQUIRE(container->find<Interface3>());
    REQUIRE(container->find<Interface4>());
}

TEST_CASE(
    "ensure 'resolve' for a particular type resolved all its dependencies recursively from both base slice and current "
    "container") {
    // clang-format off
    liant::ContainerSlice<TrivialType1, Interface1> slice(liant::makeContainer(
        liant::registerInstanceOf<TrivialType1>(),
        liant::registerInstanceOf<DerivedType1>().as<Interface1>()
    ));
    auto container = liant::makeContainer(
        liant::baseContainer(slice),
        liant::registerInstanceOf<TrivialType2>(),
        liant::registerInstanceOf<TrivialType34>(),
        liant::registerInstanceOf<DerivedType2>().as<Interface2>(),
        liant::registerInstanceOf<DerivedType34>().as<Interface3, Interface4>()
    );
    // clang-format on
    container->resolve<Interface2>();

    REQUIRE(container->find<TrivialType1>());
    REQUIRE(container->find<Interface1>());
    REQUIRE(container->find<TrivialType2>());
    REQUIRE(container->find<Interface2>());
    REQUIRE(!container->find<TrivialType34>());
    REQUIRE(!container->find<Interface3>());
    REQUIRE(!container->find<Interface4>());

    // note: Interface3 implicitly resolves Interface4 as well
    container->resolve<Interface3>();

    REQUIRE(container->find<TrivialType1>());
    REQUIRE(container->find<TrivialType2>());
    REQUIRE(container->find<TrivialType34>());
    REQUIRE(container->find<Interface1>());
    REQUIRE(container->find<Interface2>());
    REQUIRE(container->find<Interface3>());
    REQUIRE(container->find<Interface4>());
}
} // namespace liant::test