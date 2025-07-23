module wget;
import :common;

namespace wget {

int App::run() {
    auto visitor = overload{
        [&](Help) { return printHelp(); },
        [&](const Urls& urls) { return download(urls.urls); },
    };
    return std::visit(visitor, di.cmd()->parse());
}

int App::printHelp() const {
    di.help()->print();
    return 0;
}

int App::download(const std::vector<std::string>& urls) const {
    for (const std::string& url : urls) {
        di.logger()->logInfo("fetching {}...", url);

        if (const auto content = di.httpClient()->get(url)) {
            di.fileWriter()->writeString(fileName(url), *content);
            di.logger()->logInfo("done.");
        } else {
            di.logger()->logErr("abort.");
            return 42;
        }
    }
    return 0;
}

std::string App::fileName(const std::string& url) {
    const auto offset = url.rfind('/');
    return url.substr(offset + 1);
}
} // namespace wget