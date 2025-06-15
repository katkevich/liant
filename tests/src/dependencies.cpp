#include "data.hpp"
#include "liant/liant.hpp"
#include <doctest/doctest.h>

namespace liant::test {

struct Interface1 {
    virtual ~Interface1() = default;
    virtual std::string getTag() = 0;
};
LIANT_DEPENDENCY(Interface1, Interface1_PrettyGetter)

struct Interface2 {
    virtual ~Interface2() = default;
    virtual std::string getTag() = 0;
};
LIANT_DEPENDENCY(Interface2, Interface2_PrettyGetter)

struct DerivedType1 : Interface1 {
    DerivedType1(std::string tag)
        : tag(tag) {}
    DerivedType1(std::string tag, Stats& stats)
        : tag(tag)
        , stats(&stats) {}

    virtual std::string getTag() override {
        return tag;
    }

    void postCreate() {
        if (stats) {
            stats->creationOrder += "DerivedType1 ";
        }
    }

    void preDestroy() {
        if (stats) {
            stats->destroyingOrder += "DerivedType1 ";
        }
    }

    std::string tag;
    Stats* stats{};
};

struct DerivedType2 : Interface2 {
    DerivedType2(liant::Dependencies<Interface1> di, std::string tagPrefix, std::string tagSuffix)
        : di(di)
        , tag(tagPrefix + tagSuffix) {}
    DerivedType2(liant::Dependencies<Interface1> di, std::string tagPrefix, std::string tagSuffix, Stats& stats)
        : di(di)
        , tag(tagPrefix + tagSuffix)
        , stats(&stats) {}

    virtual std::string getTag() override {
        return tag;
    }

    void postCreate() {
        if (stats) {
            stats->creationOrder += "DerivedType2 ";
        }
    }

    void preDestroy() {
        if (stats) {
            stats->destroyingOrder += "DerivedType2 ";
        }
    }

    liant::Dependencies<Interface1> di;
    std::string tag;
    Stats* stats{};
};

struct Type3 {
    Type3(liant::Dependencies<Interface2, Interface1> di, std::string tag)
        : di(di)
        , tag(tag) {}

    Type3(liant::Dependencies<Interface2, Interface1> di, std::string tag, Stats& stats)
        : di(di)
        , tag(tag)
        , stats(&stats) {}

    void postCreate() {
        if (stats) {
            stats->creationOrder += "Type3 ";
        }
    }

    void preDestroy() {
        if (stats) {
            stats->destroyingOrder += "Type3 ";
        }
    }

    liant::Dependencies<Interface2, Interface1> di;
    std::string tag;
    Stats* stats{};
};

TEST_CASE(
    "ensure resolve(arguments...) creates a dependency using these arguments... and not binded arguments from "
    "Container") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<DerivedType1>().as<Interface1>().bindArgs("arg1"),
        liant::registerInstanceOf<DerivedType2>().as<Interface2>().bindArgs("ar", "g2"),
        liant::registerInstanceOf<Type3>().bindArgs("arg3")
    );
    // clang-format on

    Interface2& interface2 = container->resolveRaw<Interface2>();
    REQUIRE_EQ(interface2.getTag(), "arg2");

    // note: "explicit arg" overrides binded "arg" (see 'bindArgs("arg")')
    Type3& type3 = container->resolveRaw<Type3>("explicit arg");
    REQUIRE_EQ(type3.tag, "explicit arg");
}

TEST_CASE("ensure LIANT_DEPENDENCY macro defines pretty getters for a dependency") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<DerivedType1>().as<Interface1>().bindArgs("arg1"),
        liant::registerInstanceOf<DerivedType2>().as<Interface2>().bindArgs("ar", "g2"),
        liant::registerInstanceOf<Type3>().bindArgs("arg3")
    );
    // clang-format on
    Type3& type3 = container->resolveRaw<Type3>();

    // note: pretty getter 'Interface1_PrettyGetter'
    Interface1& interface1 = type3.di.Interface1_PrettyGetter();
    REQUIRE_EQ(interface1.getTag(), "arg1");

    // note: pretty getter 'Interface2_PrettyGetter'
    Interface2& interface2 = type3.di.Interface2_PrettyGetter();
    REQUIRE_EQ(interface2.getTag(), "arg2");
}


