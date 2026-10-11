#pragma once

#include "../calculations/BoreholeSectionAnalysis.h"
#include "../domain/SurfaceSoil.h"

#include <optional>

class SurfaceSoilResolver
{
public:
    std::optional<SurfaceSoil> resolve(const BoreholeSectionAnalysis& analysis) const;
};
