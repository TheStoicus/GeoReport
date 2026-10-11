#include "../../headers/domain/Borehole.h"

void Borehole::setBoreholeNumber(const std::string& number)
{
    boreholeNumber = number;
}

void Borehole::setDrillingYear(int year)
{
    drillingYear = year;
}

int Borehole::getDrillingYear() const
{
    return drillingYear;
}

void Borehole::setCollarElevation(double elevation)
{
    collarElevation = elevation;

    if (groundwaterDepth > 0.0)
    {
        groundwaterElevation = collarElevation - groundwaterDepth;
    }
    else
    {
        groundwaterElevation = 0.0;
    }
}

void Borehole::setTotalDepth(double depth)
{
    totalDepth = depth;
}

void Borehole::setGroundwaterDepth(double depth)
{
    groundwaterDepth = depth;

    if (groundwaterDepth > 0.0)
    {
        groundwaterElevation = collarElevation - groundwaterDepth;
    }
    else
    {
        groundwaterElevation = 0.0;
    }
}

void Borehole::setGroundwaterElevation(double elevation)
{
    groundwaterElevation = elevation;
}

void Borehole::setCoordinate(double first, double second, CoordinateSystem system)
{
    coordinate.first = first;
    coordinate.second = second;
    coordinate.system = system;
}

const std::string& Borehole::getBoreholeNumber() const
{
    return boreholeNumber;
}

double Borehole::getCollarElevation() const
{
    return collarElevation;
}

double Borehole::getTotalDepth() const
{
    return totalDepth;
}

double Borehole::getGroundwaterDepth() const
{
    return groundwaterDepth;
}

double Borehole::getGroundwaterElevation() const
{
    return groundwaterElevation;
}

const std::vector<Sample>& Borehole::getSamples() const
{
    return samples;
}

std::vector<Sample>& Borehole::getSamplesModifiable()
{
    return samples;
}

Coordinate Borehole::getCoordinate() const
{
    return coordinate;
}

void Borehole::addSample(const Sample& sample)
{
    samples.push_back(sample);
}

double Borehole::calculateBottomElevation() const
{
    return collarElevation - totalDepth;
}
