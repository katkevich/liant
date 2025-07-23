module;
#include "liant/dependency_macro.hpp"
export module wget:http_client;

import std;
import liant;
import :logger;

export namespace wget {
class HttpClient {
public:
    explicit HttpClient(liant::ContainerView<ILogger> di);
    ~HttpClient();

    [[nodiscard]] std::optional<std::string> get(const std::string& url) const;

private:
    static std::size_t write(void* ptr, std::size_t size, std::size_t nmemb, void* userdata);

private:
    liant::ContainerView<ILogger> di;
};

LIANT_DEPENDENCY(HttpClient, httpClient)
} // namespace wget