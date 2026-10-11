#include "../../headers/geology/UsefulMineralResolver.h"

#include <algorithm>


UsefulMineral
UsefulMineralResolver::resolve(
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const GroundwaterAnalysis& groundwaterAnalysis,
    const GroundwaterSettings& groundwaterSettings,
    const DevelopmentSettings& developmentSettings) const
{
    UsefulMineral result;


    const double groundwaterDepth =
        groundwaterAnalysis.getApplicableGroundwaterDepth(
            borehole);


    for (const BoreholeSection& section :
         analysis.getFinalSections().getSections())
    {
        if (!section.isUsefulMineral())
        {
            continue;
        }


        addUsefulSection(
            result,
            section.getStartDepth(),
            section.getEndDepth(),
            groundwaterDepth,
            groundwaterSettings,
            developmentSettings);
    }


    return result;
}


void
UsefulMineralResolver::addUsefulSection(
    UsefulMineral& result,
    double startDepth,
    double endDepth,
    double groundwaterDepth,
    const GroundwaterSettings& groundwaterSettings,
    const DevelopmentSettings& developmentSettings) const
{
    if (endDepth <= startDepth)
    {
        return;
    }


    /*
     * УГВ отсутствует.
     *
     * Весь интервал ПИ является сухим.
     */
    if (groundwaterDepth <= 0.0)
    {
        result.addDrySection(
            DryMineralSection(
                startDepth,
                endDepth));

        return;
    }


    const double dryBoundaryDepth =
        getDryBoundaryDepth(
            groundwaterDepth,
            groundwaterSettings);


    /*
     * --------------------------------------------------------
     * Только сухое полезное ископаемое
     * --------------------------------------------------------
     */
    if (groundwaterSettings.getUsefulMineralWaterMode() ==
        UsefulMineralWaterMode::DryOnly)
    {
        const double dryEnd =
            std::min(
                endDepth,
                dryBoundaryDepth);


        if (dryEnd > startDepth)
        {
            result.addDrySection(
                DryMineralSection(
                    startDepth,
                    dryEnd));
        }


        return;
    }


    /*
     * --------------------------------------------------------
     * Сухая часть
     * --------------------------------------------------------
     */
    if (startDepth < dryBoundaryDepth)
    {
        const double dryEnd =
            std::min(
                endDepth,
                dryBoundaryDepth);


        if (dryEnd > startDepth)
        {
            result.addDrySection(
                DryMineralSection(
                    startDepth,
                    dryEnd));
        }
    }


    /*
     * --------------------------------------------------------
     * Обводнённая часть
     * --------------------------------------------------------
     */
    if (endDepth > dryBoundaryDepth)
    {
        const double waterStart =
            std::max(
                startDepth,
                dryBoundaryDepth);


        if (endDepth > waterStart)
        {
            addWaterBearingSection(
                result,
                waterStart,
                endDepth,
                groundwaterDepth,
                developmentSettings);
        }
    }
}


void
UsefulMineralResolver::addWaterBearingSection(
    UsefulMineral& result,
    double startDepth,
    double endDepth,
    double groundwaterDepth,
    const DevelopmentSettings& developmentSettings) const
{
    if (endDepth <= startDepth)
    {
        return;
    }


    /*
     * 0 означает отсутствие ограничения.
     *
     * Например, для земснаряда пользователь может
     * задать 10 м или любое другое значение.
     */
    const double maximumWaterBearingThickness =
        developmentSettings
            .getWaterBearingMiningThickness();


    if (maximumWaterBearingThickness <= 0.0)
    {
        /*
         * Ограничение не задано —
         * принимаем весь интервал.
         */
        result.addWaterBearingSection(
            WaterBearingMineralSection(
                startDepth,
                endDepth));

        return;
    }


    /*
     * Верхняя граница обводнённого ПИ.
     */
    const double maximumEndDepth =
        groundwaterDepth +
        maximumWaterBearingThickness;


    const double waterEnd =
        std::min(
            endDepth,
            maximumEndDepth);


    if (waterEnd > startDepth)
    {
        result.addWaterBearingSection(
            WaterBearingMineralSection(
                startDepth,
                waterEnd));
    }
}


double
UsefulMineralResolver::getDryBoundaryDepth(
    double groundwaterDepth,
    const GroundwaterSettings& settings) const
{
    /*
     * Например:
     *
     * УГВ = 8.50
     * сухая подушка = 1.00
     *
     * граница = 7.50
     */
    return std::max(
        0.0,
        groundwaterDepth -
        settings.getDryLayerAboveGroundwater());
}