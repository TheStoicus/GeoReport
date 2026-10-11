#include "../../headers/domain/UnderlyingRock.h"

UnderlyingRock::UnderlyingRock(double roofDepth, double bottomDepth, const std::string& name)
    : GeologicalSection(roofDepth, bottomDepth), name(name) {}

void UnderlyingRock::setName(const std::string& newName)
{
    name = newName;
}

const std::string& UnderlyingRock::getName() const
{
    return name;
}