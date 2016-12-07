#include <gtest/gtest.h>
#include <string>
#include "../juman_morph.h"

using namespace order_concepts;

class JumanMorphTest : public ::testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(JumanMorphTest, createAMorphWithNotEnoughParameters)
{
    const std::string kInputText =
        "ました	接尾辞,動詞性接尾辞,"
        "動詞性接尾辞ます型,タ形";

    ASSERT_THROW({
        JumanMorph morph(kInputText);
    }, std::runtime_error);
}

TEST_F(JumanMorphTest, createAMorphFromASentence)
{
    const std::string kInputText =
        "ました	接尾辞,動詞性接尾辞,"
        "動詞性接尾辞ます型,タ形,ます,既知語";
    JumanMorph morph(kInputText);

    ASSERT_EQ(morph.morph(), "ました");
    ASSERT_EQ(morph.POS(), Morph::POS_TAG::OTHER);
    ASSERT_EQ(morph.subPOSs(), std::vector<std::string>({"動詞性接尾辞"}));
    ASSERT_EQ(morph.ctype(), "動詞性接尾辞ます型");
    ASSERT_EQ(morph.cform(), "タ形");
    ASSERT_EQ(morph.lemma(), "ます");
}

TEST_F(JumanMorphTest, createAMorphFromSentences)
{
    const std::string kMorph = "ました";
    const std::string kInfos =
        "接尾辞,動詞性接尾辞,"
        "動詞性接尾辞ます型,タ形,ます,既知語";
    JumanMorph morph(kMorph, kInfos);

    ASSERT_EQ(morph.morph(), "ました");
    ASSERT_EQ(morph.POS(), Morph::POS_TAG::OTHER);
    ASSERT_EQ(morph.subPOSs(), std::vector<std::string>({"動詞性接尾辞"}));
    ASSERT_EQ(morph.ctype(), "動詞性接尾辞ます型");
    ASSERT_EQ(morph.cform(), "タ形");
    ASSERT_EQ(morph.lemma(), "ます");
}

TEST_F(JumanMorphTest, createAMorphFromASentenceAndAVector)
{
    const std::string kMorph = "ました";
    const std::string kInfos =
        "接尾辞,動詞性接尾辞,"
        "動詞性接尾辞ます型,タ形,ます,既知語";
    std::vector<std::string> infos;
    util::splitStringUsing(kInfos, ",", &infos);
    JumanMorph morph(kMorph, infos);

    ASSERT_EQ(morph.morph(), "ました");
    ASSERT_EQ(morph.POS(), Morph::POS_TAG::OTHER);
    ASSERT_EQ(morph.subPOSs(), std::vector<std::string>({"動詞性接尾辞"}));
    ASSERT_EQ(morph.ctype(), "動詞性接尾辞ます型");
    ASSERT_EQ(morph.cform(), "タ形");
    ASSERT_EQ(morph.lemma(), "ます");
}

TEST_F(JumanMorphTest, createANounMorphFromASentence)
{
    const std::string kInputText =
        "切り返し	名詞,普通名詞,*,*,切り返し,未知語";
    JumanMorph morph(kInputText);

    ASSERT_EQ(morph.morph(), "切り返し");
    ASSERT_EQ(morph.POS(), Morph::POS_TAG::NOUN);
    ASSERT_EQ(morph.subPOSs(), std::vector<std::string>({"普通名詞"}));
    ASSERT_EQ(morph.ctype(), "*");
    ASSERT_EQ(morph.cform(), "*");
    ASSERT_EQ(morph.lemma(), "切り返し");
}

TEST_F(JumanMorphTest, createAnAdjectiveMorphFromASentence)
{
    const std::string kInputText =
        "大きい	形容詞,*,イ形容詞イ段特殊,"
        "基本形,大きい,おおきい,代表表記:大きい/おおきい 反義:形容詞:小さい/ちいさい";
    JumanMorph morph(kInputText);

    ASSERT_EQ(morph.morph(), "大きい");
    ASSERT_EQ(morph.POS(), Morph::POS_TAG::ADJECTIVE);
    ASSERT_EQ(morph.subPOSs(), std::vector<std::string>({"*"}));
    ASSERT_EQ(morph.ctype(), "イ形容詞イ段特殊");
    ASSERT_EQ(morph.cform(), "基本形");
    ASSERT_EQ(morph.lemma(), "大きい");
}
