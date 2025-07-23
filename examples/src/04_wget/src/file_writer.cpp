module wget;

namespace wget {
FileWriter::FileWriter(liant::ContainerView<ILogger> di)
    : di(di) {}

void FileWriter::writeString(const std::string& path, const std::string& content) const {
    std::ofstream file(path);

    if (file.is_open()) {
        file << content;
    } else {
        di.logger()->logErr("unable to open file {}", path);
    }
}
} // namespace wget