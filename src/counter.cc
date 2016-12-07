#include <sys/stat.h>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cassert>
#include "counter.h"
#include "util/split.h"
#include "util/dir.h"

namespace order_concepts {
void Counter::resetCounters()
{
    // variables for statistic
    m_total_adjective_occurrences = 0;
    m_total_neg_adjective_occurrences = 0; // neg
    m_total_adjective_dependencies = 0;
    m_total_neg_adjective_dependencies = 0; // neg
    m_total_adjective_similia = 0;
    m_total_neg_adjective_similia = 0; // neg

    for (const auto& concept0 : m_concepts) {
        m_occurrences[concept0] = 0;

        m_cooccurrences[concept0] = 0;
        m_neg_cooccurrences[concept0] = 0; // neg

        m_dependencies[concept0] = 0;
        m_neg_dependencies[concept0] = 0; // neg

        m_similia[concept0] = 0;
        m_neg_similia[concept0] = 0; // neg

        for (const auto& concept1 : m_concepts) {
            if (concept0 != concept1) {
                m_comparatives[concept0][concept1] = 0;
            }
        }
        // stats
        m_all_adjective_cooccurrences[concept0];
    }
}

void Counter::resetPatterns()
{
    std::vector<std::vector<std::string>>()
        .swap(m_simile_patterns);
    std::vector<std::vector<std::string>>()
        .swap(m_comparative_patterns);
}

Counter::Counter(
    const std::vector<std::string>& concepts,
    const std::string& adjective,
    const std::string& antonym)
: m_concepts(concepts), m_adjective(adjective), m_antonym(antonym), m_is_prep_mode(false)
{
    // make the directory for output
    util::mkdir(REASON_PATH);
    m_co_occurrences_output.open(util::concatStr(REASON_PATH, CO_OCCURRENCE_REASON_FILE), std::ios_base::out);
    m_dependencies_output.open(util::concatStr(REASON_PATH, DEPENDENCY_REASON_FILE), std::ios_base::out);
    m_similia_output.open(util::concatStr(REASON_PATH, SIMILE_REASON_FILE), std::ios_base::out);
    m_comparatives_output.open(util::concatStr(REASON_PATH, COMPARATIVE_REASON_FILE), std::ios_base::out);

    // write concepts
    std::ofstream concepts_file(util::concatStr(REASON_PATH, CONCEPT_IDS_FILE));
    for (int i = 0; i < m_concepts.size(); i++) {
        concepts_file << i << "," << m_concepts[i] << std::endl;
        m_concept_to_id[m_concepts[i]] = i;
    }
    concepts_file.close();
}

Counter::~Counter()
{
    m_co_occurrences_output.close();
    m_dependencies_output.close();
    m_similia_output.close();
    m_comparatives_output.close();
}

bool Counter::isPrepMode() const
{
    return m_is_prep_mode;
}

bool Counter::setPrepMode(bool enable)
{
    bool old_val = m_is_prep_mode;
    m_is_prep_mode = enable;
    return old_val != enable;
}

bool Counter::readPatternFile(
    const std::string& pattern_filename,
    std::vector<std::vector<std::string>>* patterns)
{
    struct stat buffer;
    // check path existence
    if (::stat(pattern_filename.c_str(), &buffer) != 0) {
        return false;
    }

    std::ifstream pattern_file;
    pattern_file.open(pattern_filename);

    std::string line;
    while (std::getline(pattern_file, line)) {
        std::vector<std::string> splitted_line;
        // skip blank lines
        if (line.empty()) {
            continue;
        }
        util::splitStringWithWhitespaces(line, &splitted_line);
        patterns->emplace_back(splitted_line);
    }
    pattern_file.close();
    return true;
}

bool Counter::readSimilePatterns()
{
    const std::string simile_file_path =
        m_pattern_file_path + "/" + SIMILE_PATTERN_FILE;
    return readPatternFile(simile_file_path, &m_simile_patterns);
}

bool Counter::readComarativePatterns()
{
    const std::string comparative_file_path =
        m_pattern_file_path + "/" + COMPARATIVE_PATTERN_FILE;
    return readPatternFile(comparative_file_path, &m_comparative_patterns);
}

bool Counter::readPatternFilesInPath(const std::string& path)
{
    resetPatterns();
    m_pattern_file_path = path;
    struct stat buffer;
    // check path existence
    if (::stat(path.c_str(), &buffer) != 0) {
        return false;
    }
    m_pattern_file_path = path;
    readSimilePatterns();
    readComarativePatterns();
    return true;
}

void Counter::count(Parser& parser)
{
    resetCounters();
    int num_lines = 0;
    try {
        while (parser.next()) {
            num_lines++;
            const std::vector<std::shared_ptr<Morph>> morphs = parser.morphs();

            std::set<std::string> found_concepts;
            countCooccurrence(parser, &found_concepts);
            // // check when they co-occurred
            // // we need check some parameter for statistic
            // if (found_concepts.size() > 0) {
            // check all variables for post calculation
            if (!isPrepMode()) {
                countDependency(parser, found_concepts);
                countSimile(parser, found_concepts);
                countComparative(parser, found_concepts);
            }
            else {
                countStats(parser, found_concepts);
            }
            // }
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "counting error!" << std::endl << std::flush;
        throw e;
    }
    std::cerr << "read " << num_lines << " lines!" << std::endl << std::flush;
}

// **************************** DO CHECK!!!! **********************************
// TODO: this check may be OK, but I'm not sure
// concepts {東京, 東京大学} is a corner case for this implementation
// -> match 東京 and skip 東京大学
bool Counter::findConcept(
    const std::set<std::string>& target_concepts,
    const std::vector<std::shared_ptr<Morph>>& morphs,
    int *cur_index,
    std::string* found_concept) const
{
    // check the POS of lemma
    if (morphs[*cur_index]->POS() != Morph::POS_TAG::NOUN) {
        return false;
    }

    // check morph not lemma
    // because some nouns are unknown and don't have lemma
    std::string concatenated_morphs = "";
    for (int i = *cur_index; i < morphs.size(); ++i) {
        const auto& morph = morphs[i];
        if (morph->POS() != Morph::POS_TAG::NOUN) {
            break;
        }

        // concatenate morphs
        concatenated_morphs.append(morph->morph());
        // check it is a concept
        if (target_concepts.find(concatenated_morphs) != target_concepts.end()) {
            // set found concept and index
            *found_concept = concatenated_morphs;
            *cur_index = i;
            return true;
        }
    }
    return false;
}

void Counter::countCooccurrence(
    const Parser& parser,
    std::set<std::string>* found_concepts)
{
    bool adj_flag = false;
    bool neg_adj_flag = false;
    const std::vector<Phrase>& phrases = parser.phrases();
    for (int i = 0; i < phrases.size(); ++i) {
        const auto& phrase = phrases[i];
        // search concept
        for (const auto& concept : m_concepts) {
            if (phrase.find(concept, Morph::POS_TAG::NOUN)) {
                found_concepts->emplace(concept); // check occurrence of concept
                m_occurrences[concept]++;
            }
        }
        // search adjective/antonym
        bool did_found_adjective =
            phrase.find(m_adjective, Morph::POS_TAG::ADJECTIVE);
        bool did_found_antonym =
            phrase.find(m_antonym, Morph::POS_TAG::ADJECTIVE);
        if (did_found_adjective || did_found_antonym) {
            // check dependency count
            // if this is not the first phrase,
            // this is depended on by leading phrases
            int dependency_count = 0;
            for (int j = 0; j < i; ++j) {
                const auto& leading_phrase = phrases[j];
                if (leading_phrase.depend() == i) {
                    dependency_count++;
                }
            }
            // not negative
            if ((did_found_adjective && !phrase.isNegative())
                || (did_found_antonym && phrase.isNegative())) {
                // count adjective's occurrence
                m_total_adjective_occurrences++;
                adj_flag = true;
                m_total_adjective_dependencies += dependency_count;
            }
            else {
                // count negative adjective's occurrence
                m_total_neg_adjective_occurrences++;
                neg_adj_flag = true;
                m_total_neg_adjective_dependencies += dependency_count;
            }
        }
    }
    if (adj_flag) {
        for (const auto& concept : *found_concepts) {
            m_cooccurrences[concept]++;
            m_co_occurrences_output << m_concept_to_id[concept] << "\t + " << parser.raw() << std::endl;
        }
    }
    if (neg_adj_flag) {
        for (const auto& concept : *found_concepts) {
            m_neg_cooccurrences[concept]++;
            m_co_occurrences_output << m_concept_to_id[concept] << "\t - " << parser.raw() << std::endl;
        }
    }
}

void Counter::countDependency(
    const Parser& parser,
    const std::set<std::string>& target_concepts)
{
    const std::vector<Phrase>& phrases = parser.phrases();
    for (int i = 0; i < phrases.size(); ++i) {
        const Phrase& phrase = phrases[i];
        // search concept
        for (const auto& concept : target_concepts) {
            // concept is not in the phrase / it has no dependency
            if (!phrase.find(concept, Morph::POS_TAG::NOUN)
                                    || phrase.depend() < 0) {
                continue;
            }
            // check dependency
            const Phrase& target_phrase = phrases[phrase.depend()];
            bool did_found_adjective =
                target_phrase.find(m_adjective, Morph::POS_TAG::ADJECTIVE);
            bool did_found_antonym =
                target_phrase.find(m_antonym, Morph::POS_TAG::ADJECTIVE);
            if (did_found_adjective || did_found_antonym) {
                // not negative
                if ((did_found_adjective && !target_phrase.isNegative())
                        || (did_found_antonym && target_phrase.isNegative())) {
                    m_dependencies[concept]++;
                    m_dependencies_output <<  m_concept_to_id[concept] << "\t + " << parser.raw() << std::endl;
                }
                else {
                    m_neg_dependencies[concept]++;
                    m_dependencies_output <<  m_concept_to_id[concept] << "\t - " << parser.raw() << std::endl;
                }
            }
        }
    }
}

#define MORPH_CHECK(morph, p, l) \
    (morph->POS() == Morph::POS_TAG::p \
    && (morph->lemma() == l))

void Counter::countSimile(
    const Parser& parser,
    const std::set<std::string>& target_concepts)
{
    const std::vector<std::shared_ptr<Morph>>& morphs = parser.morphs();
    for (const auto& pattern : m_simile_patterns) {
        // if the pattern is longer than morphs, skip it
        if (pattern.size() > morphs.size()) continue;

        // the following may be a better way (but takes time)
        // this uses lemma in patterns
        // e.g. !CONCEPT0 のような !ADJECTIVE
        // -> !CONCEPT0 だ ようだ !ADJECTIVE
        // !!!NOTE!!!:  this version assumes
        //              a pattern starts with concept and ends with adj
        std::string found_concept;
        int pattern_index = 0;
        for (int i = 0; i < morphs.size(); ++i) {
            const auto& morph = morphs[i];
            const auto& p = pattern[pattern_index++];
            bool does_match = false;

            if (p == Counter::PATTERN_CONCEPT_0_TAG) {
                // check concept
                does_match =
                    findConcept(target_concepts,
                                morphs,
                                &i,
                                &found_concept);
            }
            else if (p == Counter::PATTERN_ADJECTIVE_TAG) {
                // check adjective/antonym
                // simile does not take account of negation
                bool did_found_adjective =
                    MORPH_CHECK(morph, ADJECTIVE, m_adjective);
                bool did_found_antonym =
                    MORPH_CHECK(morph, ADJECTIVE, m_antonym);

                // count up a counter
                // NOTE: this assumes the last pattern element should be an adjective
                if (did_found_adjective) {
                    m_similia[found_concept]++;
                    m_similia_output << m_concept_to_id[found_concept]
                        << "\t + " << parser.raw() << std::endl;
                }
                if (did_found_antonym) {
                    m_neg_similia[found_concept]++;
                    m_similia_output << m_concept_to_id[found_concept]
                        << "\t - " << parser.raw() << std::endl;
                }
                does_match =
                    did_found_adjective || did_found_antonym;
            }
            else {
                // simply, check with patten's lemma
                does_match = (morph->lemma() == p);
            }

            if (!does_match || pattern_index == pattern.size()) {
                if (!does_match && pattern_index != 1) {
                    // check this morph again with the first element
                    --i;
                }
                pattern_index = 0;

                // check pattern.size() is larger than morphs.size() - (i + 1)
                // that's why this includes equality check
                if (pattern.size() >= morphs.size() - i) break;
            }
        }
    }
    // count for all patterns ending with adj
    for (const auto& pattern : m_simile_patterns) {
        // if the pattern is longer than morphs, skip it
        if (pattern.size() > morphs.size()) continue;

        int pattern_index = 1;  // skip concept (start)
        for (int i = 0; i < morphs.size(); ++i) {
            const auto& morph = morphs[i];
            const auto& p = pattern[pattern_index++];
            bool does_match = false;

            if (p == Counter::PATTERN_ADJECTIVE_TAG) {
                bool did_found_adjective =
                    MORPH_CHECK(morph, ADJECTIVE, m_adjective);
                bool did_found_antonym =
                    MORPH_CHECK(morph, ADJECTIVE, m_antonym);

                if (did_found_adjective) m_total_adjective_similia++;
                if (did_found_antonym) m_total_neg_adjective_similia++;
                does_match =
                    did_found_adjective || did_found_antonym;
            }
            else {
                // simply, check with patten's lemma
                does_match = (morph->lemma() == p);
            }

            if (!does_match || pattern_index == pattern.size()) {
                if (!does_match && pattern_index != 2) {
                    // check this morph again with the first element
                    // here, the first element is pattern[1]
                    --i;
                }
                pattern_index = 1;

                // check pattern.size() - 1 (skip concept)
                // is larger than morphs.size() - (i + 1)
                // that's why this DOES NOT include equality check
                if (pattern.size() > morphs.size() - i) break;
            }
        }
    }
}

bool canReachToIndex(const std::vector<int>& dependencies, int start, int target)
{
    return dependencies[start] == target;
}

bool canReachToIndexFromConceptWithoutTag(
    const std::vector<Phrase>& phrases,
    const std::vector<int>& dependencies, int start, int target)
{
    const std::string hiragana = "ほうが";
    const std::string kanji = "方が";
    return dependencies[start] != -1
        && (dependencies[start] == target
            || (dependencies[dependencies[start]] == target
                && (phrases[dependencies[start]].phrase().substr(0, hiragana.size()) == hiragana
                    || phrases[dependencies[start]].phrase().substr(0, kanji.size()) == kanji)));
}

void Counter::countComparative(
    const Parser& parser,
    const std::set<std::string>& target_concepts)
{
    if (target_concepts.size() <= 1) {
        return ;
    }

    const std::vector<Phrase>& phrases = parser.phrases();

    // NOTE: this function assumes each concept / adjective / antonym appears at most once

    // concept -> (found_position, TAG [PATTERN_ADJECTIVE_TAG / PATTERN_NEG_ADJECTIVE_TAG])
    const std::string NO_TAG = "";
    std::unordered_map<std::string, std::pair<int, std::string>> concept_w_tag_positions;
    std::unordered_map<std::string, int> concept_wo_tag_positions;
    const int NOT_FOUND = -2;
    // adj/ant: (found_position, is_negative)
    std::pair<int, bool> adjective_position = {NOT_FOUND, false};
    std::pair<int, bool> antonym_position = {NOT_FOUND, false};

    for (int i = 0; i < phrases.size(); ++i) {
        const auto& phrase = phrases[i];
        // search concept
        for (const auto& concept : target_concepts) {
            if (!phrase.find(concept, Morph::POS_TAG::NOUN)) continue; // concept not found

            // check the concept has a comparative pattern
            bool does_find_pattern = false;
            for (const auto& pattern : m_comparative_patterns) {
                const std::string& lemma = pattern[0]; // e.g. ほど, より
                // PATTERN_ADJECTIVE_TAG or PATTERN_NEG_ADJECTIVE_TAG
                const std::string& tag = pattern[1];

                // search a pattern in the phrase
                for (const auto& morph : phrase.morphs()) {
                    if (morph->lemma() == lemma) {
                        does_find_pattern = true;
                        break;
                    }
                }
                if (does_find_pattern) {
                    concept_w_tag_positions[concept] = std::make_pair(i, tag); // add position with the tag
                    break;
                }
            }
            if (!does_find_pattern) concept_wo_tag_positions[concept] = i; // just add the position
        }
        // search adjective / antonym
        if (phrase.find(m_adjective, Morph::POS_TAG::ADJECTIVE))
            adjective_position = std::make_pair(i, phrase.isNegative());
        if (phrase.find(m_antonym, Morph::POS_TAG::ADJECTIVE))
            antonym_position = std::make_pair(i, phrase.isNegative());
    }

    // here, we know the position of concepts / adjective / antonym

    // check dependencies
    const std::vector<int>& dependencies = parser.connections();
    for (const auto& concept0 : concept_w_tag_positions) {
        // concept0's info
        const std::string& name0 = concept0.first;
        const int& pos0 = concept0.second.first;
        const std::string& tag0 = concept0.second.second;

        for (const auto& concept1 : concept_wo_tag_positions) {
            // concept1's info
            const std::string& name1 = concept1.first;
            const int& pos1 = concept1.second;

            // on adjective
            if (canReachToIndex(dependencies, pos0, adjective_position.first)
                    && canReachToIndexFromConceptWithoutTag(phrases, dependencies, pos1, adjective_position.first)) {
                // if adjective has negation, simply a result does not depend on the tag
                if (adjective_position.second) {
                    // e.g. c0 より c1 は neg_adj / c0 ほど c1 は neg_adj
                    m_comparatives[name0][name1]++;
                    m_comparatives_output << m_concept_to_id[name0] << "\t + "
                                            << parser.raw() << std::endl;
                    m_comparatives_output << m_concept_to_id[name1] << "\t - "
                                            << parser.raw() << std::endl;
                }
                else {
                    // if adjective has no negation, check the tag
                    if (tag0 != Counter::PATTERN_NEG_ADJECTIVE_TAG) {
                        m_comparatives[name1][name0]++; // e.g. c0 より c1 は adj
                        m_comparatives_output << m_concept_to_id[name1] << "\t + "
                                                << parser.raw() << std::endl;
                        m_comparatives_output << m_concept_to_id[name0] << "\t - "
                                                << parser.raw() << std::endl;
                    }
                }
            }
            // on antonym
            if (canReachToIndex(dependencies, pos0, antonym_position.first)
                    && canReachToIndexFromConceptWithoutTag(phrases, dependencies, pos1, antonym_position.first)) {
                // if antonym has negation, simply the result does not depend on the tag
                if (antonym_position.second) {
                    // e.g. c0 より c1 は neg_ant / c0 ほど c1 は neg_ant
                    m_comparatives[name1][name0]++;
                    m_comparatives_output << m_concept_to_id[name0] << "\t + "
                                            << parser.raw() << std::endl;
                    m_comparatives_output << m_concept_to_id[name1] << "\t - "
                                                << parser.raw() << std::endl;
                }
                else {
                    // if antonym has no negation, check the tag
                    if (tag0 != Counter::PATTERN_NEG_ADJECTIVE_TAG) {
                        m_comparatives[name0][name1]++; // e.g. c0 より c1 は ant
                        m_comparatives_output << m_concept_to_id[name0] << "\t + "
                                                << parser.raw() << std::endl;
                        m_comparatives_output << m_concept_to_id[name1] << "\t - "
                                                << parser.raw() << std::endl;
                    }
                }
            }
        }
    }
}

void Counter::countStats(
    const Parser& parser,
    const std::set<std::string>& target_concepts)
{
    const std::vector<std::shared_ptr<Morph>>& morphs = parser.morphs();
    for (int i = 0; i < morphs.size(); ++i) {
        const auto& morph = morphs[i];
        if (morph->POS() == Morph::POS_TAG::ADJECTIVE) {
            m_all_adjective_occurrences[morph->lemma()]++;
            for (const auto& concept : target_concepts) {
                m_all_adjective_cooccurrences[concept][morph->lemma()]++;
            }
        }
    }
}

void Counter::save(const std::string& output_filename) const
{
    std::ofstream output_file;
    output_file.open(output_filename);

    // insert
    // write variables for statistic
    // following hints look like:
    // adjective,concept,Hint(see. counter.h)[,arg0][,arg1],...,count
    if (isPrepMode()) {
        writePrepValue(output_file);
    }
    else {
        std::cerr << "writing data to " << output_filename << "..." << std::endl << std::flush;
        writeStats(output_file);
        writeCooccurrenceCounts(output_file);
        writeDependencyCounts(output_file);
        writeSimileCounts(output_file);
        writeComparativeCounts(output_file);
        std::cerr << "write data to " << output_filename << " done." << std::endl << std::flush;
    }
    output_file.flush();
    output_file.close();
}

#define WRITE_TO_FILE_STAT(adj, stat, val) \
    output_file                                 \
        << statsString(Stats::ADJECTIVE) << "," \
        << adj << ","                   \
        << statsString(Stats::stat) << ","      \
        << val << std::endl << std::flush;
#define WRITE_TO_FILE_HINT(hint, val) \
    output_file                                 \
        << statsString(Stats::ADJECTIVE) << "," \
        << m_adjective << ","                   \
        << hintString(Hint::hint) << ","        \
        << val << std::endl << std::flush;

void Counter::writePrepValue(std::ofstream& output_file) const
{
    for (const auto& concept : m_concepts) {
        // concept occurrence
        output_file
            << statsString(Stats::CONCEPT) << ","
            << concept << ","
            << statsString(Stats::OCCURRENCE) << ","
            << m_occurrences.at(concept) << std::endl << std::flush;
        // concept-adjective co-occurrences
        for (const auto& pair : m_all_adjective_cooccurrences.at(concept)) {
            // pair = (adjective, count)
            output_file
                << statsString(Stats::CONCEPT) << ","
                << concept << ","
                << pair.first << ","
                << hintString(Hint::CO_OCCURRENCE) << ","
                << pair.second << std::endl << std::flush;
        }
    }
    for (const auto& pair : m_all_adjective_occurrences) {
        // pair = (adjective, count)
        WRITE_TO_FILE_STAT(pair.first, OCCURRENCE, pair.second);
    }
}

void Counter::writeStats(std::ofstream& output_file) const
{
    // concept occurrences
    for (const auto& concept : m_concepts) {
        output_file
            << statsString(Stats::CONCEPT) << ","
            << concept << ","
            << statsString(Stats::OCCURRENCE) << ","
            << m_occurrences.at(concept) << std::endl << std::flush;
    }

    // adjective occurrences
    WRITE_TO_FILE_STAT(m_adjective, OCCURRENCE, m_total_adjective_occurrences);
    WRITE_TO_FILE_STAT(m_adjective, NEG_OCCURRENCE,
        m_total_neg_adjective_occurrences);

    // adjective dependencies
    WRITE_TO_FILE_HINT(DEPENDENCY, m_total_adjective_dependencies);
    WRITE_TO_FILE_HINT(NEG_DEPENDENCY, m_total_neg_adjective_dependencies);

    // adjective similia
    WRITE_TO_FILE_HINT(SIMILE, m_total_adjective_similia);
    WRITE_TO_FILE_HINT(NEG_SIMILE, m_total_neg_adjective_similia);
}

#define WRITE_HINT_TO_FILE(counts, hint) \
    for (const auto& pair : counts) {    \
        output_file                         \
            << m_adjective << ","           \
            << pair.first << ","            \
            << hintString(Hint::hint) << ","\
            << pair.second << std::endl << std::flush;    \
    }

void Counter::writeCooccurrenceCounts(std::ofstream& output_file) const
{
    WRITE_HINT_TO_FILE(m_cooccurrences, CO_OCCURRENCE);
    WRITE_HINT_TO_FILE(m_neg_cooccurrences, NEG_CO_OCCURRENCE);
}

void Counter::writeDependencyCounts(std::ofstream& output_file) const
{
    WRITE_HINT_TO_FILE(m_dependencies, DEPENDENCY);
    WRITE_HINT_TO_FILE(m_neg_dependencies, NEG_DEPENDENCY);
}

void Counter::writeSimileCounts(std::ofstream& output_file) const
{
    WRITE_HINT_TO_FILE(m_similia, SIMILE);
    WRITE_HINT_TO_FILE(m_neg_similia, NEG_SIMILE);
}

void Counter::writeComparativeCounts(std::ofstream& output_file) const
{
    // count positive/negative count
    for (const auto& dict : m_comparatives) {
        for (const auto& pair : dict.second) {
            // output positive comparative
            output_file
                << m_adjective << ","
                << dict.first << ","
                << hintString(Hint::COMPARATIVE) << ","
                << pair.first << ","
                << pair.second << std::endl << std::flush;
            // output negative comparatives
            output_file
                << m_adjective << ","
                << pair.first << ","
                << hintString(Hint::NEG_COMPARATIVE) << ","
                << dict.first << ","
                << pair.second << std::endl << std::flush;
        }
    }
}

#define COUNT_AT(target, concept) \
    auto it = target.find(concept); \
    if (it == target.end()) {       \
        return 0;                   \
    }                               \
    return target.at(concept);

int Counter::cooccurrence(const std::string& concept) const
{
    return cooccurrence(concept, false);
}

int Counter::cooccurrence(
    const std::string& concept,
    const bool is_negative) const
{
    const auto& target =
        is_negative ? m_neg_cooccurrences : m_cooccurrences;
    COUNT_AT(target, concept);
}

int Counter::dependency(const std::string& concept) const
{
    return dependency(concept, false);
}

int Counter::dependency(
    const std::string& concept,
    const bool is_negative) const
{
    const auto& target =
        is_negative ? m_neg_dependencies : m_dependencies;
    COUNT_AT(target, concept);
}

int Counter::simile(const std::string& concept) const
{
    return simile(concept, false);
}

int Counter::simile(
    const std::string& concept,
    const bool is_negative) const
{
    const auto& target =
        is_negative ? m_neg_similia : m_similia;
    COUNT_AT(target, concept);
}

int Counter::comparative(
    const std::string& concept0,
    const std::string& concept1) const
{
    // concept0 > concept1
    {
        auto it = m_comparatives.find(concept0);
        if (it == m_comparatives.end()) {
            return 0;
        }
    }
    auto& submap = m_comparatives.at(concept0);
    COUNT_AT(submap, concept1);
}
} // namespace order_concepts
