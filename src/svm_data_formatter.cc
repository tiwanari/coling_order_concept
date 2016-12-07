#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cassert>
#include "util/split.h"
#include "util/dir.h"
#include "svm_data_formatter.h"
#include "counter.h"

#define IS_SAME_HINT(h, hint) (h == HINT_STR(hint))
#define HINT_STR(hint) (Counter::hintString(Counter::Hint::hint))
#define STATS_STR(stats) (Counter::statsString(Counter::Stats::stats))
// add 1 smoothing
#define STATS_LOG(stats0, stats1, stats2) \
    (std::log(std::stol( \
        m_raw_data.at(STATS_STR(stats0)) \
                    .at(stats1).at(STATS_STR(stats2)).at("")) + 1))
#define STATS_HINT_LOG(stats0, stats1, hint0) \
    (std::log(std::stol( \
        m_raw_data.at(STATS_STR(stats0)) \
                    .at(stats1).at(HINT_STR(hint0)).at("")) + 1))
#define HINT_LOG(hint0, hint1, hint2) \
    (std::log(std::stol( \
        m_raw_data.at(hint0) \
                    .at(hint1).at(HINT_STR(hint2)).at("")) + 1))

namespace order_concepts {
void SvmDataFormatter::createHintTableToBeShown()
{
    m_hint_ids.clear();
    m_hint_ids.emplace_back(HINT_STR(CO_OCCURRENCE));
    m_hint_ids.emplace_back(HINT_STR(DEPENDENCY));
    m_hint_ids.emplace_back(HINT_STR(SIMILE));
    m_hint_ids.emplace_back(HINT_STR(COMPARATIVE));
    m_hint_ids.emplace_back(HINT_STR(NEG_COMPARATIVE));
}

double SvmDataFormatter::encodeCooccurrenceValue(
    const std::string& adjective,
    const std::string& concept,
    const std::string& hint) const
{
    // x: concept, y: adjective (or negative)
    // e^PMI(x, y) = p(x, y) / (p(x) p(y))
    // = [co-occurrence(x, y) / co-occurrence(*, *)]
    //      / [(count(x) / count(*)) * (count(y) / count(*))]
    // so-score = PMI(x, y_pos) - PMI(x, y_neg)
    // e.g. PMI(タンス, 重いor軽くない) - PMI(タンス, 軽いor重くない)
    // PMI(x, y_pos) - PMI(x, y_neg)
    // = log(co-occurrence(x, y_pos)) - log(count(y_pos))
    //   - [log(co-occurrence(x, y_neg)) - log(count(y_neg))]
    // and add 1 smoothing (each denominator is canceled, so it's easy)
    // -> log(co-occurrence(x, y_pos) + 1) - log(count(y_pos) + 1)
    //   - [log(co-occurrence(x, y_neg) + 1) - log(count(y_neg) + 1)]
    return HINT_LOG(adjective, concept, CO_OCCURRENCE)
            - STATS_LOG(ADJECTIVE, adjective, OCCURRENCE)
        - (HINT_LOG(adjective, concept, NEG_CO_OCCURRENCE)
            - STATS_LOG(ADJECTIVE, adjective, NEG_OCCURRENCE));
}

double SvmDataFormatter::encodeDependencyValue(
    const std::string& adjective,
    const std::string& concept,
    const std::string& hint) const
{
    // x: concept, y: adjective (or negative)
    // p(x, y) / (p(x) p(y))
    // = [dependency(x, y) / dependency(*,*)]
    //      / [(dependency(x, *) / dependency(*, *))
    //              * (dependency(*, y) / dependency(*, *))]
    // PMI(x, y_pos) - PMI(x, y_neg)
    // = log(dependency(x, y_pos)) - log(dependency(*, y_pos))
    //   - [log(dependency(x, y_neg)) - log(dependency(*, y_neg))]
    // -> log(dependency(x, y_pos) + 1) - log(dependency(*, y_pos) + 1)
    //   - [log(dependency(x, y_neg) + 1) - log(dependency(*, y_neg) + 1)]
    return HINT_LOG(adjective, concept, DEPENDENCY)
            - STATS_HINT_LOG(ADJECTIVE, adjective, DEPENDENCY)
        - (HINT_LOG(adjective, concept, NEG_DEPENDENCY)
            - STATS_HINT_LOG(ADJECTIVE, adjective, NEG_DEPENDENCY));
}

double SvmDataFormatter::encodeSimileValue(
    const std::string& adjective,
    const std::string& concept,
    const std::string& hint) const
{
    // x: concept, y: adjective (or negative)
    // p(x, y) / (p(x) p(y))
    // = [simile(x, y) / simile("", "")]
    //      / [(simile(x, "") / simile("", ""))
    //              * (simile("", y) / simile("", ""))]
    // PMK(x, y_pos) - PMI(x, y_neg)
    // = log(simile(x, y_pos)) - log(simile("", y_pos))
    //   - [log(simile(x, y_neg)) - log(simile("", y_neg))]
    // -> log(simile(x, y_pos) + 1) - log(simile("", y_pos) + 1)
    //   - [log(simile(x, y_neg) + 1) - log(simile("", y_neg) + 1)]
    return HINT_LOG(adjective, concept, SIMILE)
            - STATS_HINT_LOG(ADJECTIVE, adjective, SIMILE)
        - (HINT_LOG(adjective, concept, NEG_SIMILE)
            - STATS_HINT_LOG(ADJECTIVE, adjective, NEG_SIMILE));
}

double SvmDataFormatter::encodeComparativeValue(
    const std::string& adjective,
    const std::string& concept,
    const std::string& hint) const
{
    // just divide by the number of concepts
    double num_of_win = 0;
    for (const auto& c : m_concept_ids) {
        if (c == concept) continue;
        int win_count =
            std::stod(
                m_raw_data
                    .at(adjective)
                        .at(concept)
                            .at(hint)
                                .at(c));
        if (win_count > 0) num_of_win++;
    }
    int num_concept = m_concept_ids.size();
    assert(num_concept > 1);
    return num_of_win / (num_concept - 1);
}

bool SvmDataFormatter::encodeValue(
    const std::string& adjective,
    const std::string& concept,
    const std::string& hint,
    double* result) const
{
    *result = 0;
    if (IS_SAME_HINT(hint, CO_OCCURRENCE)) {
        // combine it with NEG_CO_OCCURRENCE
        // so skip NEG_CO_OCCURRENCE
        *result = encodeCooccurrenceValue(adjective, concept, hint);
        return true;
    }
    else if (IS_SAME_HINT(hint, DEPENDENCY)) {
        // combine it with NEG_DEPENDENCY
        // so skip NEG_DEPENDENCY
        *result = encodeDependencyValue(adjective, concept, hint);
        return true;
    }
    else if (IS_SAME_HINT(hint, SIMILE)) {
        // combine it with NEG_SIMILE
        // so skip NEG_SIMILE
        *result = encodeSimileValue(adjective, concept, hint);
        return true;
    }
    else if (IS_SAME_HINT(hint, COMPARATIVE)) {
        *result = encodeComparativeValue(adjective, concept, hint);
        return true;
    }
    else if (IS_SAME_HINT(hint, NEG_COMPARATIVE)) {
        *result = encodeComparativeValue(adjective, concept, hint);
        return true;
    }

    if (std::isnan(*result) || std::isinf(*result)) {
        std::cerr << "nan or inf occurred" << std::endl;
        *result = 0;
        return true;
    }
    std::cerr << "smv_data_formatter.cc error: "
            << "something wrong!!! check this file!!!!" << std::endl;
    assert(false);
    return false;
}

bool SvmDataFormatter::addDataIntoIdTable(
    const std::string& data,
    std::vector<std::string>* id_table)
{
    auto it = std::find(id_table->begin(), id_table->end(), data);
    if (it == id_table->end()) {
        id_table->emplace_back(data);
        return true;
    }
    return false;
}

void SvmDataFormatter::outputIdTable(
    const std::string& output_filename,
    const std::vector<std::string>& id_table) const
{
    std::ofstream output_file;
    output_file.open(output_filename);
    for (int i = 0; i < id_table.size(); ++i) {
        // ID starts with 1
        output_file << (i + 1) << ", " << id_table.at(i) << std::endl;
    }
    output_file.flush();
    output_file.close();
}

void SvmDataFormatter::outputSvmFormatFile(
    const std::string& output_filename) const
{
    std::ofstream output_file;
    output_file.open(output_filename);
    // adjective, concept, hint[,args,...], count
    for (int i = 0; i < m_adjective_ids.size(); ++i) {
        const auto& adjective = m_adjective_ids[i];

        // comment e.g. # query 1 大きい
        output_file << "# query " << (i + 1) << " " << adjective << std::endl;
        for (int j = 0; j < m_concept_ids.size(); ++j) {
            const auto& concept = m_concept_ids[j];

            // comment e.g. # 犬
            output_file << "# " << concept << std::endl;

            // target(priority) query_id feature0:value0 feature1:value1 ...
            // e.g. 3 qid:1 1:1 2:1 3:0 4:0.2 5:0
            // NOTE: target should be added by users
            const std::string dummy_value = "1"; // dummy for prediction
            output_file << dummy_value << " qid:" << (i + 1);
            for (int k = 0; k < m_hint_ids.size(); ++k) {
                const auto& hint = m_hint_ids[k];
                double result;
                if (encodeValue(adjective, concept, hint, &result)) {
                    output_file << " " << (k + 1) << ":" << result;
                    // << m_raw_data.at(adjective).at(concept).at(hint);
                }
            }
            output_file << std::endl;
        }
    }
    output_file.flush();
    output_file.close();
}

void SvmDataFormatter::format(const std::string& output_directory_path)
{
    m_output_directory_path = output_directory_path;

    // read input file
    std::ifstream input_file;
    input_file.open(m_input_filename, std::ios::in);
    if (!input_file) {
        input_file.close();
        throw std::runtime_error("Can't open file: " + m_input_filename + "\n");
    }

    std::string line;
    while (std::getline(input_file, line)) {
        // line = e.g. 大きい,犬,SIMILE,0
        std::vector<std::string> splitted_line;
        util::splitStringUsing(line, ",", &splitted_line);

        // connect hint and its args
        std::stringstream ss;
        for (int i = 3; i < splitted_line.size()-1; i++) {
            if (i != 3) {
                ss << ",";
            }
            ss << splitted_line[i];
        }
        const std::string args = ss.str();

        const std::string& adjective = splitted_line[0];
        const std::string& concept = splitted_line[1];
        const std::string& hint = splitted_line[2];
        const std::string& count = splitted_line[splitted_line.size() - 1];
        // ignore lines for statistic
        if (!Counter::isStatsVal(adjective)) {
            // save adjective/concept ids
            addDataIntoIdTable(adjective, &m_adjective_ids);
            addDataIntoIdTable(concept, &m_concept_ids);
        }

        m_raw_data[adjective][concept][hint][args] = count;
    }
    input_file.close();

    // write to output file
    struct stat buffer;
    // check directory path existence
    if (::stat(output_directory_path.c_str(), &buffer) != 0) {
        util::mkdir(output_directory_path);
    }

    // make mapping files
    outputIdTable(adjectiveMappingFilePath(), m_adjective_ids);
    outputIdTable(conceptMappingFilePath(), m_concept_ids);
    outputIdTable(hintMappingFilePath(), m_hint_ids);
    outputSvmFormatFile(outputFilePath());
}
} // namespace order_concepts
