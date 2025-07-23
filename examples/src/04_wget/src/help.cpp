module wget;

namespace wget {
void HelpPrinter::print() const {
    std::println("Usage: wget [URL]");
    std::println("Flags:");
    std::println("-h,  --help                      print this help");
    std::println("-f,  --file                      path to the file which contains new line separated list of URLs");
}
} // namespace wget