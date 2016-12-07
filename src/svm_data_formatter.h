#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace order_concepts {
class SvmDataFormatter {
private:
    static constexpr const char* OUTPUT_FILE = "output.txt";
    static constexpr const char* ADJECTIVE_IDS_FILE = "adjective_ids.txt";
    static constexpr const char* CONCEPT_IDS_FILE = "concept_ids.txt";
    static constexpr const char* HINT_IDS_FILE = "hint_ids.txt";
private:
    std::string m_input_filename;
    std::string m_output_directory_path;
    // adjective, concept, hint, (args,...), count
    std::unordered_map<std::string,
        std::unordered_map<std::string,
            std::unordered_map<std::string,
                std::unordered_map<std::string, std::string>>>> m_raw_data;
    std::vector<std::string> m_adjective_ids;
    std::vector<std::string> m_concept_ids;
    // should be create by user
    std::vector<std::string> m_hint_ids;
private:
    void createHintTableToBeShown(); // create hint table
    bool encodeValue(
        const std::string& adjective,
        const std::string& concept,
        const std::string& hint,
        double* result) const;
    bool addDataIntoIdTable(
        const std::string& data,
        std::vector<std::string>* id_table);
    void outputIdTable(
        const std::string& output_filename,
        const std::vector<std::string>& id_table) const;
    void outputSvmFormatFile(
        const std::string& output_filename) const;
    double encodeCooccurrenceValue(
        const std::string& adjective,
        const std::string& concept,
        const std::string& hint) const;
    double encodeDependencyValue(
        const std::string& adjective,
        const std::string& concept,
        const std::string& hint) const;
    double encodeSimileValue(
        const std::string& adjective,
        const std::string& concept,
        const std::string& hint) const;
    double encodeComparativeValue(
        const std::string& adjective,
        const std::string& concept,
        const std::string& hint) const;
public:
    SvmDataFormatter(
        const std::string& input_filename)
        : m_input_filename(input_filename), m_output_directory_path("")
        { createHintTableToBeShown(); }
    void format(const std::string& output_directory_path);
    std::string outputFilePath() const
        { return m_output_directory_path + "/" + OUTPUT_FILE; }
    std::string adjectiveMappingFilePath() const
        { return m_output_directory_path + "/" + ADJECTIVE_IDS_FILE; }
    std::string conceptMappingFilePath() const
        { return m_output_directory_path + "/" + CONCEPT_IDS_FILE; }
    std::string hintMappingFilePath() const
        { return m_output_directory_path + "/" + HINT_IDS_FILE; }
};
} // namespace order_concepts
