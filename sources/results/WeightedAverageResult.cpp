#include "../../headers/results/WeightedAverageResult.h"

void WeightedAverageResult::setValue(const std::string& parameterName, double value)
{
    values[parameterName] = value;
}

double WeightedAverageResult::getValue(const std::string& parameterName) const
{
    const auto it = values.find(parameterName);

    if (it == values.end())
        return 0.0;

    return it->second;
}

const std::map<std::string, double>& WeightedAverageResult::getValues() const
{
    return values;
}
