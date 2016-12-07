#pragma once
#include <iostream>
#include <fstream>
#include <string>

namespace order_concepts {
class FileDecoder {
protected:
    std::istream* m_input_stream;
    bool m_is_eof;
    bool m_does_fail;
public:
    FileDecoder(std::istream* input)
        : m_input_stream(nullptr), m_is_eof(false), m_does_fail(false) {}
    virtual ~FileDecoder() {
        m_input_stream = nullptr;
        m_is_eof = false;
        m_does_fail = false;
    }
    virtual bool getline(std::string* line) { return getline(line, '\n'); }
    virtual bool getline(std::string* line, const char& delim) = 0;
    bool eof() const { return m_is_eof; }
    bool fail() const { return m_does_fail; }
};
} // namespace order_concepts
