#include "liant/liant.hpp"

struct Interface1 {
    virtual ~Interface1() = default;
};

struct Type1 : Interface1 {};

struct Interface2 {
    virtual ~Interface2() = default;
};
LIANT_DEPENDENCY(Interface2, interfaceTwo)

struct Type2 : Interface2 {
    Type2(liant::Dependencies<Interface1> di)
        : di(di) {}

    liant::Dependencies<Interface1> di;
};

struct Interface3 {
    virtual ~Interface3() = default;
};
LIANT_DEPENDENCY(Interface3, interfaceThree)

struct Type3 : Interface3 {
    Type3(liant::Dependencies<Interface2, Interface1> di)
        : di(di) {}

    liant::Dependencies<Interface2, Interface1> di;
};


struct Type4 {};

int main() {
    // clang-format off
    auto container = liant::makeContainer(
        liant::missing_dependency_policy::Terminate,
        liant::registerType<Type1>().as<Interface1>(),
        liant::registerType<Type4>(),
        liant::registerType<Type2>().as<Interface2>(),
        liant::registerType<Type3>().as<Interface3>());
    // clang-format on

    container->create<Interface1>();
    container->create<Interface2>();
    container->create<Interface3>();

    Interface1* interface = container->resolve<Interface1>();
}