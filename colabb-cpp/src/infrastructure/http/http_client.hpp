#ifndef COLABB_HTTP_CLIENT_HPP
#define COLABB_HTTP_CLIENT_HPP

#include <string>
#include <map>
#include <memory>

typedef void CURL;

namespace colabb {
namespace infrastructure {

class HttpClient {
public:
    struct Response {
        int status_code;
        std::string body;
        std::map<std::string, std::string> headers;
        
        bool is_success() const { return status_code >= 200 && status_code < 300; }
    };
    
    HttpClient();
    ~HttpClient();
    
    Response post(const std::string& url,
                  const std::string& body,
                  const std::map<std::string, std::string>& headers);
    
    Response get(const std::string& url,
                 const std::map<std::string, std::string>& headers = {});
    
private:
    CURL* curl_;
    
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace infrastructure
} // namespace colabb

#endif // COLABB_HTTP_CLIENT_HPP
