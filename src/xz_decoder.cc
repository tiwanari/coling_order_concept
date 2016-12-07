#include <cassert>
#include <string>
#include <lzma.h>
#include <sstream>
#include <iostream>
#include "xz_decoder.h"

namespace order_concepts {
/**
 * @param input stream (e.g. std::cin, file, etc.)
 */
XzDecoder::XzDecoder(std::istream* input)
: FileDecoder(input), m_stream(), m_string_stream()
{
    // initialize
    lzma_stream initial_lzma = LZMA_STREAM_INIT;
    m_stream = initial_lzma;

    lzma_ret ret =
        lzma_stream_decoder(&m_stream, lzma_easy_decoder_memusage(9), 0);
    if (ret != LZMA_OK || !input) {
        m_does_fail = true;
        std::cerr << "XzDecoder: liblzma initialization failed" << std::endl;
        return ;
    }

    m_input_stream = input;
}

void XzDecoder::close()
{
    if (m_input_stream != nullptr) {
        lzma_end(&m_stream);
    }

    lzma_stream initial_lzma = LZMA_STREAM_INIT;
    m_stream = initial_lzma;
}

std::size_t XzDecoder::read(void* output_buffer, const std::size_t& size)
{
    if (m_input_stream == nullptr || m_is_eof || m_does_fail) {
        return false;   // false is considered as 0
    }

    // output setting
    m_stream.next_out = static_cast<uint8_t*>(output_buffer);
    m_stream.avail_out = size;
    while (m_stream.avail_out > 0) {
        // read data from input stream if input data haven't been set yet
        if (m_stream.avail_in == 0) {
            // read data from input stream
            m_input_stream->read(reinterpret_cast<char*>(m_buffer), BUFFER_SIZE);
            m_stream.next_in = m_buffer;
            m_stream.avail_in = m_input_stream->gcount();
        }

        if (!*m_input_stream) {
            // finalize stream if no more input
            lzma_ret ret = lzma_code(&m_stream, LZMA_FINISH);
            if (ret == LZMA_OK) {
                continue; // TODO: check why this should be 'continue'
            }
            else {
                // successful termination
                if (ret == LZMA_STREAM_END) {
                    m_is_eof = true;
                }
                else {
                    m_does_fail = true;
                }
                break;
            }
        }
        else {
            // read more
            lzma_ret ret = lzma_code(&m_stream, LZMA_RUN);
            if (ret != LZMA_OK) {
                m_does_fail = true;
                break;
            }
        }
    }
    return size - m_stream.avail_out;
}

bool XzDecoder::getline(std::string* line, const char& delim)
{
    while (true) {
        // check the stored string has the delimiter or not
        // tellg() tells the current position of the input stream
        // i.e. tells by where the stream has been read
        const std::string& stored_string =
            m_string_stream.str().substr(m_string_stream.tellg());
        if (stored_string.find(delim) != std::string::npos) {
            std::getline(m_string_stream, *line, delim);
            return true;
        }

        // reduce the size of stringstream
        m_string_stream.str("");    // reset buffer
        m_string_stream.clear();    // reset flags
        m_string_stream << stored_string;

        // read more
        // +1 for null character
        uint8_t output_buffer[BUFFER_SIZE + 1] = {0};
        const std::size_t readCount = read(output_buffer, BUFFER_SIZE);
        m_string_stream << output_buffer;   // append data

        // no more input
        if (readCount <= 0) {
            // return the rest stream data
            *line = stored_string;
            // check input data is not null
            return m_string_stream.gcount() > 0;
        }
    }
    // program shouldn't reach this sentence i.e. something wrong
    return false;
}

} // namespace order_concepts
