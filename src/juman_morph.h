#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include "util/split.h"
#include "morph.h"

namespace order_concepts {
/**
 * The items in this class are based on
 * Juman dictionary:
 * see http://nlp.ist.i.kyoto-u.ac.jp/index.php?cmd=read&page=JUMAN
 */
class JumanMorph : public Morph {
    typedef Morph inherited;
private:
    static constexpr const char* STR_POS_NOUN = "名詞";
    static constexpr const char* STR_POS_VERB = "動詞";
    static constexpr const char* STR_POS_ADJECTIVE = "形容詞";
    static constexpr const char* STR_POS_AUXILIARY_VERB = "助動詞";
public:
    void init(const std::string& morph, const std::vector<std::string>& infos)
        throw (std::runtime_error);
    JumanMorph(const std::string& infos);
    JumanMorph(const std::string& morph, const std::string& infos);
    JumanMorph(const std::string& morph, const std::vector<std::string>& infos);
    ~JumanMorph() {};
public:
    virtual inline bool isNegative() {
        return *this == JumanMorph("ない",
                        "接尾辞,形容詞性述語接尾辞,イ形容詞アウオ段,基本形,ない");
    }
    virtual POS_TAG POSFrom(const std::string& str);
};
} // namespace order_concepts
