#include "infrastructure/http/http_client.hpp"
#include <curl/curl.h>
#include <sstream>

namespace colabb {
namespace infrastructure {

HttpClient::HttpClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
}

HttpClient::~HttpClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
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
    Response response;
    response.status_code = 0;
    
    if (!curl_) {
        return response;
    }
    
    // Set URL
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    
    // Set POST
    curl_easy_setopt(curl_, CURLOPT_POST, 1L);
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
    
    // Set headers
    struct curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }
    if (header_list) {
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, header_list);
    }
    
    // Set write callback
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response.body);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl_);
    
    if (res == CURLE_OK) {
        long status_code;
        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &status_code);
        response.status_code = static_cast<int>(status_code);
    }
    
    // Cleanup
    if (header_list) {
        curl_slist_free_all(header_list);
    }
    
    return response;
}

HttpClient::Response HttpClient::get(const std::string& url,
                                     const std::map<std::string, std::string>& headers) {
    Response response;
    response.status_code = 0;
    
    if (!curl_) {
        return response;
    }
    
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
    
    // Set headers
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
        long status_code;
        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &status_code);
        response.status_code = static_cast<int>(status_code);
    }
    
    if (header_list) {
        curl_slist_free_all(header_list);
    }
    
    return response;
}

} // namespace infrastructure
} // namespace colabb
