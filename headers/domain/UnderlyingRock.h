#pragma once

#include "GeologicalSection.h"

#include <string>

class UnderlyingRock : public GeologicalSection
{
public:
    UnderlyingRock() = default;

    UnderlyingRock(double roofDepth, double bottomDepth, const std::string& name);

    void setName(const std::string& name);
    const std::string& getName() const;

private:
    std::string name;
};
