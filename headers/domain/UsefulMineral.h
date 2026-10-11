#pragma once

#include "DryMineralSection.h"
#include "WaterBearingMineralSection.h"

#include <vector>

class UsefulMineral
{
public:
    void addDrySection(const DryMineralSection& section);
    void addWaterBearingSection(const WaterBearingMineralSection& section);

    const std::vector<DryMineralSection>& getDrySections() const;
    const std::vector<WaterBearingMineralSection>& getWaterBearingSections() const;
    double getDryThickness() const;
    double getWaterBearingThickness() const;
    double getTotalThickness() const;

    bool hasDrySections() const;
    bool hasWaterBearingSections() const;

    void clear();

private:
    std::vector<DryMineralSection> drySections;
    std::vector<WaterBearingMineralSection> waterBearingSections;
};
