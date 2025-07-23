module;
#include "liant/dependency_macro.hpp"
export module wget:file_writer;

import std;
import liant;
import :logger;

export namespace wget {
class FileWriter {
public:
    explicit FileWriter(liant::ContainerView<ILogger> di);

    void writeString(const std::string& path, const std::string& content) const;

private:
    liant::ContainerView<ILogger> di;
};
LIANT_DEPENDENCY(FileWriter, fileWriter)
} // namespace wget