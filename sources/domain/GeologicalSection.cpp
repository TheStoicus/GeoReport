#include "../../headers/domain/GeologicalSection.h"

GeologicalSection::GeologicalSection(double roofDepth, double bottomDepth)
    : roofDepth(roofDepth), bottomDepth(bottomDepth) {}

void GeologicalSection::setRoofDepth(double depth)
{
    roofDepth = depth;
}

void GeologicalSection::setBottomDepth(double depth)
{
    bottomDepth = depth;
}

double GeologicalSection::getRoofDepth() const
{
    return roofDepth;
}

double GeologicalSection::getBottomDepth() const
{
    return bottomDepth;
}

double GeologicalSection::getThickness() const
{
    return bottomDepth - roofDepth;
}