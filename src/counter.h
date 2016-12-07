#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include "file_format.h"
#include "parser.h"
#include "morph.h"
#include "util/cast.h"
#include "util/concat.h"

namespace order_concepts {
class Counter {
public:
    enum class Stats : unsigned int {
        ADJECTIVE,
        CONCEPT,
        OCCURRENCE,
        NEG_OCCURRENCE,
    };
    enum class Hint : unsigned int {
        CO_OCCURRENCE,
        NEG_CO_OCCURRENCE,
        DEPENDENCY,
        NEG_DEPENDENCY,
        SIMILE,
        NEG_SIMILE,
        COMPARATIVE,
        NEG_COMPARATIVE,
    };
    static bool isStatsVal(const std::string& str) {
        for (int statsVal = util::as_integer(Stats::ADJECTIVE);
                statsVal < util::as_integer(Stats::NEG_OCCURRENCE);
                    ++statsVal)
        {
            Stats val = static_cast<Stats>(statsVal);
            if (str == statsString(val)) {
                return true;
            }
        }
        return false;
    }
    static std::string statsString(const Stats& stat) {
        switch (stat) {
            case Stats::ADJECTIVE:
                return "ADJECTIVE";
            case Stats::CONCEPT:
                return "CONCEPT";
            case Stats::OCCURRENCE:
                return "OCCURRENCE";
            case Stats::NEG_OCCURRENCE:
                return "NEG_OCCURRENCE";
        }
        return "";
    }
    static std::string hintString(const Hint& hint) {
        switch (hint) {
            case Hint::CO_OCCURRENCE:
                return "CO_OCCURRENCE";
            case Hint::NEG_CO_OCCURRENCE:
                return "NEG_CO_OCCURRENCE";
            case Hint::DEPENDENCY:
                return "DEPENDENCY";
            case Hint::NEG_DEPENDENCY:
                return "NEG_DEPENDENCY";
            case Hint::SIMILE:
                return "SIMILE";
            case Hint::NEG_SIMILE:
                return "NEG_SIMILE";
            case Hint::COMPARATIVE:
                return "COMPARATIVE";
            case Hint::NEG_COMPARATIVE:
                return "NEG_COMPARATIVE";
        }
        return "";
    }
    /** output path for reason files **/
    static constexpr const char* REASON_PATH = "out/";
    /** reason files **/
    static constexpr const char* CONCEPT_IDS_FILE = "concepts.txt";
    static constexpr const char* CO_OCCURRENCE_REASON_FILE = "co_occurrences.txt";
    static constexpr const char* DEPENDENCY_REASON_FILE = "dependencies.txt";
    static constexpr const char* SIMILE_REASON_FILE = "similia.txt";
    static constexpr const char* COMPARATIVE_REASON_FILE = "comparatives.txt";
private:
    /** pattern files **/
    static constexpr const char* SIMILE_PATTERN_FILE = "simile.txt";
    static constexpr const char* COMPARATIVE_PATTERN_FILE = "comparative.txt";
    /** pattern file tags **/
    static constexpr const char* PATTERN_CONCEPT_0_TAG = "!CONCEPT0";
    static constexpr const char* PATTERN_CONCEPT_1_TAG = "!CONCEPT1";
    static constexpr const char* PATTERN_ADJECTIVE_TAG = "!ADJECTIVE";
    static constexpr const char* PATTERN_NEG_ADJECTIVE_TAG = "!NEG_ADJECTIVE";
    /** ostream for reason files **/
    std::ofstream m_co_occurrences_output;
    std::ofstream m_dependencies_output;
    std::ofstream m_similia_output;
    std::ofstream m_comparatives_output;
private:
    std::unordered_map<std::string, int> m_concept_to_id;
    std::vector<std::string> m_concepts;
    std::string m_adjective;
    std::string m_antonym;

    long m_total_adjective_occurrences; // count(y_pos)
    long m_total_neg_adjective_occurrences; // count(y_neg)

    long m_total_adjective_dependencies; // dependency(*, y_pos)
    long m_total_neg_adjective_dependencies; // dependency(*, y_neg)

