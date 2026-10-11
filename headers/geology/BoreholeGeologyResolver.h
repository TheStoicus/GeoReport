#pragma once

#include "../domain/Borehole.h"
#include "BoreholeGeology.h"

#include "../calculations/BoreholeSectionAnalysis.h"
#include "../calculations/GroundwaterAnalysis.h"

#include "../settings/GroundwaterSettings.h"
#include "../settings/DevelopmentSettings.h"

#include "OverburdenResolver.h"
#include "SurfaceSoilResolver.h"
#include "UnderlyingRockResolver.h"
#include "UsefulMineralResolver.h"


class BoreholeGeologyResolver
{
public:

    BoreholeGeology resolve(
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const GroundwaterAnalysis& groundwaterAnalysis,
        const GroundwaterSettings& groundwaterSettings,
        const DevelopmentSettings& developmentSettings) const;


private:

    SurfaceSoilResolver surfaceSoilResolver;

    OverburdenResolver overburdenResolver;

    UsefulMineralResolver usefulMineralResolver;

    UnderlyingRockResolver underlyingRockResolver;
};
