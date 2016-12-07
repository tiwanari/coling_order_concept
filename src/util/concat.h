#pragma once
#include <string>
#include <sstream>

namespace order_concepts {
namespace util {
  inline std::string concatStr(const char* str1, const char* str2)
  {
    std::stringstream ss;
    ss << str1 << str2;
    return ss.str();
  }
}
}
