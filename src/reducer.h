#pragma once
#include <string>
#include <vector>
#include <map>

namespace order_concepts {
class Reducer {
private:
    std::map<std::string, long> m_kvs;
    std::vector<std::string> m_input_filenames;
public:
    Reducer(const std::vector<std::string>& input_filenames)
        : m_input_filenames(input_filenames) {}
    bool reduce(const std::string& output_filename);
    const std::map<std::string, long>& result() { return m_kvs; }
};
} // namespace order_concepts
