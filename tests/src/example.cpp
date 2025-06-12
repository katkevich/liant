#include "liant/liant.hpp"
#include <iostream>

struct Interface1 {
    virtual ~Interface1() = default;
};

struct Interface2 {
    virtual ~Interface2() = default;
};

struct Interface3 {
    virtual ~Interface3() = default;
};

LIANT_DEPENDENCY(Interface2, interfaceTwo)
LIANT_DEPENDENCY(Interface3, interfaceThree)


struct Type1 : Interface1 {
    void postCreate() const {
        std::cout << "type1 postCreate\n";
    }
    void preDestroy() const {
        std::cout << "type1 preDestroy\n";
    }
};

struct Type2 : Interface2 {
    Type2(liant::Dependencies<Interface3> di, std::string s)
        : di(di)
        , s(s) {}

    liant::Dependencies<Interface3> di;

    std::string s;

    void postCreate() const {
        std::cout << "type2 postCreate\n";
    }
    void preDestroy() const {
        std::cout << "type2 preDestroy\n";
    }
};

struct Type3 : Interface3 {
    Type3(liant::Dependencies<Interface1> di)
        : di(di) {}

    liant::Dependencies<Interface1> di;

    // void postCreate() const {
    //     std::cout << "type3 postCreate\n";
    // }
    // void preDestroy() const {
    //     std::cout << "type3 preDestroy\n";
    // }
};


struct Type4 {
    void postCreate() const {
        std::cout << "type4 postCreate\n";
    }
    void preDestroy() const {
        std::cout << "type4 preDestroy\n";
    }
};
struct Type5 {};

int main() {
    // clang-format off
    auto container = liant::makeContainer(
        liant::registerType<Type1>().as<Interface1>(),
        liant::registerType<Type4>(),
        liant::registerType<Type2>().as<Interface2>().bindArgs("hello, world"),
        liant::registerType<Type3>().as<Interface3>()
    );
    // clang-format on

    using ContainerType = decltype(container);
    // container->create<Interface1>();
    // container->create<Interface2>();
    // container->create<Type4>();
    // container->resolve<Type4>();
    container->find<Interface1>();

    container->resolve<Interface2>("blah blah");
    container->resolve<Interface3>();
    container->resolveAll();


    Interface2* i2 = container->find<Interface2>();

    std::cout << static_cast<Type2*>(i2)->s << std::endl;

    return static_cast<Type2*>(i2)->s == "hello, world";


    // Interface1* interface = container->resolve<Interface1>();
}