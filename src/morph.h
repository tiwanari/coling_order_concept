#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include "util/split.h"

namespace order_concepts {
/**
 * Morph information container class
 */
class Morph {
public:
   enum class MORPH_TYPE : unsigned char {
       JUMAN,
       IPADIC
   };
   enum class POS_TAG : unsigned char {
       NOUN,
       VERB,
       ADJECTIVE,
       AUXILIARY_VERB,
       OTHER
   };
protected:
    std::string m_morph;
    POS_TAG m_pos;
    std::vector<std::string> m_sub_poss;
    std::string m_ctype;
    std::string m_cform;
    std::string m_lemma;
public:
    virtual void init(const std::string& morph, const std::vector<std::string>& infos)
        throw (std::runtime_error) = 0;
    virtual ~Morph() {};
    const std::string& morph() const { return m_morph; }
    const POS_TAG& POS() const { return m_pos; }
    const std::vector<std::string>& subPOSs() const { return m_sub_poss; }
    const std::string& ctype() const { return m_ctype; }
    const std::string& cform() const { return m_cform; }
    const std::string& lemma() const { return m_lemma; }
    bool operator==(const Morph& morph) const;
    bool operator!=(const Morph& morph) const { return !(*this == morph); }
public:
    virtual bool isNegative() = 0;
    virtual POS_TAG POSFrom(const std::string& str) = 0;
};
} // namespace order_concepts
