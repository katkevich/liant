module;
#include "liant/dependency_macro.hpp"
export module wget:file_reader;

import std;
import liant;
import :logger;

export namespace wget {
class FileReader {
public:
    explicit FileReader(liant::ContainerView<ILogger> di);

    [[nodiscard]] std::vector<std::string> readLines(const std::string& path) const;

private:
    liant::ContainerView<ILogger> di;
};
LIANT_DEPENDENCY(FileReader, fileReader)
} // namespace wget