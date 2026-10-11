#include "../../headers/domain/UsefulMineral.h"

void UsefulMineral::addDrySection(const DryMineralSection& section)
{
    drySections.push_back(section);
}

void UsefulMineral::addWaterBearingSection(const WaterBearingMineralSection& section)
{
    waterBearingSections.push_back(section);
}

const std::vector<DryMineralSection>& UsefulMineral::getDrySections() const
{
    return drySections;
}

const std::vector<WaterBearingMineralSection>& UsefulMineral::getWaterBearingSections() const
{
    return waterBearingSections;
}

double UsefulMineral::getDryThickness() const
{
    double thickness = 0.0;

    for (const DryMineralSection& section : drySections)
    {
        thickness += section.getThickness();
    }

    return thickness;
}

double UsefulMineral::getWaterBearingThickness() const
{
    double thickness = 0.0;

    for (const WaterBearingMineralSection& section : waterBearingSections)
    {
        thickness += section.getThickness();
    }

    return thickness;
}

double UsefulMineral::getTotalThickness() const
{
    return getDryThickness() + getWaterBearingThickness();
}

bool UsefulMineral::hasDrySections() const
{
    return !drySections.empty();
}

bool UsefulMineral::hasWaterBearingSections() const
{
    return !waterBearingSections.empty();
}

void UsefulMineral::clear()
{
    drySections.clear();
    waterBearingSections.clear();
}