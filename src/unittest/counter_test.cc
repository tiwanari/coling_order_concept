#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <algorithm>
#include "../counter.h"
#include "../file_format.h"

using namespace order_concepts;

class CounterTest : public ::testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
    void checkOutputFileIsValid(
        const std::string& filename,
        const std::string& adjective,
        const std::vector<std::string> concepts);
};

void CounterTest::checkOutputFileIsValid(
    const std::string& target_filename,
    const std::string& adjective,
    const std::vector<std::string> concepts)
{
    std::ifstream target_file;
    target_file.open(target_filename);

    std::string line;
    while (std::getline(target_file, line)) {
        // line = e.g. 大きい,犬,SIMILE,0
        std::vector<std::string> splitted_line;
        util::splitStringUsing(line, ",", &splitted_line);

        // adjective check
        // occurrence's the first element can be concept
        bool is_stats =
            Counter::isStatsVal(splitted_line[0]);
        if (!is_stats) {
            ASSERT_TRUE(splitted_line[0] == adjective);
            auto it =
                std::find(concepts.begin(), concepts.end(), splitted_line[1]);
            ASSERT_TRUE(it != concepts.end());
        }

        // TODO: more check
    }
    target_file.close();
}

TEST_F(CounterTest, setAQuery)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::vector<std::string>& concepts =
        counter.concepts();
    ASSERT_EQ(kConcepts.size(), concepts.size());
    for (int i = 0; i < kConcepts.size(); ++i) {
        ASSERT_EQ(kConcepts[i], concepts[i]);
    }

    const std::string& adjective = counter.adjective();
    ASSERT_EQ(kAdjective, adjective);
}

TEST_F(CounterTest, openFileSuccessfully)
{
    const std::vector<std::string> kConcepts = {"",};
    const std::string kAdjective = "";
    const std::string kAntonym = "";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kExistingDataset =
        "./unittest/dataset/counter_test_data0.JDP";
    ASSERT_NO_THROW({
        Parser parser(Morph::MORPH_TYPE::JUMAN, kExistingDataset);
        counter.count(parser);
    });
}

TEST_F(CounterTest, openFileUnsuccessfully)
{
    const std::vector<std::string> kConcepts = {"",};
    const std::string kAdjective = "";
    const std::string kAntonym = "";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kNotExistingDataset = "./not_exist.txt";
    ASSERT_THROW({
        Parser parser(Morph::MORPH_TYPE::JUMAN, kNotExistingDataset);
        counter.count(parser);
    }, std::runtime_error);
}

TEST_F(CounterTest, countCooccurrenceInAMiddleFile)
{
    const std::vector<std::string> kConcepts =
        {"ホットプレート"};
    const std::string kAdjective = "重い";
    const std::string kAntonym = "軽い";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kMiddleDataset =
        "./unittest/dataset/counter_test_data1.JDP";
    Parser parser(Morph::MORPH_TYPE::JUMAN, kMiddleDataset);
    counter.count(parser);
    ASSERT_EQ(2, counter.cooccurrence(kConcepts[0]));
}

TEST_F(CounterTest, readPatternsFromFile)
{
    const std::vector<std::string> kConcepts = {"",};
    const std::string kAdjective = "";
    const std::string kAntonym = "";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kPatternFilesPath =
        "./unittest/dataset/ja/count_patterns/juman";
    ASSERT_TRUE(counter.readPatternFilesInPath(kPatternFilesPath));

    const std::vector<std::vector<std::string>> kSimilePatterns =
        {
            {"!CONCEPT0", "だ", "ようだ", "!ADJECTIVE", },
        };
    const std::vector<std::vector<std::string>> kComparativePatterns =
        {
            {"より", "!ADJECTIVE", },
            {"ほど", "!NEG_ADJECTIVE", },
        };
    ASSERT_EQ(kSimilePatterns, counter.similePatterns());
    ASSERT_EQ(kComparativePatterns, counter.comparativePatterns());

    const std::string kNotExistingPath = "./not_exist";
    ASSERT_FALSE(counter.readPatternFilesInPath(kNotExistingPath));
    ASSERT_EQ(0, counter.similePatterns().size());
    ASSERT_EQ(0, counter.comparativePatterns().size());
}

