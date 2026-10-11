#include "../../headers/domain/Overburden.h"

Overburden::Overburden(double roofDepth, double bottomDepth, OverburdenType type)
    : GeologicalSection(roofDepth, bottomDepth), type(type) {}

void Overburden::setType(OverburdenType newType)
{
    type = newType;
}

OverburdenType Overburden::getType() const
{
    return type;
}