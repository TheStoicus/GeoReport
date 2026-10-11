#pragma once

#include "GeologicalSection.h"

enum class OverburdenType
{
    Surface,
    Internal
};

class Overburden : public GeologicalSection
{
public:
    Overburden() = default;

    Overburden(double roofDepth, double bottomDepth, OverburdenType type);

    void setType(OverburdenType type);
    OverburdenType getType() const;

private:
    OverburdenType type = OverburdenType::Surface;
};
