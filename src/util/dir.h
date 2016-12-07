#pragma once
#include <vector>
#include <string>
#include <sstream>
#if defined _MSC_VER
    #include <direct.h>
#elif defined __GNUC__
    #include <sys/stat.h>
#endif
#include "util/split.h"

namespace order_concepts {
namespace util {
inline bool mkdir(const std::string& directory_name)
{
    std::vector<std::string> directory_names;
    bool is_ok = false;
    #if defined _MSC_VER
        std::string delim = "\\";
        splitStringUsing(directory_name, delim, &directory_names);
        std::stringstream ss;
        for (const auto& directory : directory_names) {
            ss << directory << delim;
            // check the success of creating target directory
            is_ok = ::_mkdir(ss.str().c_str()) == 0;
        }
    #elif defined __GNUC__
        std::string delim = "/";
        splitStringUsing(directory_name, delim, &directory_names);
        std::stringstream ss;
        for (const auto& directory : directory_names) {
            ss << directory << delim;
            // check the success of creating target directory
            is_ok = ::mkdir(ss.str().c_str(), 0755) == 0;
        }
    #endif
    return is_ok;
}

inline std::string directory(const std::string& path)
{
    bool is_found = path.find_last_of("/\\") != std::string::npos;
    return path.substr(0,
        is_found ? path.find_last_of("/\\") - 1 : std::string::npos);
}

inline std::string basename(const std::string& path)
{
    return path.substr(path.find_last_of("/\\") + 1);
}

inline std::string removeExtention(const std::string& filename)
{
    return filename.substr(0, filename.find_last_of('.'));
}
}   // namespace util
}   // namespace order_concepts