TEST_F(CounterTest, saveToFile)
{
    const std::vector<std::string> kConcepts =
        {"くじら",
        "犬",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kPatternFilesPath =
        "./unittest/dataset/ja/count_patterns/juman";
    ASSERT_TRUE(counter.readPatternFilesInPath(kPatternFilesPath));

    const std::string kSmallDataset =
        "./unittest/dataset/counter_test_data0.JDP";
    Parser parser(Morph::MORPH_TYPE::JUMAN, kSmallDataset);
    counter.count(parser);

    const std::string kOutputFile =
        "./unittest/out/counter_test_out0.txt";
    counter.save(kOutputFile);

    struct stat buffer;
    // check path existence
    ASSERT_EQ(0, ::stat(kOutputFile.c_str(), &buffer));
    // check the
    checkOutputFileIsValid(kOutputFile, kAdjective, kConcepts);
}

TEST_F(CounterTest, countStatsModeInASmallFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kSmallDataset =
        "./unittest/dataset/counter_test_data0.JDP";
    counter.setPrepMode(true);
    Parser parser(Morph::MORPH_TYPE::JUMAN, kSmallDataset);
    counter.count(parser);

    const std::string kOutputFile =
        "./unittest/out/counter_test_out1.txt";
    counter.save(kOutputFile);
}

TEST_F(CounterTest, writeConceptIdsToFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kSmallDataset =
        "./unittest/dataset/counter_test_data0.JDP";
    Parser parser(Morph::MORPH_TYPE::JUMAN, kSmallDataset);
    counter.count(parser);

    const std::vector<std::string> kAnswers =
        {"0,犬",
        "1,猫",
        "2,象",
        "3,ねずみ"};

    std::string file_path = util::concatStr(Counter::REASON_PATH, Counter::CONCEPT_IDS_FILE);
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());

    int i = 0;
    std::string line;
    while (getline(ifs, line)) ASSERT_EQ(kAnswers[i++], line);
    ASSERT_EQ(kAnswers.size(), i);
}

TEST_F(CounterTest, writeCooccurrenceSentencesToFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kSmallDataset =
        "./unittest/dataset/counter_test_data0.JDP";
    Parser parser(Morph::MORPH_TYPE::JUMAN, kSmallDataset);
    counter.count(parser);

    const std::vector<std::string> kAnswers =
        {"0\t + すくすくと育ち大きい犬になりました",
         "0\t + 犬は大きい。",
         "0\t + くじらは犬よりも大きい。",
         "3\t - ねずみは犬よりも大きくない。",
         "0\t - ねずみは犬よりも大きくない。",
         "3\t - ねずみは犬よりも大きくない。",
         "0\t - ねずみは犬よりも大きくない。",
         "0\t - 象は犬よりも大きくない。", // why this order is
         "2\t - 象は犬よりも大きくない。", // 0 -> 2 ?
         "0\t - 犬のように小さい。"};

    std::string file_path = util::concatStr(Counter::REASON_PATH, Counter::CO_OCCURRENCE_REASON_FILE);
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());

    int i = 0;
    std::string line;
    while (getline(ifs, line)) ASSERT_EQ(kAnswers[i++], line);
    ASSERT_EQ(kAnswers.size(), i);
}

