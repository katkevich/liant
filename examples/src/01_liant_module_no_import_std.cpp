import liant;
#include <cstdio>

// ain't strictly necessary but nice to have if you want use 'LIANT_DEPENDENCY' functionality
#include "liant/dependency_macro.hpp"

namespace example {
struct Type1 {
    void postCreate() {
        std::puts("Type1 postCreate");
    }

    void preDestroy() {
        std::puts("Type1 preDestroy");
    }
};
LIANT_DEPENDENCY(Type1, prettyCustomGetType1)

struct Type2 {
    liant::Dependencies<Type1> di;
};
} // namespace example

int main() {
    std::puts("Hello from Liant module");

    auto container = liant::makeContainer(           //
        liant::registerInstanceOf<example::Type1>(), //
        liant::registerInstanceOf<example::Type2>());
    container->resolveAll();

    liant::SharedPtr type2Inst = container->find<example::Type2>();

    type2Inst->di.prettyCustomGetType1();

    return 0;
}