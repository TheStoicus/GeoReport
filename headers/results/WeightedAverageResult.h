#pragma once

#include <map>
#include <string>

class WeightedAverageResult
{
private:
    std::map<std::string, double> values;

public:
    void setValue(const std::string& parameterName, double value);
    double getValue(const std::string& parameterName) const;

    const std::map<std::string, double>& getValues() const;
};
