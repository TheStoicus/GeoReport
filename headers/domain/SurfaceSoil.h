#pragma once

#include "GeologicalSection.h"

class SurfaceSoil : public GeologicalSection
{
public:
    SurfaceSoil() = default;

    SurfaceSoil(double roofDepth, double bottomDepth);
};
