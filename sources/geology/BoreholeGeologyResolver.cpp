#include "../../headers/geology/BoreholeGeologyResolver.h"

BoreholeGeology BoreholeGeologyResolver::resolve(
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const GroundwaterAnalysis& groundwaterAnalysis,
    const GroundwaterSettings& groundwaterSettings,
    const DevelopmentSettings& developmentSettings) const
{
    BoreholeGeology result;

    const auto surfaceSoil = surfaceSoilResolver.resolve(analysis);
    if (surfaceSoil.has_value())
        result.setSurfaceSoil(*surfaceSoil);

    result.setOverburden(
        overburdenResolver.resolve(analysis));

    UsefulMineral usefulMineral =
        usefulMineralResolver.resolve(
            borehole,
            analysis,
            groundwaterAnalysis,
            groundwaterSettings,
            developmentSettings);

    result.setUsefulMineral(usefulMineral);

    result.setUnderlyingRock(
        underlyingRockResolver.resolve(
            borehole,
            analysis,
            usefulMineral));

    return result;
}
