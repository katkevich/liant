import liant;
import std;
// #include "liant/liant.hpp"

namespace example {
struct Interface1 {};
struct Interface2 {};
struct Interface3 {};

struct Type1 : Interface1 {
    Type1(liant::ContainerView<Interface3> di)
        : di(di) {}
    liant::ContainerView<Interface3> di;
};

struct Type2 : Interface2 {
    Type2(liant::ContainerView<Interface1> di)
        : di(di) {}
    liant::ContainerView<Interface1> di;
};

struct Type3 : Interface3 {
    Type3(liant::ContainerView<Interface2> di)
        : di(di) {}
    liant::ContainerView<Interface2> di;
};
} // namespace example

int main() {
    using namespace example;

    auto container = liant::makeContainer(                   //
        liant::registerInstanceOf<Type1>().as<Interface1>(), //
        liant::registerInstanceOf<Type2>().as<Interface2>(), //
        liant::registerInstanceOf<Type3>().as<Interface3>());

    // clang: error: static assertion failed due to requirement '!TypeList<example::Interface3, example::Interface2, example::Interface1>::contains()': Detected cycle in your dependencies
    container->resolveAll();

    return 0;
}