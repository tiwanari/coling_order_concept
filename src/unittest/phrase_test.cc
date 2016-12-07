#include <gtest/gtest.h>
#include <string>
#include <memory>
#include "../phrase.h"
#include "../juman_morph.h"
#include "../ipa_morph.h"

using namespace order_concepts;

class PhraseTest : public ::testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(PhraseTest, createAPhraseUsingVector)
{
    const std::string kInputText0 =
        "切り返し	名詞,普通名詞,*,*,切り返し,未知語";
    const std::shared_ptr<JumanMorph> kMorph0(new JumanMorph(kInputText0));
    const std::string kInputText1 =
        "ました	接尾辞,動詞性接尾辞,"
        "動詞性接尾辞ます型,タ形,ます,既知語";
    const std::shared_ptr<JumanMorph> kMorph1(new JumanMorph(kInputText1));
    const std::vector<std::shared_ptr<Morph>> kMorphs = {kMorph0, kMorph1};
    Phrase phrase(kMorphs);

    ASSERT_EQ(kMorph0->morph() + kMorph1->morph(), phrase.phrase());
    ASSERT_EQ("切り返しました", phrase.phrase());

    const std::vector<std::shared_ptr<Morph>>& morphs = phrase.morphs();
    ASSERT_EQ(kMorph0, morphs[0]);
    ASSERT_EQ(kMorph1, morphs[1]);
}

TEST_F(PhraseTest, createAPhraseUsingAddFunction)
{
    const std::string kInputText0 =
        "切り返し	名詞,普通名詞,*,*,切り返し,未知語";
    const std::shared_ptr<JumanMorph> kMorph0(new JumanMorph(kInputText0));
    const std::string kInputText1 =
        "ました	接尾辞,動詞性接尾辞,"
        "動詞性接尾辞ます型,タ形,ます,既知語";
    const std::shared_ptr<JumanMorph> kMorph1(new JumanMorph(kInputText1));

    Phrase phrase;
    phrase.add(kMorph0);
    phrase.add(kMorph1);

    ASSERT_EQ(kMorph0->morph() + kMorph1->morph(), phrase.phrase());
    ASSERT_EQ("切り返しました", phrase.phrase());

    const std::vector<std::shared_ptr<Morph>>& morphs = phrase.morphs();
    ASSERT_EQ(kMorph0, morphs[0]);
    ASSERT_EQ(kMorph1, morphs[1]);
}

TEST_F(PhraseTest, clearPhrase)
{
    const std::string kInputText0 =
        "切り返し	名詞,普通名詞,*,*,切り返し,未知語";
    const std::shared_ptr<JumanMorph> kMorph0(new JumanMorph(kInputText0));
    const std::string kInputText1 =
        "ました	接尾辞,動詞性接尾辞,"
        "動詞性接尾辞ます型,タ形,ます,既知語";
    const std::shared_ptr<JumanMorph> kMorph1(new JumanMorph(kInputText1));

    Phrase phrase;
    phrase.add(kMorph0);
    phrase.add(kMorph1);
    phrase.clear();

    ASSERT_EQ("", phrase.phrase());
    ASSERT_EQ(0, phrase.morphs().size());
}

TEST_F(PhraseTest, creatANegativePhrase)
{
    const std::string kInputText0 =
        "大きく	形容詞,*,イ形容詞イ段特殊,基本連用形,大きい,既知語";
    const std::shared_ptr<JumanMorph> kMorph0(new JumanMorph(kInputText0));
    const std::string kInputText1 =
        "ない	接尾辞,形容詞性述語接尾辞,イ形容詞アウオ段,基本形,ない,既知語";
    const std::shared_ptr<JumanMorph> kMorph1(new JumanMorph(kInputText1));

    Phrase phrase;
    phrase.add(kMorph0);
    ASSERT_FALSE(phrase.isNegative());
    phrase.add(kMorph1);
    ASSERT_TRUE(phrase.isNegative());
}
