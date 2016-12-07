#include <gtest/gtest.h>
#include <string>
#include <sys/stat.h>
#include <fstream>
#include <map>
#include "../reducer.h"

using namespace order_concepts;

class ReducerTest : public ::testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(ReducerTest, allFilesExist)
{
    const std::vector<std::string> kFiles =
        {"./unittest/dataset/reducer_test_exist0.txt",
         "./unittest/dataset/reducer_test_exist1.txt",
         "./unittest/dataset/reducer_test_exist2.txt",
         "./unittest/dataset/reducer_test_exist3.txt", };
    const std::string kOutputFile = "./unittest/out/reducer_exist_test.out";
    Reducer reducer(kFiles);
    ASSERT_TRUE(reducer.reduce(kOutputFile));

    struct stat buffer;
    // check path existence
    ASSERT_EQ(0, ::stat(kOutputFile.c_str(), &buffer));
}

TEST_F(ReducerTest, aFileDoesNotExist)
{
    const std::vector<std::string> kFiles =
        {"./unittest/dataset/reducer_test_exist0.txt",
         "./unittest/dataset/reducer_test_exist1.txt",
         "./unittest/dataset/reducer_test_exist2.txt",
         "./unittest/dataset/reducer_test_exist3.txt",
         "./not_exist.txt", };
    const std::string kOutputFile = "./unittest/out/reducer_not_exist_test.out";
    Reducer reducer(kFiles);
    ASSERT_FALSE(reducer.reduce(kOutputFile));

    struct stat buffer;
    // check path existence
    ASSERT_EQ(0, ::stat(kOutputFile.c_str(), &buffer));
}

TEST_F(ReducerTest, reduceCountIntoOne)
{
    const std::vector<std::string> kFiles =
        {"./unittest/dataset/reducer_test_data0.txt",
         "./unittest/dataset/reducer_test_data1.txt", };
    const std::string kOutputFile = "./unittest/out/reducer_test_count0.out";

    Reducer reducer(kFiles);
    ASSERT_TRUE(reducer.reduce(kOutputFile));

    // check output file
    const std::map<std::string, std::string> kCorrectPairs =
        {{"ADJECTIVE,大きい,OCCURRENCE", "12"},
         {"ADJECTIVE,大きい,NEG_OCCURRENCE", "12"},
         {"ADJECTIVE,大きい,DEPENDENCY", "13"},
         {"ADJECTIVE,大きい,NEG_DEPENDENCY", "11"},
         {"ADJECTIVE,大きい,SIMILE", "4"},
         {"ADJECTIVE,大きい,NEG_SIMILE", "5"},
         {"大きい,犬,CO_OCCURRENCE", "7"},
         {"大きい,くじら,CO_OCCURRENCE", "3"},
         {"大きい,犬,NEG_CO_OCCURRENCE", "10"},
         {"大きい,くじら,NEG_CO_OCCURRENCE", "6"},
         {"大きい,犬,DEPENDENCY", "7"},
         {"大きい,くじら,DEPENDENCY", "6"},
         {"大きい,犬,NEG_DEPENDENCY", "12"},
         {"大きい,くじら,NEG_DEPENDENCY", "2"},
         {"大きい,犬,SIMILE", "0"},
         {"大きい,くじら,SIMILE", "2"},
         {"大きい,犬,NEG_SIMILE", "2"},
         {"大きい,くじら,NEG_SIMILE", "0"},
         {"大きい,犬,COMPARATIVE,くじら", "5"},
         {"大きい,くじら,NEG_COMPARATIVE,犬", "3"},
         {"大きい,くじら,COMPARATIVE,犬", "3"},
         {"大きい,犬,NEG_COMPARATIVE,くじら", "3"},};

    std::ifstream target_file;
    target_file.open(kOutputFile);

    std::string line;
    int line_count = 0;
    while (std::getline(target_file, line)) {
        line_count++;
        // line = e.g. 大きい,犬,SIMILE,0
        auto index = line.find_last_of(",");
        ASSERT_NE(index, std::string::npos);
        // split string into a pair of key and value
        const std::string& key = line.substr(0, index);
        const std::string& value = line.substr(index+1);
        ASSERT_EQ(kCorrectPairs.at(key), value);
    }
    ASSERT_EQ(kCorrectPairs.size(), line_count);
    target_file.close();
}
