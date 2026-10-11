#pragma once

#include "GeologicalSection.h"

class DryMineralSection : public GeologicalSection
{
public:
    DryMineralSection() = default;

    DryMineralSection(double roofDepth, double bottomDepth);
};