TEST_F(CounterTest, writeDependencySentencesToFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kSmallDataset =
        "./unittest/dataset/counter_test_data0.JDP";
    Parser parser(Morph::MORPH_TYPE::JUMAN, kSmallDataset);
    counter.count(parser);

    // NOTE: this may be an unexpected output
    // but the current version of dependency check is somewhat loose
    // and it finds a dependency from "犬よりも" to "大きい."
    const std::vector<std::string> kAnswers =
        {"0\t + 犬は大きい。",
         "0\t + くじらは犬よりも大きい。", // this one!
         "3\t - ねずみは犬よりも大きくない。",
         "0\t - ねずみは犬よりも大きくない。", // this one!
         "3\t - ねずみは犬よりも大きくない。",
         "0\t - ねずみは犬よりも大きくない。", // this one!
         "2\t - 象は犬よりも大きくない。",
         "0\t - 象は犬よりも大きくない。", // this one!
         "0\t - 犬のように小さい。"};

    std::string file_path = util::concatStr(Counter::REASON_PATH, Counter::DEPENDENCY_REASON_FILE);
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());

    int i = 0;
    std::string line;
    while (getline(ifs, line)) ASSERT_EQ(kAnswers[i++], line);
    ASSERT_EQ(kAnswers.size(), i);
}

TEST_F(CounterTest, writeSimileSentencesToFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kPatternFilesPath =
        "./unittest/dataset/ja/count_patterns/juman";
    ASSERT_TRUE(counter.readPatternFilesInPath(kPatternFilesPath));

    const std::string kSmallDataset =
        "./unittest/dataset/counter_test_data0.JDP";
    Parser parser(Morph::MORPH_TYPE::JUMAN, kSmallDataset);
    counter.count(parser);

    const std::vector<std::string> kAnswers =
        {"0\t - 犬のように小さい。"};

    std::string file_path = util::concatStr(Counter::REASON_PATH, Counter::SIMILE_REASON_FILE);
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());

    int i = 0;
    std::string line;
    while (getline(ifs, line)) ASSERT_EQ(kAnswers[i++], line);
    ASSERT_EQ(kAnswers.size(), i);
}

TEST_F(CounterTest, writeComparativeSentencesToFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kPatternFilesPath =
        "./unittest/dataset/ja/count_patterns/juman";
    ASSERT_TRUE(counter.readPatternFilesInPath(kPatternFilesPath));

    const std::string kSmallDataset =
        "./unittest/dataset/counter_test_data0.JDP";
    Parser parser(Morph::MORPH_TYPE::JUMAN, kSmallDataset);
    counter.count(parser);

    const std::vector<std::string> kAnswers =
        {"0\t + ねずみは犬よりも大きくない。",
         "3\t - ねずみは犬よりも大きくない。",
         "0\t + ねずみは犬よりも大きくない。",
         "3\t - ねずみは犬よりも大きくない。",
         "0\t + 象は犬よりも大きくない。",
         "2\t - 象は犬よりも大きくない。"};

    std::string file_path = util::concatStr(Counter::REASON_PATH, Counter::COMPARATIVE_REASON_FILE);
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());

    int i = 0;
    std::string line;
    while (getline(ifs, line)) ASSERT_EQ(kAnswers[i++], line);
    ASSERT_EQ(kAnswers.size(), i);
}

TEST_F(CounterTest, countAdjectiveAdverbsInFilesFormattedInIPA)
{
    const std::vector<std::string> kConcepts =
        {"百合",
        "バラ", };
    const std::string kAdjective = "綺麗だ";
    const std::string kAntonym = "";
    Counter counter(kConcepts, kAdjective, "");

    const std::string kPatternFilesPath
        = "./unittest/dataset/ja/count_patterns/ipa";
    ASSERT_TRUE(counter.readPatternFilesInPath(kPatternFilesPath));

    const std::string kSmallDataset =
        "./unittest/dataset/counter_test_data5.JDP";
    Parser parser(Morph::MORPH_TYPE::IPADIC, kSmallDataset);
    counter.count(parser);

    ASSERT_EQ(6, counter.cooccurrence(kConcepts[0]));
    ASSERT_EQ(3, counter.cooccurrence(kConcepts[1]));

    ASSERT_EQ(5, counter.dependency(kConcepts[0]));
    ASSERT_EQ(1, counter.dependency(kConcepts[1]));

    ASSERT_EQ(1, counter.simile(kConcepts[0]));
    ASSERT_EQ(1, counter.simile(kConcepts[1]));

    ASSERT_EQ(0, counter.comparative(kConcepts[0], kConcepts[1]));
    ASSERT_EQ(1, counter.comparative(kConcepts[1], kConcepts[0]));
}


