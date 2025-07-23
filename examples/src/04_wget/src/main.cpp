import std;
import liant;
import wget;

int main(int argc, char** argv) {
    // clang-format off
    auto common = liant::makeContainer(
        liant::registerInstanceOf<wget::ConsoleLogger>().as<wget::ILogger>()
    );

    auto container = liant::makeContainer(
        liant::baseContainer(common),
        liant::registerInstanceOf<wget::App>(),
        liant::registerInstanceOf<wget::CmdParser>().bindArgs(argc, argv),
        liant::registerInstanceOf<wget::HelpPrinter>(),
        liant::registerInstanceOf<wget::FileReader>(),
        liant::registerInstanceOf<wget::FileWriter>(),
        liant::registerInstanceOf<wget::HttpClient>()
    );
    // clang-format on

    return container->resolve<wget::App>()->run();
}