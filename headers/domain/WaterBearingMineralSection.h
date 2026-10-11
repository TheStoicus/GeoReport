#pragma once

#include "GeologicalSection.h"

class WaterBearingMineralSection : public GeologicalSection
{
public:
    WaterBearingMineralSection() = default;

    WaterBearingMineralSection(double roofDepth, double bottomDepth);
};
