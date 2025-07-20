#include "data.hpp"
#include "liant/liant.hpp"
#include <doctest/doctest.h>

namespace liant::test {

struct S1 {
    int i = 1;
};
struct S2 {
    int i = 2;
};
struct S3 {
    int i = 3;
};
struct S4 {
    int i = 4;
};
struct S5 {
    int i = 5;
};

TEST_CASE("should construct ContainerSliceLazy from same type ContainerSliceLazy") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSliceLazy<S1, S2, S3, S4> slice(container);
    liant::ContainerSliceLazy<S1, S2, S3, S4> slice2(slice);

    REQUIRE_EQ(container.use_count(), 3);

    slice2.resolve<S2>();
    slice2.resolve<S3>();

    REQUIRE_FALSE(slice2.find<S1>());
    REQUIRE_EQ(slice2.find<S2>()->i, 2);
    REQUIRE_EQ(slice2.find<S3>()->i, 3);
    REQUIRE_FALSE(slice2.find<S4>());

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("should construct ContainerSliceLazy from ContainerSliceLazy with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSliceLazy<S1, S2, S3, S4> slice(container);
    liant::ContainerSliceLazy<S1, S2> slice2(slice);

    REQUIRE_EQ(container.use_count(), 3);

    slice2.resolve<S2>();

    REQUIRE_FALSE(slice2.find<S1>());
    REQUIRE_EQ(slice2.find<S2>()->i, 2);

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("should construct ContainerSliceLazy from Container with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSliceLazy<S1, S2> slice(container);

    REQUIRE_EQ(container.use_count(), 2);

    REQUIRE_FALSE(slice.find<S1>());
    REQUIRE_FALSE(slice.find<S2>());

    slice.resolve<S1>();

    REQUIRE_EQ(slice.find<S1>()->i, 1);
    REQUIRE_FALSE(slice.find<S2>());

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("should construct ContainerViewLazy from ContainerViewLazy") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerViewLazy<S1, S2, S3, S4> view(container);
    liant::ContainerViewLazy<S1, S2, S3, S4> view2(view);

    REQUIRE_EQ(container.use_count(), 1);

    view2.resolve<S2>();
    view2.resolve<S3>();

    REQUIRE_FALSE(view2.find<S1>());
    REQUIRE_EQ(view2.find<S2>()->i, 2);
    REQUIRE_EQ(view2.find<S3>()->i, 3);
    REQUIRE_FALSE(view2.find<S4>());

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("should construct ContainerViewLazy from ContainerViewLazy with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerViewLazy<S1, S2, S3, S4> view(container);
    liant::ContainerViewLazy<S1, S2> view2(view);

    REQUIRE_EQ(container.use_count(), 1);

    view2.resolve<S1>();

    REQUIRE_EQ(view2.find<S1>()->i, 1);
    REQUIRE_FALSE(view2.find<S2>());

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("should construct ContainerViewLazy from Container with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerViewLazy<S1, S2> view(container);

    REQUIRE_EQ(container.use_count(), 1);

    view.resolve<S1>();

    REQUIRE_EQ(view.find<S1>()->i, 1);
    REQUIRE_FALSE(view.find<S2>());

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("should construct ContainerViewLazy from ContainerSlice with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSliceLazy<S1, S2, S3, S4> slice(container);
    liant::ContainerViewLazy<S1, S2> view(slice);

    REQUIRE_EQ(container.use_count(), 2);

    view.resolve<S1>();

    REQUIRE_EQ(view.find<S1>()->i, 1);
    REQUIRE_FALSE(view.find<S2>());

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("should construct ContainerSliceLazy from ContainerViewLazy with wider interface (different order)") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerViewLazy<S1, S2, S3, S4> view(container);
    liant::ContainerSliceLazy<S2, S1> slice(view);

    REQUIRE_EQ(container.use_count(), 2);

    view.resolve<S1>();

    REQUIRE_EQ(view.find<S1>()->i, 1);
    REQUIRE_FALSE(view.find<S2>());

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("ensure ContainerSliceLazy assignment indeed assign different container under the hood") {
    // clang-format off
    auto container1 = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>()
    );
    auto container2 = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>()
    );
    // clang-format on

    liant::ContainerSliceLazy<S1, S2> slice1(container1);
    liant::ContainerSliceLazy<S1, S2, S3> slice2(container2);

    REQUIRE_EQ(container1.use_count(), 2);
    REQUIRE_EQ(container2.use_count(), 2);
    REQUIRE_EQ(slice1.useCount(), 2);
    REQUIRE_EQ(slice2.useCount(), 2);

    slice2.resolve<S2>();

    REQUIRE_FALSE(slice1.find<S1>());
    REQUIRE_FALSE(slice1.find<S2>());

    REQUIRE_FALSE(slice2.find<S1>());
    REQUIRE_EQ(slice2.find<S2>()->i, 2);

    slice1 = slice2;

    REQUIRE_EQ(container1.use_count(), 1);
    REQUIRE_EQ(container2.use_count(), 3);
    REQUIRE_EQ(slice1.useCount(), 3);
    REQUIRE_EQ(slice2.useCount(), 3);

    REQUIRE_FALSE(slice1.find<S1>());
    REQUIRE_EQ(slice1.find<S2>()->i, 2);

    REQUIRE_FALSE(slice2.find<S1>());
    REQUIRE_EQ(slice2.find<S2>()->i, 2);
}

TEST_CASE("should construct ContainerSlice from ContainerViewLazy with wider interface (different order)") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerViewLazy<S1, S2, S3> view(container);
    REQUIRE_EQ(container.use_count(), 1);
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());

    liant::ContainerSlice<S2, S1> slice(view);
    REQUIRE_EQ(container.use_count(), 2);
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());

    view.resolve<S3>();

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_FALSE(container->find<S4>());
}
} // namespace liant::test