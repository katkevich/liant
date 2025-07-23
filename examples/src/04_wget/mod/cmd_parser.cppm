module;
#include "liant/dependency_macro.hpp"

export module wget:cmd_parser;

import std;
import liant;
import :logger;
import :file_reader;

export namespace wget {
struct Help {};
struct Urls {
    std::vector<std::string> urls;
};

class CmdParser {
public:
    CmdParser(liant::ContainerView<ILogger, FileReader> di, int argc, char** argv);

    [[nodiscard]] std::variant<Help, Urls> parse() const;

private:
    liant::ContainerView<ILogger, FileReader> di;

    int argc{};
    char** argv{};
};
LIANT_DEPENDENCY(CmdParser, cmd)
} // namespace wget