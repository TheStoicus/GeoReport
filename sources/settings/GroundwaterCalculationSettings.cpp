#include "../../headers/settings/GroundwaterCalculationSettings.h"

#include <algorithm>


void GroundwaterCalculationSettings::setMode(
    GroundwaterCalculationMode value)
{
    mode = value;
}


GroundwaterCalculationMode
GroundwaterCalculationSettings::getMode() const
{
    return mode;
}


void GroundwaterCalculationSettings::setMajorityThreshold(
    double threshold)
{
    majorityThreshold =
        std::clamp(
            threshold,
            0.0,
            1.0);
}


double
GroundwaterCalculationSettings::getMajorityThreshold() const
{
    return majorityThreshold;
}