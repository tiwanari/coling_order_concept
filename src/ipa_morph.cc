#include <iostream>
#include <stdexcept>
#include "ipa_morph.h"

namespace order_concepts {
void IPAMorph::init(
    const std::string& morph,
    const std::vector<std::string>& infos)
throw (std::runtime_error)
{
    if (infos.size() < 7) {
        std::stringstream ss;
        for (const auto& info : infos) {
            ss << info << ", ";
        }
        ss << "size: " << infos.size() << std::endl;
        throw std::runtime_error(
            "Not enough params for ipa morph (" + morph + "): " + ss.str());
    }
    m_morph     = morph;
    m_pos       = POSFrom(infos[0]);
    m_sub_poss  = std::vector<std::string>({infos[1], infos[2], infos[3]});
    m_ctype     = infos[4];
    m_cform     = infos[5];
    m_lemma     = infos[6];
}

IPAMorph::IPAMorph(const std::string& infos)
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

IPAMorph::IPAMorph(
    const std::string& morph,
    const std::string& infos)
{
    std::vector<std::string> morph_infos;
    util::splitStringUsing(infos, ",", &morph_infos);
    init(morph, morph_infos);
}

IPAMorph::IPAMorph(
    const std::string& morph,
    const std::vector<std::string>& infos)
{
    init(morph, infos);
}

std::shared_ptr<Morph> IPAMorph::createAdjectiveMorph(
    const std::shared_ptr<Morph> noun,
    const std::shared_ptr<Morph> aux_verb)
{
    // merge morphs [noun (the stem of adjective verb) + auxiliary verb]
    // they should be IPAMorphs
    std::stringstream new_morph_infos;
    new_morph_infos
        << noun->morph() << aux_verb->morph() << "\t"
        << STR_POS_ADJECTIVE << ",*,*,*,*,*,"
        << noun->lemma() << "だ"; // aux_verb->lemma();
    return std::make_shared<IPAMorph>(new_morph_infos.str());
}

Morph::POS_TAG IPAMorph::POSFrom(const std::string& str)
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