    long m_total_adjective_similia; // simile("", y)
    long m_total_neg_adjective_similia; // simile("", y)

    std::unordered_map<std::string, int> m_occurrences; // occurrence(x)
    std::unordered_map<std::string, int> m_cooccurrences; // co-occurrence(x, y_pos)
    std::unordered_map<std::string, int> m_neg_cooccurrences; // co-occurrence(x, y_neg)
    std::unordered_map<std::string, int> m_dependencies; // dependency(x, y_pos)
    std::unordered_map<std::string, int> m_neg_dependencies; // dependency(x, y_neg)
    std::unordered_map<std::string, int> m_similia; // simile(x, y_pos)
    std::unordered_map<std::string, int> m_neg_similia; // simile(x, y_neg)
    // comparative is a boolean parameter
    // e.g. if "concept0 > concept1" patern exists,
    // comparative[concept0][concept1] also exists
    // this has negative counts as reverse version
    std::unordered_map<
        std::string,
        std::unordered_map<std::string, int>> m_comparatives;

    std::string m_pattern_file_path;
    std::vector<std::vector<std::string>> m_simile_patterns;
    std::vector<std::vector<std::string>> m_comparative_patterns;
private:
    // for making dataset
    bool m_is_prep_mode;
    // // co-occurred adjective
    // std::unordered_map<std::string, long> m_cooccurred_adjective;
    // occurrence(any_y)
    std::unordered_map<std::string, long> m_all_adjective_occurrences;
    // co-occurrence(x, any_y)
    std::unordered_map<
        std::string,
        std::unordered_map<std::string, long>> m_all_adjective_cooccurrences;
private:
    void resetCounters();
    void resetPatterns();
    bool readPatternFile(
        const std::string& pattern_filename,
        std::vector<std::vector<std::string>>* patterns);
    bool readSimilePatterns();
    bool readComarativePatterns();
    bool findConcept(
        const std::set<std::string>& target_concepts,
        const std::vector<std::shared_ptr<Morph>>& morphs,
        int* cur_index,
        std::string* found_concept) const;
    void countCooccurrence(
        const Parser& parser,
        std::set<std::string>* found_concepts);
    void countDependency(
        const Parser& parser,
        const std::set<std::string>& target_concepts);
    void countSimile(
        const Parser& parser,
        const std::set<std::string>& target_concepts);
    void countComparative(
        const Parser& parser,
        const std::set<std::string>& target_concepts);
    void countStats(
        const Parser& parser,
        const std::set<std::string>& target_concepts);
    void writePrepValue(std::ofstream& output_file) const;
    void writeStats(std::ofstream& output_file) const;
    void writeCooccurrenceCounts(std::ofstream& output_file) const;
    void writeDependencyCounts(std::ofstream& output_file) const;
    void writeSimileCounts(std::ofstream& output_file) const;
    void writeComparativeCounts(std::ofstream& output_file) const;
public:
    Counter(
        const std::vector<std::string>& concepts,
        const std::string& adjective,
        const std::string& antonym);
    ~Counter();
    bool isPrepMode() const;
    bool setPrepMode(bool enable);
    bool readPatternFilesInPath(const std::string& path);
    void count(Parser& parser);
    void save(const std::string& output_filename) const;
    const std::vector<std::string>& concepts() const { return m_concepts; }
    const std::string& adjective() const { return m_adjective; }
    int cooccurrence(const std::string& concept) const;
    int cooccurrence(
        const std::string& concept,
        const bool is_negative) const;
    int dependency(const std::string& concept) const;
    int dependency(
        const std::string& concept,
        const bool is_negative) const;
    int simile(const std::string& concept) const;
    int simile(
        const std::string& concept,
        const bool is_negative) const;
    int comparative(
        const std::string& concept0,
        const std::string& concept1) const;
    const std::vector<std::vector<std::string>>&
        similePatterns() const { return m_simile_patterns; }
    const std::vector<std::vector<std::string>>&
        comparativePatterns() const { return m_comparative_patterns; }
};
} // namespace order_concepts
