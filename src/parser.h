#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include "file_decoder.h"
#include "file_format.h"
#include "phrase.h"
#include "morph.h"

namespace order_concepts {
class Parser {
private:
    static constexpr const char* SOS_OR_SOURCE_PREFIX = "#";
    static constexpr const char* SOS_SECOND_PREFIX = "S-ID:";
    static constexpr const char* DIRECTIVE_PREFIX = "*";
    static constexpr const char* EOS_PREFIX = "EOS";
    enum class Type : unsigned char {
        SOURCE,
        PLAIN,
        DIRECTIVE,
        SOS, // start of sentence
        EOS, // end of sentence
        UNKNOWN,
    };
private:
    long m_num_of_lines;
    std::string m_input_filename;
    std::ifstream m_input_file;
    std::string m_cur_sentence;
    Phrase m_cur_phrase;
    std::vector<Phrase> m_cur_phrases;
    std::vector<int> m_cur_connections;
    FileFormat m_format;
    FileDecoder* m_decoder;
private:
    void init(
        const Morph::MORPH_TYPE type,
        const std::string& input_filename,
        const FileFormat& format);
    Parser::Type sentenceType(
        const std::vector<std::string>& splitted_line) const;
    Parser::Type parseLine(const std::string& line);
public:
    Parser(const Morph::MORPH_TYPE type, const std::string& input_filename);
    Parser(const Morph::MORPH_TYPE type,
            const std::string& input_filename,
            const FileFormat& file_format);
    virtual ~Parser();
    bool next();
    const long& numberOfLines() const { return m_num_of_lines; }
    const std::string& raw() const { return m_cur_sentence; }
    const std::vector<Phrase>& phrases() const { return m_cur_phrases; }
    std::vector<std::shared_ptr<Morph>> morphs() const;
    const std::vector<int>& connections() const { return m_cur_connections; }
};
} // namespace order_concepts