TEST_CASE("ensure destroying order is opposite to the creation order of the dependencies") {
    Stats stats;

    // clang-format off
    auto container = liant::makeContainer(
        // note: use std::reference_wrapper in order to bind references
        liant::registerInstanceOf<DerivedType2>().as<Interface2>().bindArgs("ar", "g2", std::ref(stats)),
        liant::registerInstanceOf<Type3>().bindArgs("arg3", std::ref(stats)),
        liant::registerInstanceOf<DerivedType1>().as<Interface1>().bindArgs("arg1", std::ref(stats))
    );
    // clang-format on

    // note: not yet created anything
    REQUIRE_EQ(stats.creationOrder, "");

    container->resolveAll();

    // note: creation order is different from the declaration order and is determined by interdependencies between registered types
    REQUIRE_EQ(stats.creationOrder, "DerivedType1 DerivedType2 Type3 ");

    // note: not yet destooyed anything
    REQUIRE_EQ(stats.destroyingOrder, "");

    container.reset();

    // note: destroying order is opposite to the creation order
    REQUIRE_EQ(stats.destroyingOrder, "Type3 DerivedType2 DerivedType1 ");
}

TEST_CASE(
    "ensure destroying order is opposite to the creation order of the dependencies (even when manually resolving some "
    "of the dependencies)") {
    Stats stats;

    // clang-format off
    auto container = liant::makeContainer(
        // note: use std::reference_wrapper in order to bind references
        liant::registerInstanceOf<DerivedType2>().as<Interface2>().bindArgs("ar", "g2", std::ref(stats)),
        liant::registerInstanceOf<Type3>().bindArgs("arg3", std::ref(stats)),
        liant::registerInstanceOf<DerivedType1>().as<Interface1>().bindArgs("arg1", std::ref(stats))
    );
    // clang-format on

    REQUIRE_EQ(stats.creationOrder, "");

    container->resolve<Interface2>();
    REQUIRE_EQ(stats.creationOrder, "DerivedType1 DerivedType2 ");

    container->resolve<Type3>();
    REQUIRE_EQ(stats.creationOrder, "DerivedType1 DerivedType2 Type3 ");

    container->resolveAll();
    REQUIRE_EQ(stats.creationOrder, "DerivedType1 DerivedType2 Type3 ");

    container.reset();

    // note: destroying order is opposite to the creation order
    REQUIRE_EQ(stats.destroyingOrder, "Type3 DerivedType2 DerivedType1 ");
}

TEST_CASE("ensure dependency ain't destroyed if it wasn't created in the first place") {
    Stats stats;

    // clang-format off
    auto container = liant::makeContainer(
        // note: use std::reference_wrapper in order to bind references
        liant::registerInstanceOf<DerivedType2>().as<Interface2>().bindArgs("ar", "g2", std::ref(stats)),
        liant::registerInstanceOf<Type3>().bindArgs("arg3", std::ref(stats)),
        liant::registerInstanceOf<DerivedType1>().as<Interface1>().bindArgs("arg1", std::ref(stats))
    );
    // clang-format on

    REQUIRE_EQ(stats.creationOrder, "");

    container->resolve<Interface2>();
    REQUIRE_EQ(stats.creationOrder, "DerivedType1 DerivedType2 ");

    container.reset();

    // note: destroying order is opposite to the creation order
    // note: missing Type3 was never created
    REQUIRE_EQ(stats.destroyingOrder, "DerivedType2 DerivedType1 ");
}

