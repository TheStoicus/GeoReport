#include "../../headers/calculations/GroundwaterAnalysis.h"


void GroundwaterAnalysis::analyze(
    const std::vector<Borehole>& boreholes)
{
    GroundwaterCalculationSettings settings;

    analyze(
        boreholes,
        settings);
}


void GroundwaterAnalysis::analyze(
    const std::vector<Borehole>& boreholes,
    const GroundwaterCalculationSettings& settings)
{
    totalBoreholeCount =
        boreholes.size();

    waterBearingBoreholeCount = 0;

    waterBearingRatio = 0.0;

    groundwaterPredominant = false;

    calculationMode =
        GroundwaterCalculationMode::IndividualLevel;

    meanGroundwaterLevelAvailable = false;

    meanGroundwaterDepth = 0.0;

    meanGroundwaterElevation = 0.0;


    if (boreholes.empty())
    {
        return;
    }


    double groundwaterDepthSum = 0.0;

    double groundwaterElevationSum = 0.0;


    for (const Borehole& borehole : boreholes)
    {
        /*
         * Используем фактическую глубину УГВ,
         * считанную из исходного Excel.
         */
        const double groundwaterDepth =
            borehole.getGroundwaterDepth();


        /*
         * Отсутствующий УГВ не участвует
         * в расчёте среднего уровня.
         */
        if (groundwaterDepth <= 0.0)
        {
            continue;
        }


        ++waterBearingBoreholeCount;


        groundwaterDepthSum +=
            groundwaterDepth;


        groundwaterElevationSum +=
            borehole.getGroundwaterElevation();
    }


    waterBearingRatio =
        static_cast<double>(
            waterBearingBoreholeCount)
        /
        static_cast<double>(
            totalBoreholeCount);


    /*
     * --------------------------------------------------------
     * Определяем режим расчёта
     * --------------------------------------------------------
     */

    const GroundwaterCalculationMode requestedMode =
        settings.getMode();


    if (requestedMode ==
        GroundwaterCalculationMode::MeanLevel)
    {
        groundwaterPredominant = true;
    }
    else if (requestedMode ==
             GroundwaterCalculationMode::IndividualLevel)
    {
        groundwaterPredominant = false;
    }
    else
    {
        /*
         * Автоматический режим.
         *
         * УГВ считается преобладающим,
         * если его вскрывает не менее заданной
         * доли скважин.
         */
        groundwaterPredominant =
            waterBearingRatio >=
            settings.getMajorityThreshold();
    }


    if (!groundwaterPredominant)
    {
        calculationMode =
            GroundwaterCalculationMode::IndividualLevel;

        return;
    }


    /*
     * Средний уровень невозможно определить,
     * если нет ни одной водоносной скважины.
     */
    if (waterBearingBoreholeCount == 0)
    {
        groundwaterPredominant = false;

        calculationMode =
            GroundwaterCalculationMode::IndividualLevel;

        return;
    }


    calculationMode =
        GroundwaterCalculationMode::MeanLevel;


    /*
     * Средняя глубина — только для контроля.
     *
     * Для расчётов она не используется.
     */
    meanGroundwaterDepth =
        groundwaterDepthSum
        /
        static_cast<double>(
            waterBearingBoreholeCount);


    /*
     * Средняя абсолютная отметка УГВ —
     * основное расчётное значение.
     */
    meanGroundwaterElevation =
        groundwaterElevationSum
        /
        static_cast<double>(
            waterBearingBoreholeCount);


    meanGroundwaterLevelAvailable = true;
}


std::size_t
GroundwaterAnalysis::getTotalBoreholeCount() const
{
    return totalBoreholeCount;
}


std::size_t
GroundwaterAnalysis::getWaterBearingBoreholeCount() const
{
    return waterBearingBoreholeCount;
}


double
GroundwaterAnalysis::getWaterBearingRatio() const
{
    return waterBearingRatio;
}


GroundwaterCalculationMode
GroundwaterAnalysis::getCalculationMode() const
{
    return calculationMode;
}


double
GroundwaterAnalysis::getMeanGroundwaterDepth() const
{
    return meanGroundwaterDepth;
}


double
GroundwaterAnalysis::getMeanGroundwaterElevation() const
{
    return meanGroundwaterElevation;
}


bool
GroundwaterAnalysis::isGroundwaterPredominant() const
{
    return groundwaterPredominant;
}


bool
GroundwaterAnalysis::hasMeanGroundwaterLevel() const
{
    return meanGroundwaterLevelAvailable;
}


double
GroundwaterAnalysis::getApplicableGroundwaterDepth(
    const Borehole& borehole) const
{
    /*
     * При среднем уровне используем
     * среднюю абсолютную отметку УГВ.
     */
    if (groundwaterPredominant &&
        meanGroundwaterLevelAvailable)
    {
        return
            borehole.getCollarElevation()
            -
            meanGroundwaterElevation;
    }


    /*
     * Иначе используем фактический УГВ
     * конкретной скважины.
     */
    return borehole.getGroundwaterDepth();
}


double
GroundwaterAnalysis::getApplicableGroundwaterElevation(
    const Borehole& borehole) const
{
    if (groundwaterPredominant &&
        meanGroundwaterLevelAvailable)
    {
        return meanGroundwaterElevation;
    }


    return borehole.getGroundwaterElevation();
}