#pragma once

enum class UsefulMineralWaterMode
{
    DryAndWaterBearing,
    DryOnly
};

class GroundwaterSettings
{
public:
    void setUsefulMineralWaterMode(UsefulMineralWaterMode mode);
    void setDryLayerAboveGroundwater(double thickness);

    double getDryLayerAboveGroundwater() const;
    UsefulMineralWaterMode getUsefulMineralWaterMode() const;

private:
    UsefulMineralWaterMode usefulMineralWaterMode = UsefulMineralWaterMode::DryAndWaterBearing;

    /*
     * Толщина сухого материала, который оставляется над уровнем грунтовых вод.
     * Например:
     * 0.5 м
     * 0.8 м
     * 1.0 м
     */
    double dryLayerAboveGroundwater = 1.0;
};
