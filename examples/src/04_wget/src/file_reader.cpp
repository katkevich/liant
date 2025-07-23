module wget;

namespace wget {
FileReader::FileReader(liant::ContainerView<ILogger> di)
    : di(di) {}

std::vector<std::string> FileReader::readLines(const std::string& path) const {
    std::vector<std::string> urls;

    std::ifstream file(path);
    for (std::string line; std::getline(file, line);) {
        urls.push_back(std::move(line));
    }

    return urls;
}
} // namespace wget