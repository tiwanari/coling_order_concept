#include <iostream>
#include <stdexcept>
#include "juman_morph.h"

namespace order_concepts {
void JumanMorph::init(
    const std::string& morph,
    const std::vector<std::string>& infos)
throw (std::runtime_error)
{
    if (infos.size() < 5) {
        std::stringstream ss;
        for (const auto& info : infos)
            ss << info << ", ";
        ss << "size: " << infos.size() << std::endl;
        throw std::runtime_error(
            "Not enough params for juman morph (" + morph + "): " + ss.str());
    }
    m_morph     = morph;
    m_pos       = POSFrom(infos[0]);
    m_sub_poss  = std::vector<std::string>({infos[1]});
    m_ctype     = infos[2];
    m_cform     = infos[3];
    m_lemma     = infos[4];
}

JumanMorph::JumanMorph(const std::string& infos)
{
    std::vector<std::string> splitted_line;
    util::splitStringUsing(infos, "\t", &splitted_line);
    if (splitted_line.size() != 2) {
        std::stringstream ss;
        ss << "Invalid params for morph (size: " << splitted_line.size()
            << "): " << infos << std::endl;
        throw std::runtime_error(ss.str());
    }
    std::vector<std::string> morph_infos;
    util::splitStringUsing(splitted_line[1], ",", &morph_infos);
    init(splitted_line[0], morph_infos);
}

JumanMorph::JumanMorph(
    const std::string& morph,
    const std::string& infos)
{
    std::vector<std::string> morph_infos;
    util::splitStringUsing(infos, ",", &morph_infos);
    init(morph, morph_infos);
}

JumanMorph::JumanMorph(
    const std::string& morph,
    const std::vector<std::string>& infos)
{
    init(morph, infos);
}

Morph::POS_TAG JumanMorph::POSFrom(const std::string& str)
{
    if (str == STR_POS_NOUN) {
        return POS_TAG::NOUN;
    }
    else if (str == STR_POS_VERB) {
        return POS_TAG::VERB;
    }
    else if (str == STR_POS_ADJECTIVE) {
        return POS_TAG::ADJECTIVE;
    }
    else if (str == STR_POS_AUXILIARY_VERB) {
        return POS_TAG::AUXILIARY_VERB;
    }
    return POS_TAG::OTHER;
};
} // namespace order_concepts
