export module wget:app;

import liant;
import :cmd_parser;
import :logger;
import :http_client;
import :file_writer;
import :help;

export namespace wget {
class App {
public:
    int run();

private:
    int printHelp() const;
    int download(const std::vector<std::string>& urls) const;

    [[nodiscard]] static std::string fileName(const std::string& url);

public:
    liant::ContainerView<ILogger, HttpClient, CmdParser, FileWriter, HelpPrinter> di;
};
} // namespace wget