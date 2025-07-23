module;
#include <curl/curl.h>
module wget;

namespace wget {

HttpClient::HttpClient(liant::ContainerView<ILogger> di)
    : di(di) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}
HttpClient::~HttpClient() {
    curl_global_cleanup();
}

std::optional<std::string> HttpClient::get(const std::string& url) const {

    auto curl = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>(curl_easy_init(), curl_easy_cleanup);

    if (!curl) {
        di.logger()->logErr("error: curl_easy_init() failed");
        return {};
    }

    std::string buffer;

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &HttpClient::write);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &buffer);

    const CURLcode res = curl_easy_perform(curl.get());

    if (res != CURLE_OK) {
        di.logger()->logErr("error: curl_easy_perform() failed: {}", curl_easy_strerror(res));
        return {};
    } else {
        return buffer;
    }
}

std::size_t HttpClient::write(void* ptr, std::size_t size, std::size_t nmemb, void* userdata) {
    std::string* buffer = static_cast<std::string*>(userdata);
    std::size_t totalSize = size * nmemb;
    buffer->append(static_cast<char*>(ptr), totalSize);
    return totalSize;
}
} // namespace wget