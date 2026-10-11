#include "../../headers/calculations/MainIndicatorsCalculator.h"

#include <algorithm>

MainIndicators MainIndicatorsCalculator::calculate(
    const Borehole& borehole,
    const BoreholeGeology& geology,
    const GroundwaterAnalysis& groundwaterAnalysis) const
{
    MainIndicators result;

    result.setBoreholeNumber(borehole.getBoreholeNumber());
    result.setBoreholeDepth(borehole.getTotalDepth());
    result.setCollarElevation(borehole.getCollarElevation());
    result.setOverburdenThickness(geology.getTotalOverburdenThickness());

    const UsefulMineral& useful = geology.getUsefulMineral();

    result.setUsefulMineralThickness(useful.getTotalThickness());
    result.setDryMineralThickness(useful.getDryThickness());
    result.setWaterBearingMineralThickness(useful.getWaterBearingThickness());

    bool hasUseful = false;
    double topDepth = 0.0;
    double bottomDepth = 0.0;

    for (const DryMineralSection& section : useful.getDrySections())
    {
        if (!hasUseful)
        {
            topDepth = section.getRoofDepth();
            bottomDepth = section.getBottomDepth();
            hasUseful = true;
        }
        else
        {
            topDepth = std::min(topDepth, section.getRoofDepth());
            bottomDepth = std::max(bottomDepth, section.getBottomDepth());
        }
    }

    for (const WaterBearingMineralSection& section : useful.getWaterBearingSections())
    {
        if (!hasUseful)
        {
            topDepth = section.getRoofDepth();
            bottomDepth = section.getBottomDepth();
            hasUseful = true;
        }
        else
        {
            topDepth = std::min(topDepth, section.getRoofDepth());
            bottomDepth = std::max(bottomDepth, section.getBottomDepth());
        }
    }

    if (hasUseful)
    {
        result.setUsefulMineralTopDepth(topDepth);
        result.setUsefulMineralBottomDepth(bottomDepth);
        result.setUsefulMineralTopElevation(
            borehole.getCollarElevation() - topDepth);
        result.setUsefulMineralBottomElevation(
            borehole.getCollarElevation() - bottomDepth);
    }

    bool hasDry = false;
    double dryTopDepth = 0.0;
    double dryBottomDepth = 0.0;
    for (const DryMineralSection& section : useful.getDrySections())
    {
        if (!hasDry)
        {
            dryTopDepth = section.getRoofDepth();
            dryBottomDepth = section.getBottomDepth();
            hasDry = true;
        }
        else
        {
            dryTopDepth = std::min(dryTopDepth, section.getRoofDepth());
            dryBottomDepth = std::max(dryBottomDepth, section.getBottomDepth());
        }
    }

    if (hasDry)
    {
        result.setDryMineralTopElevation(
            borehole.getCollarElevation() - dryTopDepth);
        result.setDryMineralBottomElevation(
            borehole.getCollarElevation() - dryBottomDepth);
    }

    bool hasWater = false;
    double waterTopDepth = 0.0;
    double waterBottomDepth = 0.0;
    for (const WaterBearingMineralSection& section : useful.getWaterBearingSections())
    {
        if (!hasWater)
        {
            waterTopDepth = section.getRoofDepth();
            waterBottomDepth = section.getBottomDepth();
            hasWater = true;
        }
        else
        {
            waterTopDepth = std::min(waterTopDepth, section.getRoofDepth());
            waterBottomDepth = std::max(waterBottomDepth, section.getBottomDepth());
        }
    }

    if (hasWater)
    {
        result.setWaterBearingMineralTopElevation(
            borehole.getCollarElevation() - waterTopDepth);
        result.setWaterBearingMineralBottomElevation(
            borehole.getCollarElevation() - waterBottomDepth);
    }

    /*
     * В таблице по скважинам всегда показываем ФАКТИЧЕСКИЙ УГВ,
     * записанный для данной скважины в исходном Excel.
     *
     * Это намеренно отличается от расчётного УГВ, который используется
     * UsefulMineralResolver. Если на месторождении принят средний УГВ,
     * он влияет на расчёт границы сухого/обводнённого ПИ, но не должен
     * подменять фактическое значение в паспорте скважины.
     */
    result.setGroundwaterDepth(
        borehole.getGroundwaterDepth());
    result.setGroundwaterElevation(
        borehole.getGroundwaterElevation());

    result.setUnderlyingRockThickness(
        geology.getTotalUnderlyingRockThickness());

    if (!geology.getUnderlyingRock().empty())
        result.setUnderlyingRockName(
            geology.getUnderlyingRock().front().getName());

    return result;
}
