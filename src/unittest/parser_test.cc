#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <future>
#include "../morph.h"
#include "../parser.h"
#include "../phrase.h"
#include "../file_format.h"

using namespace order_concepts;

struct ParserTestData {
    Morph::MORPH_TYPE type;
    std::string filename;
    FileFormat format;
} kJumanSmall[] = {
    { Morph::MORPH_TYPE::JUMAN, "./unittest/dataset/parser_test_data0.JDP", FileFormat::PLAIN, },
    { Morph::MORPH_TYPE::JUMAN, "./unittest/dataset/parser_test_data0.JDP.xz", FileFormat::XZ, },
};

ParserTestData kIPASmall[] = {
    { Morph::MORPH_TYPE::IPADIC, "./unittest/dataset/parser_test_data2.JDP", FileFormat::PLAIN, },
    { Morph::MORPH_TYPE::IPADIC, "./unittest/dataset/parser_test_data2.JDP.xz", FileFormat::XZ, },
};

class ParserJumanTest : public ::testing::TestWithParam<ParserTestData> {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

class ParserIPATest : public ::testing::TestWithParam<ParserTestData> {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

/** juman tests **/
TEST_P(ParserJumanTest, openFileSuccessfully)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kExistingDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    ASSERT_NO_THROW({
        Parser parser(kMorphType, kExistingDataset, kFileFormat);
    });
}

TEST_P(ParserJumanTest, openFileUnsuccessfully)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kNotExistingDataset = "./not_exist.txt";
    const FileFormat kFileFormat = GetParam().format;
    ASSERT_THROW({
        Parser parser(kMorphType, kNotExistingDataset, kFileFormat);
    }, std::runtime_error);
}

TEST_P(ParserJumanTest, readEmptyRawValue)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_EQ("", parser.raw());
}

TEST_P(ParserJumanTest, parseASentence)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    const std::string kFirstLine =
        "すくすくと育ち早いものであと３日で４ヶ月です。";
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_TRUE(parser.next());
    ASSERT_EQ(kFirstLine, parser.raw());
}

TEST_P(ParserJumanTest, parseSentences)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    const std::string kFirstLine =
        "すくすくと育ち早いものであと３日で４ヶ月です。";
    const std::string kSecondLine =
        "元気に育ってくれてるのでそれだけで嬉しいです。";
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_TRUE(parser.next());
    ASSERT_EQ(kFirstLine, parser.raw());
    ASSERT_TRUE(parser.next());
    ASSERT_EQ(kSecondLine, parser.raw());
    ASSERT_FALSE(parser.next());
}

// TEST_F(ParserJumanTest, parseLargeData)
// {
//     const Morph::MORPH_TYPE kMorphType = Morph::MORPH_TYPE::JUMAN;
//     const std::string kLargeDataset =
//         "./unittest/dataset/parser_test_data1.JDP";
//     const long kNumOfLines = 10000;
//     Parser parser(kMorphType, kLargeDataset);
//     while (parser.next()) {
//     }
//     ASSERT_EQ(kNumOfLines, parser.numberOfLines());
// }

TEST_P(ParserJumanTest, parseASentenceAsPhrases)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    const std::vector<std::string> kPhrases =
        {"すくすくと",
        "育ち",
        "早い",
        "もので",
        "あと",
        "３日で",
        "４ヶ月です。", };
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_TRUE(parser.next());

    const std::vector<Phrase>& parsed_phrases = parser.phrases();
    ASSERT_EQ(kPhrases.size(), parsed_phrases.size());
    for (int i = 0; i < kPhrases.size(); ++i)
        ASSERT_EQ(kPhrases[i], parsed_phrases[i].phrase());
}

// TEST_F(ParserJumanTest, parseInParallel)
// {
//     const Morph::MORPH_TYPE kMorphType = Morph::MORPH_TYPE::JUMAN;
//     const std::string kLargeDataset =
//         "./unittest/dataset/parser_test_data1.JDP";
//     const long kNumOfLines = 10000;
//     const int kNumParser = 3;
//
//     std::vector<int> count(kNumParser);
//     std::vector<std::thread> threads;
//     for (int i = 0; i < kNumParser; ++i) {
//         threads.push_back(
//             std::thread([i, kLargeDataset, &count] {
//                 Parser parser(kMorphType, kLargeDataset);
//                 while (parser.next()) {
//                 }
//                 count[i] = parser.numberOfLines();
//             })
//         );
//     }
//     for (std::thread& th : threads) th.join();
//     for (int i = 0; i < kNumParser; ++i) ASSERT_EQ(kNumOfLines, count[i]);
// }

TEST_P(ParserJumanTest, parseASentenceAsConnections)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    const std::vector<int> kConnections = {1, 2, 3, 6, 5, 6, -1, };
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_TRUE(parser.next());

    const std::vector<int>& connections = parser.connections();
    ASSERT_EQ(kConnections.size(), connections.size());
    for (int i = 0; i < kConnections.size(); ++i) {
        ASSERT_EQ(kConnections[i], connections[i]);
    }
}

