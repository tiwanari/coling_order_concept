#pragma once
#include <string>
#include <sstream>
#include <iterator>
#include <vector>

namespace order_concepts {
namespace util {
inline bool splitStringUsing(
    const std::string& str,
    const std::string& delim,
    std::vector<std::string>* output)
{
    if (str.empty()) return false;
    size_t cp, fp; // current position, found position
    const size_t dsize = delim.size();

    for (cp = 0;
         (fp = str.find(delim, cp)) != std::string::npos;
         cp = fp + dsize)
        output->emplace_back(str, cp, fp - cp);

    output->emplace_back(str, cp, str.size() - cp);

    if (output->size() <= 1) return false;
    else return true;
}

inline bool splitStringWithWhitespaces(
    const std::string& str,
    std::vector<std::string>* output)
{
    if (str.empty()) return false;
    std::istringstream iss(str);
    std::copy(std::istream_iterator<std::string>(iss),
        std::istream_iterator<std::string>(),
        std::back_inserter(*output));

    if (output->size() <= 1) return false;
    else return true;
}
}   // namespace util
}   // namespace order_concepts
