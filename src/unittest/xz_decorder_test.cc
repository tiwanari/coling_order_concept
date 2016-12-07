#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <iostream>
#include "../xz_decoder.h"

using namespace order_concepts;

class XzDecoderTest : public ::testing::TestWithParam<int> {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_P(XzDecoderTest, decordAFile)
{
    const std::string kInputFile = "./unittest/dataset/xz_decoder_data0.txt.xz";
    const std::size_t kBufferSize = GetParam();
    const std::string kExpectedString = "hello\nworld\n";
    char buffer[kBufferSize + 1];
    std::ifstream input_file;
    input_file.open(kInputFile);

    ASSERT_TRUE(input_file);

    XzDecoder decoder(&input_file);
    int i = 0;
    int input_size;
    while ((input_size = decoder.read(&buffer, kBufferSize)) > 0) {
        buffer[input_size] = '\0';
        ASSERT_STREQ(kExpectedString.substr(i, input_size).c_str(), buffer);
        i += input_size;
    }
    input_file.close();
}

INSTANTIATE_TEST_CASE_P(
    TestWithDifferentBufferSize,
    XzDecoderTest,
    ::testing::Values(1, 2, 5, 10, 100, 1000));

TEST_F(XzDecoderTest, decordAFileAsLines)
{
    const std::string kInputFile = "./unittest/dataset/xz_decoder_data0.txt.xz";
    const std::string kExpectedLines[] = {
        "hello",
        "world",
    };
    std::ifstream input_file;
    input_file.open(kInputFile);

    ASSERT_TRUE(input_file);

    XzDecoder decoder(&input_file);
    int i = 0;
    std::string line;
    while (decoder.getline(&line)) {
        ASSERT_EQ(kExpectedLines[i++], line);
    }
    input_file.close();
}
