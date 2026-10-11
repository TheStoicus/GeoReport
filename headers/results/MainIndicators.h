#pragma once

#include <string>

class MainIndicators
{
public:
    void setBoreholeNumber(const std::string& number);
    const std::string& getBoreholeNumber() const;

    void setBoreholeDepth(double value);
    double getBoreholeDepth() const;

    void setCollarElevation(double value);
    double getCollarElevation() const;

    void setOverburdenThickness(double value);
    double getOverburdenThickness() const;

    void setUsefulMineralTopDepth(double value);
    double getUsefulMineralTopDepth() const;

    void setUsefulMineralThickness(double value);
    double getUsefulMineralThickness() const;

    void setDryMineralThickness(double value);
    double getDryMineralThickness() const;

    void setWaterBearingMineralThickness(double value);
    double getWaterBearingMineralThickness() const;

    void setUsefulMineralBottomDepth(double value);
    double getUsefulMineralBottomDepth() const;

    void setUsefulMineralTopElevation(double value);
    double getUsefulMineralTopElevation() const;

    void setUsefulMineralBottomElevation(double value);
    double getUsefulMineralBottomElevation() const;

    void setDryMineralTopElevation(double value);
    double getDryMineralTopElevation() const;

    void setDryMineralBottomElevation(double value);
    double getDryMineralBottomElevation() const;

    void setWaterBearingMineralTopElevation(double value);
    double getWaterBearingMineralTopElevation() const;

    void setWaterBearingMineralBottomElevation(double value);
    double getWaterBearingMineralBottomElevation() const;

    void setGroundwaterDepth(double value);
    double getGroundwaterDepth() const;

    void setGroundwaterElevation(double value);
    double getGroundwaterElevation() const;

    void setUnderlyingRockThickness(double value);
    double getUnderlyingRockThickness() const;

    void setUnderlyingRockName(const std::string& name);
    const std::string& getUnderlyingRockName() const;

private:
    std::string boreholeNumber;
    double boreholeDepth = 0.0;
    double collarElevation = 0.0;
    double overburdenThickness = 0.0;
    double usefulMineralTopDepth = 0.0;
    double usefulMineralThickness = 0.0;
    double dryMineralThickness = 0.0;
    double waterBearingMineralThickness = 0.0;
    double usefulMineralBottomDepth = 0.0;
    double usefulMineralTopElevation = 0.0;
    double usefulMineralBottomElevation = 0.0;
    double dryMineralTopElevation = 0.0;
    double dryMineralBottomElevation = 0.0;
    double waterBearingMineralTopElevation = 0.0;
    double waterBearingMineralBottomElevation = 0.0;
    double groundwaterDepth = 0.0;
    double groundwaterElevation = 0.0;
    double underlyingRockThickness = 0.0;
    std::string underlyingRockName;
};
