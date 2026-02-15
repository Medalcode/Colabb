#include "infrastructure/http/http_client.hpp"
#include <curl/curl.h>
#include <sstream>
#include <thread>
#include <chrono>

namespace colabb {
namespace infrastructure {

std::once_flag HttpClient::curl_init_flag_;

HttpClient::HttpClient() {
    std::call_once(curl_init_flag_, []() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    });
    curl_ = curl_easy_init();
}

HttpClient::~HttpClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
}

size_t HttpClient::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), total_size);
    return total_size;
}

HttpClient::Response HttpClient::post(const std::string& url,
                                      const std::string& body,
                                      const std::map<std::string, std::string>& headers) {
    constexpr int kMaxAttempts = 3;
    constexpr long kConnectTimeoutSec = 3L;
    constexpr long kRequestTimeoutSec = 12L;

    Response last_response{0, "", {}};
    if (!curl_) {
        return last_response;
    }

    for (int attempt = 0; attempt < kMaxAttempts; ++attempt) {
        Response response{0, "", {}};
        curl_easy_reset(curl_);

        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, kConnectTimeoutSec);
        curl_easy_setopt(curl_, CURLOPT_TIMEOUT, kRequestTimeoutSec);
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));

        struct curl_slist* header_list = nullptr;
        for (const auto& [key, value] : headers) {
            std::string header = key + ": " + value;
            header_list = curl_slist_append(header_list, header.c_str());
        }
        if (header_list) {
            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, header_list);
        }

        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response.body);

        CURLcode res = curl_easy_perform(curl_);
        if (res == CURLE_OK) {
            long status_code = 0;
            curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &status_code);
            response.status_code = static_cast<int>(status_code);
        }

        if (header_list) {
            curl_slist_free_all(header_list);
        }

        last_response = response;

        const bool transient_curl_error =
            res == CURLE_OPERATION_TIMEDOUT || res == CURLE_COULDNT_CONNECT || res == CURLE_RECV_ERROR;
        const bool retryable_http = response.status_code == 429 || (response.status_code >= 500 && response.status_code < 600);
        const bool should_retry = attempt + 1 < kMaxAttempts && (transient_curl_error || retryable_http || response.status_code == 0);
        if (!should_retry) {
            return response;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100 * (1 << attempt)));
    }

    return last_response;
}

HttpClient::Response HttpClient::get(const std::string& url,
                                     const std::map<std::string, std::string>& headers) {
    constexpr int kMaxAttempts = 3;
    constexpr long kConnectTimeoutSec = 3L;
    constexpr long kRequestTimeoutSec = 10L;

    Response last_response{0, "", {}};
    if (!curl_) {
        return last_response;
    }

    for (int attempt = 0; attempt < kMaxAttempts; ++attempt) {
        Response response{0, "", {}};
        curl_easy_reset(curl_);
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, kConnectTimeoutSec);
        curl_easy_setopt(curl_, CURLOPT_TIMEOUT, kRequestTimeoutSec);
        curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);

        struct curl_slist* header_list = nullptr;
        for (const auto& [key, value] : headers) {
            std::string header = key + ": " + value;
            header_list = curl_slist_append(header_list, header.c_str());
        }
        if (header_list) {
            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, header_list);
        }

        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response.body);

        CURLcode res = curl_easy_perform(curl_);
        if (res == CURLE_OK) {
            long status_code = 0;
            curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &status_code);
            response.status_code = static_cast<int>(status_code);
        }

        if (header_list) {
            curl_slist_free_all(header_list);
        }

        last_response = response;

        const bool transient_curl_error =
            res == CURLE_OPERATION_TIMEDOUT || res == CURLE_COULDNT_CONNECT || res == CURLE_RECV_ERROR;
        const bool retryable_http = response.status_code == 429 || (response.status_code >= 500 && response.status_code < 600);
        const bool should_retry = attempt + 1 < kMaxAttempts && (transient_curl_error || retryable_http || response.status_code == 0);
        if (!should_retry) {
            return response;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100 * (1 << attempt)));
    }

    return last_response;
}

} // namespace infrastructure
} // namespace colabb
