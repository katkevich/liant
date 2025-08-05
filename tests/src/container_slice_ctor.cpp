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

TEST_CASE("should construct ContainerSlice from same type ContainerSlice") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSlice<S1, S2, S3, S4> slice(container);
    liant::ContainerSlice<S1, S2, S3, S4> slice2(slice);

    REQUIRE_EQ(container.use_count(), 3);

    REQUIRE_EQ(slice2.find<S1>()->i, 1);
    REQUIRE_EQ(slice2.find<S2>()->i, 2);
    REQUIRE_EQ(slice2.find<S3>()->i, 3);
    REQUIRE_EQ(slice2.find<S4>()->i, 4);

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_EQ(container->find<S4>()->i, 4);
}

TEST_CASE("should construct ContainerSlice from ContainerSlice with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSlice<S1, S2, S3, S4> slice(container);
    liant::ContainerSlice<S1, S2> slice2(slice);

    REQUIRE_EQ(container.use_count(), 3);

    REQUIRE_EQ(slice2.find<S1>()->i, 1);
    REQUIRE_EQ(slice2.find<S2>()->i, 2);

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    // S3 & S4 are being resolved during 'liant::ContainerSlice<S1, S2, S3, S4>' creation
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_EQ(container->find<S4>()->i, 4);
}


TEST_CASE("should construct ContainerSlice from Container with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSlice<S1, S2> slice(container);

    REQUIRE_EQ(container.use_count(), 2);

    REQUIRE_EQ(slice.find<S1>()->i, 1);
    REQUIRE_EQ(slice.find<S2>()->i, 2);

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("should construct ContainerView from ContainerView") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerView<S1, S2, S3, S4> view(container);
    liant::ContainerView<S1, S2, S3, S4> view2(view);

    REQUIRE_EQ(container.use_count(), 1);

    REQUIRE_EQ(view2.find<S1>()->i, 1);
    REQUIRE_EQ(view2.find<S2>()->i, 2);
    REQUIRE_EQ(view2.find<S3>()->i, 3);
    REQUIRE_EQ(view2.find<S4>()->i, 4);

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_EQ(container->find<S4>()->i, 4);
}


TEST_CASE("should construct ContainerView from ContainerView with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerView<S1, S2, S3, S4> view(container);
    liant::ContainerView<S1, S2> view2(view);

    REQUIRE_EQ(container.use_count(), 1);

    REQUIRE_EQ(view2.find<S1>()->i, 1);
    REQUIRE_EQ(view2.find<S2>()->i, 2);

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    // S3 & S4 are being resolved during 'liant::ContainerSlice<S1, S2, S3, S4>' creation
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_EQ(container->find<S4>()->i, 4);
}

TEST_CASE("should construct ContainerView from Container with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerView<S1, S2> view(container);

    REQUIRE_EQ(container.use_count(), 1);

    REQUIRE_EQ(view.find<S1>()->i, 1);
    REQUIRE_EQ(view.find<S2>()->i, 2);

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("should construct ContainerView from ContainerSlice with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSlice<S1, S2, S3, S4> slice(container);
    liant::ContainerView<S1, S2> view(slice);

    REQUIRE_EQ(container.use_count(), 2);

    REQUIRE_EQ(view.find<S1>()->i, 1);
    REQUIRE_EQ(view.find<S2>()->i, 2);

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    // S3 & S4 are being resolved during 'liant::ContainerSlice<S1, S2, S3, S4>' creation
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_EQ(container->find<S4>()->i, 4);
}

TEST_CASE("should construct ContainerSlice from ContainerView with wider interface (different order)") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerView<S1, S2, S3, S4> view(container);
    liant::ContainerSlice<S2, S1> slice(view);

    REQUIRE_EQ(container.use_count(), 2);

    REQUIRE_EQ(slice.find<S1>()->i, 1);
    REQUIRE_EQ(slice.find<S2>()->i, 2);

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    // S3 & S4 are being resolved during 'liant::ContainerSlice<S1, S2, S3, S4>' creation
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_EQ(container->find<S4>()->i, 4);
}

