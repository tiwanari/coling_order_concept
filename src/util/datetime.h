#pragma once
#include <ctime>
#include <string>

namespace order_concepts {
namespace util {
inline std::string currentDateTime()
{
    const std::time_t now = std::time(nullptr);
    const std::tm* tstruct = std::localtime(&now);
    char buffer[16];
    // e.g. 20151228-214712 stands for 2015.12.28-21:47:12
    std::strftime(buffer, sizeof(buffer), "%Y%m%d-%H%M%S", tstruct);
    return std::string(buffer);
}

inline std::string currentDate()
{
    const std::time_t now = std::time(nullptr);
    const std::tm* tstruct = std::localtime(&now);
    char buffer[9];
    // e.g. 20151228 stands for 2015.12.28
    std::strftime(buffer, sizeof(buffer), "%Y%m%d", tstruct);
    return std::string(buffer);
}
}   // namespace util
}   // namespace order_concepts
