#include <iostream>
#include <stdexcept>
#include "morph.h"

namespace order_concepts {
bool Morph::operator==(const Morph& morph) const
{
    if (this->morph() != morph.morph()) return false;
    if (this->POS() != morph.POS()) return false;
    if (this->subPOSs() != morph.subPOSs()) return false;
    if (this->ctype() != morph.ctype()) return false;
    if (this->cform() != morph.cform()) return false;
    if (this->lemma() != morph.lemma()) return false;
    return true;
}
} // namespace order_concepts
