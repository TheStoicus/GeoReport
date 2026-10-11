#pragma once

#include "../domain/Borehole.h"
#include "../geology/BoreholeGeology.h"
#include "GroundwaterAnalysis.h"
#include "../results/MainIndicators.h"

class MainIndicatorsCalculator
{
public:
    MainIndicators calculate(const Borehole& borehole, const BoreholeGeology& geology,
        const GroundwaterAnalysis& groundwaterAnalysis) const;
};