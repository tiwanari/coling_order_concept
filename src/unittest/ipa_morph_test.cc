#include <gtest/gtest.h>
#include <string>
#include "../ipa_morph.h"

using namespace order_concepts;

class IPAMorphTest : public ::testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(IPAMorphTest, createAMorphWithNotEnoughParameters)
{
    const std::string kInputText =
        "まし	助動詞,*,*,*,特殊・マス,連用形";

    ASSERT_THROW({
        IPAMorph morph(kInputText);
    }, std::runtime_error);
}

TEST_F(IPAMorphTest, createAMorphFromASentence)
{
    const std::string kInputText =
        "まし	助動詞,*,*,*,特殊・マス,連用形,ます,マシ,マシ";
    IPAMorph morph(kInputText);

    ASSERT_EQ(morph.morph(), "まし");
    ASSERT_EQ(morph.POS(), Morph::POS_TAG::AUXILIARY_VERB);
    ASSERT_EQ(morph.subPOSs(), std::vector<std::string>({"*","*","*",}));
    ASSERT_EQ(morph.ctype(), "特殊・マス");
    ASSERT_EQ(morph.cform(), "連用形");
    ASSERT_EQ(morph.lemma(), "ます");
}

TEST_F(IPAMorphTest, createAMorphFromSentences)
{
    const std::string kMorph = "まし";
    const std::string kInfos =
        "助動詞,*,*,*,特殊・マス,連用形,ます,マシ,マシ";
    IPAMorph morph(kMorph, kInfos);

    ASSERT_EQ(morph.morph(), "まし");
    ASSERT_EQ(morph.POS(), Morph::POS_TAG::AUXILIARY_VERB);
    ASSERT_EQ(morph.subPOSs(), std::vector<std::string>({"*","*","*",}));
    ASSERT_EQ(morph.ctype(), "特殊・マス");
    ASSERT_EQ(morph.cform(), "連用形");
    ASSERT_EQ(morph.lemma(), "ます");
}

TEST_F(IPAMorphTest, createAMorphFromASentenceAndAVector)
{
    const std::string kMorph = "まし";
    const std::string kInfos =
        "助動詞,*,*,*,特殊・マス,連用形,ます,マシ,マシ";
    std::vector<std::string> infos;
    util::splitStringUsing(kInfos, ",", &infos);
    IPAMorph morph(kMorph, infos);

    ASSERT_EQ(morph.morph(), "まし");
    ASSERT_EQ(morph.POS(), Morph::POS_TAG::AUXILIARY_VERB);
    ASSERT_EQ(morph.subPOSs(), std::vector<std::string>({"*","*","*",}));
    ASSERT_EQ(morph.ctype(), "特殊・マス");
    ASSERT_EQ(morph.cform(), "連用形");
    ASSERT_EQ(morph.lemma(), "ます");
}

TEST_F(IPAMorphTest, createANounMorphFromASentence)
{
    const std::string kInputText =
        "切り返し	名詞,一般,*,*,*,*,切り返し,キリカエシ,キリカエシ";
    IPAMorph morph(kInputText);

    ASSERT_EQ(morph.morph(), "切り返し");
    ASSERT_EQ(morph.POS(), Morph::POS_TAG::NOUN);
    ASSERT_EQ(morph.subPOSs(), std::vector<std::string>({"一般","*","*",}));
    ASSERT_EQ(morph.ctype(), "*");
    ASSERT_EQ(morph.cform(), "*");
    ASSERT_EQ(morph.lemma(), "切り返し");
}

TEST_F(IPAMorphTest, createAnAdjectiveMorphFromASentence)
{
    const std::string kInputText =
        "大きい	形容詞,自立,*,*,形容詞・イ段,基本形,大きい,オオキイ,オーキイ";
    IPAMorph morph(kInputText);

    ASSERT_EQ(morph.morph(), "大きい");
    ASSERT_EQ(morph.POS(), Morph::POS_TAG::ADJECTIVE);
    ASSERT_EQ(morph.subPOSs(), std::vector<std::string>({"自立","*","*",}));
    ASSERT_EQ(morph.ctype(), "形容詞・イ段");
    ASSERT_EQ(morph.cform(), "基本形");
    ASSERT_EQ(morph.lemma(), "大きい");
}
