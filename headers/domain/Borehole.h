#pragma once

#include <string>
#include <vector>

#include "Sample.h"

enum class CoordinateSystem
{
    WGS84,
    SK1963
};

struct Coordinate
{
    double first = 0.0;
    double second = 0.0;
    CoordinateSystem system = CoordinateSystem::WGS84;
};

class Borehole
{
private:
    std::string boreholeNumber;
    int drillingYear = 0;
    std::vector<Sample> samples;

    double collarElevation = 0.0;
    double totalDepth = 0.0;
    double groundwaterDepth = 0.0;
    double groundwaterElevation = 0.0;

    Coordinate coordinate;

public:
    void setBoreholeNumber(const std::string& number);
    void setDrillingYear(int year);
    void setCollarElevation(double elevation);
    void setTotalDepth(double depth);
    void setGroundwaterDepth(double depth);
    void setGroundwaterElevation(double elevation);
    void setCoordinate(double first, double second, CoordinateSystem system);
    
    const std::string& getBoreholeNumber() const;
    int getDrillingYear() const;
    double getCollarElevation() const;
    double getTotalDepth() const;
    double getGroundwaterDepth() const;
    double getGroundwaterElevation() const;
    const std::vector<Sample>& getSamples() const;
    std::vector<Sample>& getSamplesModifiable();
    Coordinate getCoordinate() const;
    
    void addSample(const Sample& sample);

    double calculateBottomElevation() const;
};
