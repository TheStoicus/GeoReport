#pragma once

#include "../domain/Borehole.h"
#include "../calculations/BoreholeSectionAnalysis.h"
#include "../calculations/GroundwaterAnalysis.h"
#include "../settings/GroundwaterSettings.h"
#include "../settings/DevelopmentSettings.h"
#include "../domain/UsefulMineral.h"


class UsefulMineralResolver
{
public:

    UsefulMineral resolve(
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const GroundwaterAnalysis& groundwaterAnalysis,
        const GroundwaterSettings& groundwaterSettings,
        const DevelopmentSettings& developmentSettings) const;


private:

    void addUsefulSection(
        UsefulMineral& result,
        double startDepth,
        double endDepth,
        double groundwaterDepth,
        const GroundwaterSettings& groundwaterSettings,
        const DevelopmentSettings& developmentSettings) const;


    double getDryBoundaryDepth(
        double groundwaterDepth,
        const GroundwaterSettings& settings) const;


    void addWaterBearingSection(
        UsefulMineral& result,
        double startDepth,
        double endDepth,
        double groundwaterDepth,
        const DevelopmentSettings& developmentSettings) const;
};
