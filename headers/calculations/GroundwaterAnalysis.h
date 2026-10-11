#pragma once

#include "../domain/Borehole.h"
#include "../settings/GroundwaterCalculationSettings.h"

#include <cstddef>
#include <vector>


class GroundwaterAnalysis
{
public:

    void analyze(
        const std::vector<Borehole>& boreholes);

    void analyze(
        const std::vector<Borehole>& boreholes,
        const GroundwaterCalculationSettings& settings);


    std::size_t getTotalBoreholeCount() const;

    std::size_t getWaterBearingBoreholeCount() const;

    double getWaterBearingRatio() const;


    GroundwaterCalculationMode
    getCalculationMode() const;


    double getMeanGroundwaterDepth() const;

    double getMeanGroundwaterElevation() const;


    bool isGroundwaterPredominant() const;

    bool hasMeanGroundwaterLevel() const;


    /*
     * Расчётная глубина УГВ для конкретной скважины.
     *
     * Если используется средний уровень:
     *
     *     отметка устья - средняя отметка УГВ
     *
     * Если используется индивидуальный уровень:
     *
     *     фактическая глубина УГВ.
     */
    double getApplicableGroundwaterDepth(
        const Borehole& borehole) const;


    /*
     * Расчётная абсолютная отметка УГВ.
     */
    double getApplicableGroundwaterElevation(
        const Borehole& borehole) const;


private:

    std::size_t totalBoreholeCount = 0;

    std::size_t waterBearingBoreholeCount = 0;

    double waterBearingRatio = 0.0;


    bool groundwaterPredominant = false;


    GroundwaterCalculationMode calculationMode =
        GroundwaterCalculationMode::IndividualLevel;


    bool meanGroundwaterLevelAvailable = false;


    /*
     * Средняя глубина — только информационное значение.
     */
    double meanGroundwaterDepth = 0.0;


    /*
     * Основное расчётное значение.
     */
    double meanGroundwaterElevation = 0.0;
};
