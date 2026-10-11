#include "../../headers/settings/DevelopmentSettings.h"

#include <algorithm>


void DevelopmentSettings::setMethod(
    DevelopmentMethod value)
{
    method = value;

    /*
     * Для экскаватора устанавливаем стандартное
     * ограничение 2.5 м обводнённого ПИ.
     *
     * Пользователь при необходимости может
     * изменить это значение вручную.
     */
    if (method == DevelopmentMethod::Excavator)
    {
        waterBearingMiningThickness = 2.5;
    }
}


DevelopmentMethod
DevelopmentSettings::getMethod() const
{
    return method;
}


void DevelopmentSettings::setWaterBearingMiningThickness(
    double thickness)
{
    waterBearingMiningThickness =
        std::max(
            0.0,
            thickness);
}


double
DevelopmentSettings::getWaterBearingMiningThickness() const
{
    return waterBearingMiningThickness;
}


void DevelopmentSettings::setDryProtectiveLayerThickness(
    double thickness)
{
    dryProtectiveLayerThickness =
        std::max(
            0.0,
            thickness);
}


double
DevelopmentSettings::getDryProtectiveLayerThickness() const
{
    return dryProtectiveLayerThickness;
}