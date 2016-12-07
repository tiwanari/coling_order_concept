#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <cctype>
#include <algorithm>
#include "../svm_data_formatter.h"
#include "../util/split.h"
#include "../util/type.h"

using namespace order_concepts;

class SvmDataFormatterTest : public ::testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(SvmDataFormatterTest, openFileSuccessfully)
{
    const std::string kExistingDataset =
        "./unittest/dataset/svm_data_formatter_test_data0.txt";
    SvmDataFormatter formatter(kExistingDataset);
    const std::string kOutputPath = "./unittest/out/svm_data_formatter_test0";
    ASSERT_NO_THROW({
        formatter.format(kOutputPath);
    });
}

TEST_F(SvmDataFormatterTest, openFileUnsuccessfully)
{
    const std::string kNotExistingDataset = "./not_exist.txt";
    SvmDataFormatter formatter(kNotExistingDataset);
    const std::string kOutputPath = "./unittest/out/svm_data_formatter_test1";
    ASSERT_THROW({
        formatter.format(kOutputPath);
    }, std::runtime_error);
}

TEST_F(SvmDataFormatterTest, outputFormatedData)
{
    const std::string kExistingDataset =
        "./unittest/dataset/svm_data_formatter_test_data0.txt";
    SvmDataFormatter formatter(kExistingDataset);
    const std::string kOutputPath = "./unittest/out/svm_data_formatter_test2";
    formatter.format(kOutputPath);

    // check output file
    std::ifstream target_file;
    target_file.open(formatter.outputFilePath());
    ASSERT_TRUE(target_file);

    std::string line;
    while (std::getline(target_file, line)) {
        std::vector<std::string> splitted_line;
        util::splitStringWithWhitespaces(line, &splitted_line);
        // not comment
        if (splitted_line[0] != "#") {
            // data = e.g. 1 qid:1 1:0 2:2 3:3 4:4 5:2
            int feature_id = 0;
            for (const auto& element : splitted_line) {
                if (element == "1") continue;    // skip the first element because it is a dummy
                std::vector<std::string> key_and_value;
                util::splitStringUsing(element, ":", &key_and_value);
                // not query id
                if (key_and_value[0] != "qid") {
                    // feature id should be a number string
                    // check all characters which compose the string are digits
                    ASSERT_TRUE(
                        std::all_of(key_and_value[0].cbegin(),
                                    key_and_value[0].cend(),
                                    isdigit));
                    // feature id should be ordered in ascending
                    ASSERT_GT(std::stoi(key_and_value[0]), feature_id);
                    feature_id = std::stoi(key_and_value[0]);
                }

                // this is the same check as that of feature id
                // value should be a number string e.g. "-0.123"
                ASSERT_TRUE(util::is<double>(key_and_value[1]));
            }
        }
    }
    target_file.close();
}
