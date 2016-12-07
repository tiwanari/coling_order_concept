#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include "reducer.h"

namespace order_concepts {
bool Reducer::reduce(const std::string& output_filename)
{
    bool doAllFilesExist = true;

    // count hints from all data
    for (const auto& filename : m_input_filenames) {
        struct stat buffer;
        // check path existence
        if (::stat(filename.c_str(), &buffer) != 0) {
            doAllFilesExist = false;
            continue;
        }

        std::ifstream input_file;
        input_file.open(filename);

        std::string line;
        while (std::getline(input_file, line)) {
            // line = e.g. 大きい,犬,SIMILE,0
            auto index = line.find_last_of(",");
            if (index == std::string::npos) {
                // NOTE: the format of this file is something wrong
                break;
            }
            // split string into a pair of key and value
            const std::string& key = line.substr(0, index);
            const std::string& value = line.substr(index+1);
            m_kvs[key] += std::stol(value);
        }
        input_file.close();
    }

    std::ofstream output_file;
    output_file.open(output_filename);
    for (const auto& pair : m_kvs) {
        output_file
            << pair.first << ","
            << pair.second << std::endl;
    }
    output_file.close();

    return doAllFilesExist;
}
} // namespace order_concepts
