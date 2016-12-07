#pragma once
#include <string>
#include <sstream>
#include <lzma.h>
#include "file_decoder.h"

namespace order_concepts {
class XzDecoder : public FileDecoder {
private:
    const static int BUFFER_SIZE = 8192;
private:
    uint8_t m_buffer[BUFFER_SIZE];
    lzma_stream m_stream;
    std::stringstream m_string_stream;

    // disallows copy
    XzDecoder(const XzDecoder &);
    XzDecoder &operator=(const XzDecoder &);

public:
    XzDecoder(std::istream* input);
    ~XzDecoder() { close(); }
    bool getline(std::string* line) override { return getline(line, '\n'); }
    bool getline(std::string* line, const char& delim) override;
    std::size_t read(void* output_buffer, const std::size_t& size);
    void close();
};
} // namespace order_concepts
