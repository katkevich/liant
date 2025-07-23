module wget;

namespace wget {
void ConsoleLogger::logImpl(std::string_view message) const {
    std::cout << message << '\n';
}
} // namespace wget