#pragma once
#include <vector>
#include <memory>
#include "morph.h"

namespace order_concepts {
class Phrase {
private:
    std::vector<std::shared_ptr<Morph>> m_morphs;
    bool m_is_negative;
    int m_depend;
    Morph::MORPH_TYPE m_morph_type;
private:
    std::shared_ptr<Morph> createMorph(const std::string& infos);
public:
    Phrase() : m_is_negative(false), m_morph_type(Morph::MORPH_TYPE::JUMAN) {}
    Phrase(const std::vector<std::shared_ptr<Morph>>& morphs)
        : m_is_negative(false), m_morph_type(Morph::MORPH_TYPE::JUMAN) { m_morphs = morphs; }
    void setMorhpType(const Morph::MORPH_TYPE& type) { m_morph_type = type; }
    void add(std::shared_ptr<Morph> morph);
    void add(const std::string& infos);
    bool find(const std::string& lemma, Morph::POS_TAG pos) const;
    bool isNegative() const { return m_is_negative; }
    void clear();
    const std::vector<std::shared_ptr<Morph>>& morphs() const { return m_morphs; }
    std::string phrase() const;
    const int& depend() const { return m_depend; }
    void setDepend(int d) { m_depend = d; }
    bool operator==(const Phrase& phrase) const;
    bool operator!=(const Phrase& phrase) const { return !(*this == phrase); }
};
} // namespace order_concepts