TEST_CASE("ensure dependencies support optional postCreate/preDestroy customization points") {
    Stats stats;
    struct Foo {
        Foo(Stats& stats)
            : stats(stats) {}

        void postCreate() {
            stats.creationOrder += "Foo ";
        }
        void preDestroy() {
            stats.destroyingOrder += "Foo ";
        }

        Stats& stats;
    };

    struct Bar {};

    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<Foo>().bindArgs(std::ref(stats)),
        liant::registerInstanceOf<Bar>().bindArgs()
    );
    // clang-format on

    container->resolveAll();

    // note: postCreate kicked in and modified 'stats'
    REQUIRE_EQ(stats.creationOrder, "Foo ");

    container.reset();

    // note: preDestroy kicked in and modified 'stats'
    REQUIRE_EQ(stats.destroyingOrder, "Foo ");
}

TEST_CASE("ensure aggregates can be created as is without explicit ctor") {
    struct Payload {};

    struct NoDependencies {
        Payload& payloadRef;
    };

    struct WithDependencies {
        liant::Dependencies<NoDependencies> di;
        Payload& payloadRef;
    };

    Payload payload;

    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<WithDependencies>().bindArgs(std::ref(payload)),
        liant::registerInstanceOf<NoDependencies>().bindArgs(std::ref(payload))
    );
    // clang-format on

    container->resolve<WithDependencies>();

    REQUIRE(container->find<NoDependencies>());
    REQUIRE(container->find<WithDependencies>());
}

TEST_CASE("ensure empty liant::SharedPtr doesn't keep container alive") {

    Stats stats;

    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<Trackable<1>>().bindArgs(std::ref(stats)),
        liant::registerInstanceOf<Trackable<2>>().bindArgs(std::ref(stats))
    );
    // clang-format on

    std::weak_ptr weakContainer(container);

    REQUIRE_EQ(stats.destroyingOrder, "");

    container->resolve<Trackable<1>>();

    SharedPtr<Trackable<2>> sharedPtr = container->find<Trackable<2>>();
    // note: sharedPtr is in fact empty (isn't pointing to dependency coz dependency isn't resolved (created) yet)
    REQUIRE_FALSE(bool(sharedPtr));

    container.reset();

    // note: container is destroyed!
    REQUIRE(weakContainer.expired());
    REQUIRE_EQ(stats.destroyingOrder, "Trackable1 ");
}

TEST_CASE("ensure non-empty liant::SharedPtr keeps container alive") {

    Stats stats;

    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<Trackable<1>>().bindArgs(std::ref(stats)),
        liant::registerInstanceOf<Trackable<2>>().bindArgs(std::ref(stats))
    );
    // clang-format on

    std::weak_ptr weakContainer(container);

    REQUIRE_EQ(stats.destroyingOrder, "");

    container->resolveAll();

    SharedPtr<Trackable<2>> sharedPtr = container->find<Trackable<2>>();
    // note: sharedPtr does in fact contain pointer to the dependency
    REQUIRE(sharedPtr);

    container.reset();

    // note: container is still alive!
    REQUIRE(!weakContainer.expired());
    REQUIRE_EQ(stats.destroyingOrder, "");
    REQUIRE(weakContainer.lock()->findRaw<Trackable<1>>());

    sharedPtr.reset();

    // note: container is finally destroyed
    REQUIRE(weakContainer.expired());
}

TEST_CASE("ensure liant::SharedRef (never empty) keeps container alive") {

    Stats stats;

    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<Trackable<1>>().bindArgs(std::ref(stats)),
        liant::registerInstanceOf<Trackable<2>>().bindArgs(std::ref(stats))
    );
    // clang-format on

    std::weak_ptr weakContainer(container);

    REQUIRE_EQ(stats.destroyingOrder, "");

    {
        SharedRef sharedRef = container->resolve<Trackable<1>>();

        container.reset();

        // note: container is still alive!
        REQUIRE(!weakContainer.expired());
        REQUIRE_EQ(stats.destroyingOrder, "");
        REQUIRE(weakContainer.lock()->findRaw<Trackable<1>>());
    }
    // note: sharedRef goes out of scope

    // note: container is finally destroyed
    REQUIRE(weakContainer.expired());
}
} // namespace liant::test