TEST_P(ParserJumanTest, followConnections)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_TRUE(parser.next());

    const std::vector<int> kConnections[] =
        {{1, 2, 3, 6, -1, },
         {2, 3, 6, -1},
         {3, 6, -1},
         {6, -1},
         {5, 6, -1},
         {6, -1},
         {-1}};
    const std::vector<int>& connections = parser.connections();
    for (int i = 0; i < connections.size(); ++i) {
        int index = connections[i];
        int j = 0;
        while (index != -1) {
            ASSERT_EQ(kConnections[i][j++], index);
            index = connections[index];
        }
    }
}

INSTANTIATE_TEST_CASE_P(
    TestWithSmallJumanDataSet,
    ParserJumanTest,
    ::testing::ValuesIn(kJumanSmall));

/** IPA dic tests **/
TEST_P(ParserIPATest, openFileSuccessfully)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kExistingDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    ASSERT_NO_THROW({
        Parser parser(kMorphType, kExistingDataset, kFileFormat);
    });
}

TEST_P(ParserIPATest, openFileUnsuccessfully)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kNotExistingDataset = "./not_exist.txt";
    const FileFormat kFileFormat = GetParam().format;
    ASSERT_THROW({
        Parser parser(kMorphType, kNotExistingDataset, kFileFormat);
    }, std::runtime_error);
}

TEST_P(ParserIPATest, readEmptyRawValue)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_EQ("", parser.raw());
}

TEST_P(ParserIPATest, parseASentence)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    const std::string kFirstLine =
        "すくすくと育ち早いものであと３日で４ヶ月です。";
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_TRUE(parser.next());
    ASSERT_EQ(kFirstLine, parser.raw());
}

TEST_P(ParserIPATest, parseSentences)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    const std::string kFirstLine =
        "すくすくと育ち早いものであと３日で４ヶ月です。";
    const std::string kSecondLine =
        "元気に育ってくれてるのでそれだけで嬉しいです。";
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_TRUE(parser.next());
    ASSERT_EQ(kFirstLine, parser.raw());
    ASSERT_TRUE(parser.next());
    ASSERT_EQ(kSecondLine, parser.raw());
    ASSERT_FALSE(parser.next());
}

TEST_P(ParserIPATest, parseASentenceAsPhrases)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    const std::vector<std::string> kPhrases =
        {"すくすくと",
        "育ち早い",
        "もので",
        "あと",
        "３日で",
        "４ヶ月です。", };
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_TRUE(parser.next());

    const std::vector<Phrase>& parsed_phrases = parser.phrases();
    ASSERT_EQ(kPhrases.size(), parsed_phrases.size());
    for (int i = 0; i < kPhrases.size(); ++i)
        ASSERT_EQ(kPhrases[i], parsed_phrases[i].phrase());
}

// TEST_F(ParserIPATest, parseInParallel)
// {
//     const Morph::MORPH_TYPE kMorphType = Morph::MORPH_TYPE::JUMAN;
//     const std::string kLargeDataset =
//         "./unittest/dataset/parser_test_data1.JDP";
//     const long kNumOfLines = 10000;
//     const int kNumParser = 3;
//
//     std::vector<int> count(kNumParser);
//     std::vector<std::thread> threads;
//     for (int i = 0; i < kNumParser; ++i) {
//         threads.push_back(
//             std::thread([i, kLargeDataset, &count] {
//                 Parser parser(kMorphType, kLargeDataset);
//                 while (parser.next()) {
//                 }
//                 count[i] = parser.numberOfLines();
//             })
//         );
//     }
//     for (std::thread& th : threads) th.join();
//     for (int i = 0; i < kNumParser; ++i) ASSERT_EQ(kNumOfLines, count[i]);
// }

TEST_P(ParserIPATest, parseASentenceAsConnections)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    const std::vector<int> kConnections = {1, 2, 5, 4, 5, -1, };
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_TRUE(parser.next());

    const std::vector<int>& connections = parser.connections();
    ASSERT_EQ(kConnections.size(), connections.size());
    for (int i = 0; i < kConnections.size(); ++i) {
        ASSERT_EQ(kConnections[i], connections[i]);
    }
}

TEST_P(ParserIPATest, followConnections)
{
    const Morph::MORPH_TYPE kMorphType = GetParam().type;
    const std::string kSmallDataset = GetParam().filename;
    const FileFormat kFileFormat = GetParam().format;
    Parser parser(kMorphType, kSmallDataset, kFileFormat);
    ASSERT_TRUE(parser.next());

    const std::vector<int> kConnections[] =
        {{1, 2, 5, -1, },
         {2, 5, -1},
         {5, -1},
         {4, 5, -1},
         {5, -1},
         {-1}};
    const std::vector<int>& connections = parser.connections();
    for (int i = 0; i < connections.size(); ++i) {
        int index = connections[i];
        int j = 0;
        while (index != -1) {
            ASSERT_EQ(kConnections[i][j++], index);
            index = connections[index];
        }
    }
}

INSTANTIATE_TEST_CASE_P(
    TestWithSmallIPADataSet,
    ParserIPATest,
    ::testing::ValuesIn(kIPASmall));