TEST_CASE("ensure ContainerSlice assign to itself ain't broken") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSlice<S1, S2, S3> slice(container);
    REQUIRE_EQ(container.use_count(), 2);

    auto& other = slice;
    slice = other;
    REQUIRE_EQ(container.use_count(), 2);

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_FALSE(container->find<S4>());
}


TEST_CASE("should construct ContainerSliceWeak from ContainerSlice with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSlice<S1, S2, S3, S4> slice(container);
    liant::ContainerSliceWeak<S2, S1> sliceWeak(slice);

    REQUIRE_EQ(container.use_count(), 2);

    liant::ContainerSlice<S2, S1> slice2 = sliceWeak.lock();
    REQUIRE_EQ(container.use_count(), 3);

    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S3>()->i, 3);
    REQUIRE_EQ(container->find<S4>()->i, 4);
}

TEST_CASE("should construct ContainerSliceWeakLazy from ContainerSliceLazy with wider interface") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSliceLazy<S1, S2, S3, S4> slice(container);
    liant::ContainerSliceWeakLazy<S2, S1> sliceWeak(slice);

    REQUIRE_EQ(container.use_count(), 2);

    liant::ContainerSliceLazy<S2, S1> slice2 = sliceWeak.lock();
    REQUIRE_EQ(container.use_count(), 3);

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());

    slice2.resolve<S2>();

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_EQ(container->find<S2>()->i, 2);
    REQUIRE_FALSE(container->find<S3>());
    REQUIRE_FALSE(container->find<S4>());
}

TEST_CASE("Ensure multiple layers of indirection work") {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerInstanceOf<S1>(),
        liant::registerInstanceOf<S2>(),
        liant::registerInstanceOf<S3>(),
        liant::registerInstanceOf<S4>()
    );
    // clang-format on

    liant::ContainerSliceLazy<S1, S2, S3, S4> slice1(container);
    liant::ContainerSliceLazy<S2, S3, S4> slice2(slice1);
    liant::ContainerSliceLazy<S3, S4> slice3(slice2);
    liant::ContainerSliceLazy<S4> slice4(slice3);

    REQUIRE_EQ(container.use_count(), 5);

    slice4.resolve<S4>();

    REQUIRE_EQ(slice4.find<S4>()->i, 4);
}

TEST_CASE("should construct ContainerSlice from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    // all instances will be resolved here
    liant::ContainerSlice<S1, S2> slice(view);

    REQUIRE(container->find<S1>());
    REQUIRE(container->find<S2>());
}

TEST_CASE("should construct ContainerSliceLazy from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSliceLazy<S1, S2> slice(view);

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should construct ContainerSliceWeak from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    // all instances will be resolved here
    liant::ContainerSliceWeak<S1, S2> slice(view);

    REQUIRE(container->find<S1>());
    REQUIRE(container->find<S2>());
}

TEST_CASE("should construct ContainerSliceWeakLazy from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSliceWeakLazy<S1, S2> slice(view);

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should construct ContainerView from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    // all instances will be resolved here
    liant::ContainerView<S1, S2> nonLazyView(view);

    REQUIRE(container->find<S1>());
    REQUIRE(container->find<S2>());
}

TEST_CASE("should construct ContainerViewLazy from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerViewLazy<S1, S2> view2(view);

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
}


