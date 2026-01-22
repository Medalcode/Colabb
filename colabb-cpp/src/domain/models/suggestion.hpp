#ifndef COLABB_SUGGESTION_HPP
#define COLABB_SUGGESTION_HPP

#include <string>
#include <chrono>

namespace colabb {
namespace domain {

struct Suggestion {
    std::string command;
    std::string explanation;
    float confidence;
    std::chrono::system_clock::time_point timestamp;
    
    Suggestion()
        : confidence(0.0f)
        , timestamp(std::chrono::system_clock::now()) {}
    
    Suggestion(const std::string& cmd, const std::string& expl = "", float conf = 1.0f)
        : command(cmd)
        , explanation(expl)
        , confidence(conf)
        , timestamp(std::chrono::system_clock::now()) {}
};

} // namespace domain
} // namespace colabb

#endif // COLABB_SUGGESTION_HPP
