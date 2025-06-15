#include "data.hpp"
#include "liant/liant.hpp"
#include <doctest/doctest.h>

namespace liant::test {

TEST_CASE("should resolve types with trivial ctor") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<Trivial<1>>(),
        liant::registerInstanceOf<Trivial<2>>()
    );
    // clang-format on
    container->resolveAll();

    REQUIRE(container->find<Trivial<1>>());
    REQUIRE(container->find<Trivial<2>>());

    REQUIRE(container->find<Trivial<1>>()->Id == 1);
    REQUIRE(container->find<Trivial<2>>()->Id == 2);
}

TEST_CASE("should resolve types with trivial ctor registered as interfaces") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<TrivialDerived<1>>().as<Interface<1>>(),
        liant::registerInstanceOf<TrivialDerived<2>>().as<Interface<2>>()
    );
    // clang-format on
    container->resolveAll();

    REQUIRE(container->find<Interface<1>>());
    REQUIRE(container->find<Interface<2>>());

    REQUIRE(container->find<Interface<1>>()->getId() == 1);
    REQUIRE(container->find<Interface<2>>()->getId() == 2);
}

TEST_CASE("should resolve types with non-trivial ctor and binded ctor arguments") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<NonTrivial<1>>().bindArgs(11),
        liant::registerInstanceOf<NonTrivial<2>>().bindArgs(22)
    );
    // clang-format on
    container->resolveAll();

    REQUIRE(container->find<NonTrivial<1>>());
    REQUIRE(container->find<NonTrivial<2>>());

    REQUIRE(container->find<NonTrivial<1>>()->Id == 11);
    REQUIRE(container->find<NonTrivial<2>>()->Id == 22);
}

TEST_CASE("should resolve types with non-trivial ctor and binded ctor arguments registered as interfaces") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<NonTrivialDerived<1>>().as<Interface<1>>().bindArgs(111),
        liant::registerInstanceOf<NonTrivialDerived<2>>().as<Interface<2>>().bindArgs(222)
    );
    // clang-format on
    container->resolveAll();

    REQUIRE(container->find<Interface<1>>());
    REQUIRE(container->find<Interface<2>>());

    REQUIRE(container->find<Interface<1>>()->getId() == 111);
    REQUIRE(container->find<Interface<2>>()->getId() == 222);
}

TEST_CASE("should resolve types with trivial ctor registered as multiple interfaces") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<TrivialDerivedMultipleInterfaces<1, Interface<1>, Interface<2>>>().as<Interface<1>, Interface<2>>(),
        liant::registerInstanceOf<TrivialDerivedMultipleInterfaces<2, Interface<3>, Interface<4>>>().as<Interface<3>, Interface<4>>()
    );
    // clang-format on
    container->resolveAll();

    REQUIRE(container->find<Interface<1>>());
    REQUIRE(container->find<Interface<2>>());
    REQUIRE(container->find<Interface<3>>());
    REQUIRE(container->find<Interface<4>>());

    REQUIRE(container->find<Interface<1>>()->getId() == 1);
    REQUIRE(container->find<Interface<2>>()->getId() == 1);
    REQUIRE(container->find<Interface<3>>()->getId() == 2);
    REQUIRE(container->find<Interface<4>>()->getId() == 2);
}

TEST_CASE("should resolve types with non-trivial ctor and binded ctor arguments registered as multiple interfaces") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<NonTrivialDerivedMultipleInterfaces<1, Interface<1>, Interface<2>>>().as<Interface<1>, Interface<2>>().bindArgs(11),
        liant::registerInstanceOf<NonTrivialDerivedMultipleInterfaces<2, Interface<3>, Interface<4>>>().as<Interface<3>, Interface<4>>().bindArgs(22)
    );
    // clang-format on
    container->resolveAll();

    REQUIRE(container->find<Interface<1>>());
    REQUIRE(container->find<Interface<2>>());
    REQUIRE(container->find<Interface<3>>());
    REQUIRE(container->find<Interface<4>>());

    REQUIRE(container->find<Interface<1>>()->getId() == 11);
    REQUIRE(container->find<Interface<2>>()->getId() == 11);
    REQUIRE(container->find<Interface<3>>()->getId() == 22);
    REQUIRE(container->find<Interface<4>>()->getId() == 22);
}

TEST_CASE("should instantiate different instances of the same trivial type if registered with different interfaces") {
    // clang-format off
    auto container = liant::makeContainer(
        // note: same type but different interfaces
        liant::registerInstanceOf<TrivialDerivedMultipleInterfaces<1, Interface<1>, Interface<2>>>().as<Interface<1>>(),
        liant::registerInstanceOf<TrivialDerivedMultipleInterfaces<1, Interface<1>, Interface<2>>>().as<Interface<2>>()
    );
    // clang-format on
    container->resolveAll();

    REQUIRE(container->find<Interface<1>>());
    REQUIRE(container->find<Interface<2>>());

    // note: different instances
    REQUIRE(static_cast<void*>(container->find<Interface<1>>().get()) !=
        static_cast<void*>(container->find<Interface<2>>().get()));

    REQUIRE(container->find<Interface<1>>()->getId() == 1);
    REQUIRE(container->find<Interface<2>>()->getId() == 1);
}

TEST_CASE(
    "should instantiate different instances of the same non-trivial type with binded ctor arguments if registered with "
    "different interfaces") {
    // clang-format off
    auto container = liant::makeContainer(
        // note: same type but different interfaces
        liant::registerInstanceOf<NonTrivialDerivedMultipleInterfaces<1, Interface<1>, Interface<2>>>().as<Interface<1>>().bindArgs(11),
        liant::registerInstanceOf<NonTrivialDerivedMultipleInterfaces<1, Interface<1>, Interface<2>>>().as<Interface<2>>().bindArgs(22)
    );
    // clang-format on
    container->resolveAll();

    REQUIRE(container->find<Interface<1>>());
    REQUIRE(container->find<Interface<2>>());

    // note: two different instances were created
    REQUIRE(static_cast<void*>(container->find<Interface<1>>().get()) !=
        static_cast<void*>(container->find<Interface<2>>().get()));

    REQUIRE(container->find<Interface<1>>()->getId() == 11);
    REQUIRE(container->find<Interface<2>>()->getId() == 22);
}
} // namespace liant::test