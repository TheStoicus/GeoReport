#include "../../headers/settings/GroundwaterSettings.h"

void GroundwaterSettings::setUsefulMineralWaterMode(
    UsefulMineralWaterMode mode)
{
    usefulMineralWaterMode = mode;
}

UsefulMineralWaterMode
GroundwaterSettings::getUsefulMineralWaterMode() const
{
    return usefulMineralWaterMode;
}

void GroundwaterSettings::setDryLayerAboveGroundwater(
    double thickness)
{
    if (thickness < 0.0)
    {
        return;
    }

    dryLayerAboveGroundwater = thickness;
}

double GroundwaterSettings::getDryLayerAboveGroundwater() const
{
    return dryLayerAboveGroundwater;
}