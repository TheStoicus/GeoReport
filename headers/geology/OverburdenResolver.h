#pragma once

#include "../calculations/BoreholeSectionAnalysis.h"
#include "../domain/Overburden.h"

#include <vector>

class OverburdenResolver
{
public:
    std::vector<Overburden> resolve(const BoreholeSectionAnalysis& analysis) const;

private:
    Overburden createOverburden(const BoreholeSection& section) const;
    OverburdenType getOverburdenType(const BoreholeSection& section) const;
};
