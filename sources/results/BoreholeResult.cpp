#include "../../headers/results/BoreholeResult.h"

void BoreholeResult::setBoreholeNumber(const std::string& number)
{
    boreholeNumber = number;
}

const std::string& BoreholeResult::getBoreholeNumber() const
{
    return boreholeNumber;
}

void BoreholeResult::setTotalThickness(double thickness)
{
    totalThickness = thickness;
}

double BoreholeResult::getTotalThickness() const
{
    return totalThickness;
}

void BoreholeResult::setWeightedAverage(const WeightedAverageResult& result)
{
    weightedAverage = result;
}

const WeightedAverageResult& BoreholeResult::getWeightedAverage() const
{
    return weightedAverage;
}

void BoreholeResult::setParameterThickness(const std::string& parameterName, double thickness)
{
    parameterThickness[parameterName] = thickness;
}

double BoreholeResult::getParameterThickness(const std::string& parameterName) const
{
    const auto it = parameterThickness.find(parameterName);

    if (it == parameterThickness.end())
        return 0.0;

    return it->second;
}

const std::map<std::string, double>& BoreholeResult::getParameterThicknesses() const
{
    return parameterThickness;
}
