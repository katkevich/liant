import liant;
import std;

// ain't strictly necessary but nice to have if you want use 'LIANT_DEPENDENCY' functionality
#include "liant/dependency_macro.hpp"

namespace example {
struct Type1 {
    void postCreate() {
        std::println("Type1 postCreate");
    }

    void preDestroy() {
        std::println("Type1 preDestroy");
    }
};
LIANT_DEPENDENCY(Type1, prettyCustomGetType1)

struct Type2 {
    liant::Dependencies<Type1> di;
};
} // namespace example

int main() {
    std::println("Hello from Liant module with C++23 'import std'");

    auto container = liant::makeContainer(           //
        liant::registerInstanceOf<example::Type1>(), //
        liant::registerInstanceOf<example::Type2>());
    container->resolveAll();

    liant::SharedPtr type2Inst = container->find<example::Type2>();

    type2Inst->di.prettyCustomGetType1();

    return 0;
}