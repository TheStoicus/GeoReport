#pragma once

#include "../domain/BoreholeSection.h"
#include "SectionDecision.h"

#include <vector>

class BoreholeSectionResolver
{
public:
    FinalBoreholeSections resolve(
        const SectionClassificationResult& classification,
        const std::vector<SectionDecision>& decisions) const;

private:
    const SectionDecision* findDecision(
        const BoreholeSection& ambiguousSection,
        const std::vector<SectionDecision>& decisions) const;
};
