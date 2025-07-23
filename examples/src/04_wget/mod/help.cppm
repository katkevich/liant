module;
#include "liant/dependency_macro.hpp"

export module wget:help;

import liant;

export namespace wget {
class HelpPrinter {
public:
    void print() const;
};
LIANT_DEPENDENCY(HelpPrinter, help)
} // namespace wget