/** tests in various morph types **/
struct CounterSmallTestData {
    std::string pattern_filename;
    Morph::MORPH_TYPE type;
    std::string filename;
    FileFormat format;
} kSmallDatasets[] = {
    { "./unittest/dataset/ja/count_patterns/juman", Morph::MORPH_TYPE::JUMAN,
                "./unittest/dataset/counter_test_data0.JDP", FileFormat::PLAIN, },
    { "./unittest/dataset/ja/count_patterns/ipa", Morph::MORPH_TYPE::IPADIC,
                "./unittest/dataset/counter_test_data4.JDP", FileFormat::PLAIN, },
};

class CounterVariousFormatTest : public ::testing::TestWithParam<CounterSmallTestData> {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};


TEST_P(CounterVariousFormatTest, countCooccurrenceInASmallFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kSmallDataset = GetParam().filename;
    Parser parser(GetParam().type, kSmallDataset);
    counter.count(parser);

    ASSERT_EQ(3, counter.cooccurrence(kConcepts[0]));
    ASSERT_EQ(0, counter.cooccurrence(kConcepts[1]));
}

TEST_P(CounterVariousFormatTest, countDependencyInASmallFile)
{
    const std::vector<std::string> kConcepts =
        {"犬",
        "猫",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kSmallDataset = GetParam().filename;
    Parser parser(GetParam().type, kSmallDataset);
    counter.count(parser);

    ASSERT_EQ(2, counter.dependency(kConcepts[0]));
    ASSERT_EQ(0, counter.dependency(kConcepts[1]));
}

TEST_P(CounterVariousFormatTest, countSimileInASmallFile)
{
    const std::vector<std::string> kConcepts =
        {"くじら",
        "犬",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kPatternFilesPath = GetParam().pattern_filename;
    ASSERT_TRUE(counter.readPatternFilesInPath(kPatternFilesPath));

    const std::string kSmallDataset = GetParam().filename;
    Parser parser(GetParam().type, kSmallDataset);
    counter.count(parser);

    ASSERT_EQ(1, counter.simile(kConcepts[0]));
    ASSERT_EQ(0, counter.simile(kConcepts[1]));
}

TEST_P(CounterVariousFormatTest, countComparativeInASmallFile)
{
    const std::vector<std::string> kConcepts =
        {"くじら",
        "犬",
        "象",
        "ねずみ", };
    const std::string kAdjective = "大きい";
    const std::string kAntonym = "小さい";
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kPatternFilesPath = GetParam().pattern_filename;
    ASSERT_TRUE(counter.readPatternFilesInPath(kPatternFilesPath));

    const std::string kSmallDataset = GetParam().filename;
    Parser parser(GetParam().type, kSmallDataset);
    counter.count(parser);

    ASSERT_EQ(1, counter.comparative(kConcepts[0], kConcepts[1]));
    ASSERT_EQ(2, counter.comparative(kConcepts[1], kConcepts[3]));
}

INSTANTIATE_TEST_CASE_P(
    TestWithSmallDataSet,
    CounterVariousFormatTest,
    ::testing::ValuesIn(kSmallDatasets));

/** comparative pattern tests **/
const std::string COMPARATIVE_DATA_PATH
    = "./unittest/dataset/counter_test_comparative/counter_test_comparative_";
struct CounterComparativeTestData {
    std::string filename;
    std::string concept0;
    std::string concept1;
    std::string adjective;
    std::string antonym;
} kPatterns[] = {
    // { COMPARATIVE_DATA_PATH + "data0.JDP", "バス", "電車", "速い", "遅い", },
    // { COMPARATIVE_DATA_PATH + "data1.JDP", "バス", "電車", "速い", "遅い", }, // で行くより
    { COMPARATIVE_DATA_PATH + "data2.JDP", "電車", "バス", "速い", "遅い", },
    // { COMPARATIVE_DATA_PATH + "data3.JDP", "バス", "電車", "速い", "遅い", }, // 乗り継ぐより
    // { COMPARATIVE_DATA_PATH + "data4.JDP", "電車", "バス", "速い", "遅い", }, // 早いし遅れも
    { COMPARATIVE_DATA_PATH + "data5.JDP", "電車", "バス", "速い", "遅い", },
    { COMPARATIVE_DATA_PATH + "data6.JDP", "電車", "バス", "速い", "遅い", },
    { COMPARATIVE_DATA_PATH + "data7.JDP", "電車", "バス", "速い", "遅い", },
    { COMPARATIVE_DATA_PATH + "data8.JDP", "バス", "電車", "速い", "遅い", }, // NG: 速いだって？
    { COMPARATIVE_DATA_PATH + "data9.JDP", "電車", "バス", "速い", "遅い", },
    { COMPARATIVE_DATA_PATH + "data10.JDP", "電車", "バス", "速い", "遅い", },
    { COMPARATIVE_DATA_PATH + "data11.JDP", "バス", "電車", "速い", "遅い", },
    { COMPARATIVE_DATA_PATH + "data12.JDP", "バス", "電車", "速い", "遅い", },
    { COMPARATIVE_DATA_PATH + "data13.JDP", "バス", "電車", "速い", "遅い", }, // NG: なぜか速いです
    { COMPARATIVE_DATA_PATH + "data14.JDP", "電車", "バス", "速い", "遅い", },
    { COMPARATIVE_DATA_PATH + "data15.JDP", "ビール", "ワイン", "美味しい", "不味い", },
    // { COMPARATIVE_DATA_PATH + "data16.JDP", "ビール", "ワイン", "美味しい", "不味い", },
    { COMPARATIVE_DATA_PATH + "data17.JDP", "ビール", "ワイン", "美味しい", "不味い", }, // やっぱり
    { COMPARATIVE_DATA_PATH + "data18.JDP", "ワイン", "ビール", "美味しい", "不味い", },
    // { COMPARATIVE_DATA_PATH + "data19.JDP", "ワイン", "ビール", "美味しい", "不味い", }, // ワイン vs 赤ワイン ??
    { COMPARATIVE_DATA_PATH + "data20.JDP", "ワイン", "ビール", "美味しい", "不味い", },
    // { COMPARATIVE_DATA_PATH + "data21.JDP", "ワイン", "ビール", "美味しい", "不味い", }, // DEPEND: 感じた
    // { COMPARATIVE_DATA_PATH + "data22.JDP", "ワイン", "ビール", "美味しい", "不味い", }, // PHRASE: ビールですが・・ワインより
    // { COMPARATIVE_DATA_PATH + "data23.JDP", "ビール", "ワイン", "美味しい", "不味い", }, // DEPEND: ことも
    // { COMPARATIVE_DATA_PATH + "data24.JDP", "ワイン", "ビール", "美味しい", "不味い", }, // DEPEND: 感じます
    { COMPARATIVE_DATA_PATH + "data25.JDP", "ビール", "ワイン", "美味しい", "不味い", },
    // { COMPARATIVE_DATA_PATH + "data26.JDP", "ワイン", "ビール", "美味しい", "不味い", }, // DEPEND: 感じて
    { COMPARATIVE_DATA_PATH + "data27.JDP", "ビール", "ワイン", "美味しい", "不味い", },
    { COMPARATIVE_DATA_PATH + "data28.JDP", "ワイン", "ビール", "美味しい", "不味い", },
    { COMPARATIVE_DATA_PATH + "data29.JDP", "ビール", "ワイン", "美味しい", "不味い", },
    { COMPARATIVE_DATA_PATH + "data30.JDP", "ワイン", "ビール", "美味しい", "不味い", },
    { COMPARATIVE_DATA_PATH + "data31.JDP", "バラ", "ラベンダー", "綺麗だ", "醜い", },
    // 3 patters for data 32
    // { COMPARATIVE_DATA_PATH + "data32.JDP", "チェロ", "ビオラ", "心地よい", "不快だ", },
    // { COMPARATIVE_DATA_PATH + "data32.JDP", "ビオラ", "バイオリン", "心地よい", "不快だ", },
    { COMPARATIVE_DATA_PATH + "data32.JDP", "チェロ", "バイオリン", "心地よい", "不快だ", },
    // { COMPARATIVE_DATA_PATH + "data33.JDP", "豚肉", "牛肉", "好きだ", "嫌いだ", },
    // { COMPARATIVE_DATA_PATH + "data34.JDP", "豚肉", "牛肉", "好きだ", "嫌いだ", }, // PHRASE: 豚肉大好き♡牛肉・鶏肉よりも，
    { COMPARATIVE_DATA_PATH + "data35.JDP", "豚肉", "牛肉", "好きだ", "嫌いだ", },
    // 2 patterns for data 36
    // { COMPARATIVE_DATA_PATH + "data36.JDP", "豚肉", "牛肉", "好きだ", "嫌いだ", },
    // { COMPARATIVE_DATA_PATH + "data36.JDP", "鳥肉", "牛肉", "好きだ", "嫌いだ", },
    { COMPARATIVE_DATA_PATH + "data37.JDP", "鶏肉", "豚肉", "好きだ", "嫌いだ", },
    { COMPARATIVE_DATA_PATH + "data38.JDP", "鳥肉", "牛肉", "好きだ", "嫌いだ", },
    // { COMPARATIVE_DATA_PATH + "data39.JDP", "ラム", "鶏肉", "好きだ", "嫌いだ", }, // 牛肉や鶏肉より - マトンやラム (w/o 肉)
    { COMPARATIVE_DATA_PATH + "data40.JDP", "豚肉", "牛肉", "好きだ", "嫌いだ", },
    { COMPARATIVE_DATA_PATH + "data41.JDP", "豚肉", "鶏肉", "好きだ", "嫌いだ", },
    { COMPARATIVE_DATA_PATH + "data42.JDP", "鶏肉", "牛肉", "好きだ", "嫌いだ", },
    { COMPARATIVE_DATA_PATH + "data43.JDP", "豚肉", "牛肉", "好きだ", "嫌いだ", },
    // { COMPARATIVE_DATA_PATH + "data44.JDP", "鶏肉", "牛肉", "好きだ", "嫌いだ", }, // DEPEND: not depend
    // 3 patterns for data45: 野菜 > 豚肉 > 牛肉
    // { COMPARATIVE_DATA_PATH + "data45.JDP", "豚肉", "牛肉", "好きだ", "嫌いだ", },
    { COMPARATIVE_DATA_PATH + "data45.JDP", "野菜", "豚肉", "好きだ", "嫌いだ", },
    // { COMPARATIVE_DATA_PATH + "data45.JDP", "野菜", "牛肉", "好きだ", "嫌いだ", },
    // { COMPARATIVE_DATA_PATH + "data46.JDP", "イタリア", "スペイン", "危険だ", "安全だ", }, // DEPEND: 国だ
    // { COMPARATIVE_DATA_PATH + "data47.JDP", "日本", "ロシア", "危険だ", "安全だ", }, // DEPEND: 地域だ
    { COMPARATIVE_DATA_PATH + "data48.JDP", "インド", "タイ", "危険だ", "安全だ", },
    // { COMPARATIVE_DATA_PATH + "data49.JDP", "タイ", "日本", "危険だ", "安全だ", }, // NEW: ほどではないにしても
    // { COMPARATIVE_DATA_PATH + "data50.JDP", "日本", "スペイン", "危険だ", "安全だ", }, // DEPEND: not depend
    { COMPARATIVE_DATA_PATH + "data51.JDP", "日本", "スペイン", "安全だ", "危険だ", }, // NG: 安全かどうか？！
    { COMPARATIVE_DATA_PATH + "data52.JDP", "ドラゴンボール", "ワンピース", "面白い", "つまらない", },
    // { COMPARATIVE_DATA_PATH + "data53.JDP", "ファミマ", "ローソン", "便利だ", "不便だ", },
    { COMPARATIVE_DATA_PATH + "data54.JDP", "ファミマ", "セブンイレブン", "便利だ", "不便だ", },
    { COMPARATIVE_DATA_PATH + "data55.JDP", "バーガーキング", "マクドナルド", "美味しい", "不味い", },
    // 2 patterns for data56
    // { COMPARATIVE_DATA_PATH + "data56.JDP", "ロッテリア", "マック", "高い", "安い", }, // DEPEND: too difficult
    // { COMPARATIVE_DATA_PATH + "data56.JDP", "ロッテリア", "モスバーガー", "安い", "高い", }, // DEPEND: too difficult
    { COMPARATIVE_DATA_PATH + "data57.JDP", "モスバーガー", "フレッシュネスバーガー", "美味しい", "不味い", },
    { COMPARATIVE_DATA_PATH + "data58.JDP", "バーガーキング", "マクドナルド", "美味しい", "不味い", },
    // { COMPARATIVE_DATA_PATH + "data59.JDP", "モスバーガー", "マクドナルド", "美味しい", "不味い", }, // DEPEND: イメージのようです
    // { COMPARATIVE_DATA_PATH + "data60.JDP", "モスバーガー", "マクドナルド", "美味しい", "不味い", }, // DEPEND: 感じた
    { COMPARATIVE_DATA_PATH + "data61.JDP", "セブンイレブン", "ローソン", "多い", "少ない", },
    { COMPARATIVE_DATA_PATH + "data62.JDP", "ファミマ", "ローソン", "多い", "少ない", },
    { COMPARATIVE_DATA_PATH + "data63.JDP", "ファミマ", "ローソン", "多い", "少ない", },
    { COMPARATIVE_DATA_PATH + "data64.JDP", "ファミマ", "セブンイレブン", "多い", "少ない", },
    // { COMPARATIVE_DATA_PATH + "data65.JDP", "銀閣寺", "金閣寺", "古い", "新しい", },
};

class CounterComparativeTest : public ::testing::TestWithParam<CounterComparativeTestData> {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_P(CounterComparativeTest, countComparativeMorePatterns)
{
    const std::string kSmallDataset = GetParam().filename;
    const std::vector<std::string> kConcepts =
        {GetParam().concept0,
         GetParam().concept1, };
    const std::string kAdjective = GetParam().adjective;
    const std::string kAntonym = GetParam().antonym;
    Counter counter(kConcepts, kAdjective, kAntonym);

    const std::string kPatternFilesPath =
        "./unittest/dataset/ja/count_patterns/juman";
    ASSERT_TRUE(counter.readPatternFilesInPath(kPatternFilesPath));

    Parser parser(Morph::MORPH_TYPE::JUMAN, kSmallDataset);
    counter.count(parser);

    ASSERT_EQ(1, counter.comparative(GetParam().concept0, GetParam().concept1));
}

INSTANTIATE_TEST_CASE_P(
    ComparativeTestWithParams,
    CounterComparativeTest,
    ::testing::ValuesIn(kPatterns));