TEST_CASE("should assign ContainerSlice from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSlice<S1, S2> slice(container2);

    REQUIRE_EQ(slice.find<S1>()->i, 42);
    REQUIRE_EQ(slice.find<S2>()->i, 43);
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    slice = view;
    REQUIRE_EQ(slice.find<S1>()->i, 1);
    REQUIRE_EQ(slice.find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
}

TEST_CASE("should assign ContainerSliceLazy from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSliceLazy<S1, S2> slice(container2);

    REQUIRE_EQ(slice.find<S1>()->i, 42);
    REQUIRE_EQ(slice.find<S2>()->i, 43);

    slice = view;
    REQUIRE_FALSE(slice.find<S1>());
    REQUIRE_FALSE(slice.find<S2>());
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    slice.resolveAll();
    REQUIRE_EQ(slice.find<S1>()->i, 1);
    REQUIRE_EQ(slice.find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
}

TEST_CASE("should assign ContainerSliceWeak from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSliceWeak<S1, S2> slice(container2);

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    slice = view;
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
}

TEST_CASE("should assign ContainerSliceWeakLazy from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSliceWeakLazy<S1, S2> slice(container2);

    slice = view;
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    slice.lock().resolveAll();
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
}

TEST_CASE("should assign ContainerView from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerView<S1, S2> view2(container2);

    REQUIRE_EQ(view2.find<S1>()->i, 42);
    REQUIRE_EQ(view2.find<S2>()->i, 43);
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    view2 = view;
    REQUIRE_EQ(view2.find<S1>()->i, 1);
    REQUIRE_EQ(view2.find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
}

TEST_CASE("should assign ContainerViewLazy from ContainerViewLazy (same interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerViewLazy<S1, S2> view2(container2);

    REQUIRE_EQ(view2.find<S1>()->i, 42);
    REQUIRE_EQ(view2.find<S2>()->i, 43);

    view2 = view;
    REQUIRE_FALSE(view2.find<S1>());
    REQUIRE_FALSE(view2.find<S2>());
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    view2.resolveAll();
    REQUIRE_EQ(view2.find<S1>()->i, 1);
    REQUIRE_EQ(view2.find<S2>()->i, 2);
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S2>()->i, 2);
}
////////////////////////////////////////////////////
TEST_CASE("should construct ContainerSlice from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    // S1 instance will be resolved here
    liant::ContainerSlice<S1> slice(view);

    REQUIRE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should construct ContainerSliceLazy from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSliceLazy<S1> slice(view);

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    slice.resolveAll();

    REQUIRE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should construct ContainerSliceWeak from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    // S1 instance will be resolved here
    liant::ContainerSliceWeak<S1> slice(view);

    REQUIRE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should construct ContainerSliceWeakLazy from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSliceWeakLazy<S1> slice(view);

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    slice.lock().resolveAll();

    REQUIRE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should construct ContainerView from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    // S1 instance will be resolved here
    liant::ContainerView<S1> nonLazyView(view);

    REQUIRE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should construct ContainerViewLazy from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerViewLazy<S1> view2(view);

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    view2.resolveAll();

    REQUIRE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());
}


TEST_CASE("should assign ContainerSlice from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSlice<S1> slice(container2);

    REQUIRE_EQ(slice.find<S1>()->i, 42);
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    slice = view;
    REQUIRE_EQ(slice.find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should assign ContainerSliceLazy from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSliceLazy<S1> slice(container2);

    REQUIRE_EQ(slice.find<S1>()->i, 42);

    slice = view;
    REQUIRE_FALSE(slice.find<S1>());
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    slice.resolveAll();
    REQUIRE_EQ(slice.find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should assign ContainerSliceWeak from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSliceWeak<S1> slice(container2);

    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    slice = view;
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should assign ContainerSliceWeakLazy from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerSliceWeakLazy<S1> slice(container2);

    slice = view;
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    slice.lock().resolveAll();
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should assign ContainerView from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerView<S1> view2(container2);

    REQUIRE_EQ(view2.find<S1>()->i, 42);
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    view2 = view;
    REQUIRE_EQ(view2.find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
}

TEST_CASE("should assign ContainerViewLazy from ContainerViewLazy (narrower interfaces)") {
    auto container = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    auto container2 = liant::makeContainer(liant::registerInstanceOf<S1>(), liant::registerInstanceOf<S2>());
    container2->resolveRaw<S1>().i = 42;
    container2->resolveRaw<S2>().i = 43;

    liant::ContainerViewLazy<S1, S2> view(container);
    liant::ContainerViewLazy<S1> view2(container2);

    REQUIRE_EQ(view2.find<S1>()->i, 42);

    view2 = view;
    REQUIRE_FALSE(view2.find<S1>());
    REQUIRE_FALSE(container->find<S1>());
    REQUIRE_FALSE(container->find<S2>());

    view2.resolveAll();
    REQUIRE_EQ(view2.find<S1>()->i, 1);
    REQUIRE_EQ(container->find<S1>()->i, 1);
    REQUIRE_FALSE(container->find<S2>());
}
} // namespace liant::test