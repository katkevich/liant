module wget;

namespace wget {
using namespace std::string_view_literals;

CmdParser::CmdParser(liant::ContainerView<ILogger, FileReader> di, int argc, char** argv)
    : di(di)
    , argc(argc)
    , argv(argv) {}

std::variant<Help, Urls> CmdParser::parse() const {

    if (argc == 2 && (argv[1] == "-h"sv || argv[1] == "--help"sv)) {
        return Help{};
    } else if (argc == 3 && (argv[1] == "-f"sv || argv[1] == "--file"sv)) {
        return Urls{ di.fileReader()->readLines(argv[2]) };
    } else if (argc == 2) {
        return Urls{ { argv[1] } };
    } else {
        di.logger()->logErr("invalid input");
        return Help{};
    }
}
} // namespace wget