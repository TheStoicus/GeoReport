#pragma once

#include <string>
#include <vector>

#include "../domain/Borehole.h"
#include "../domain/Block.h"
#include "../domain/Deposit.h"
#include "../geology/BoreholeGeology.h"
#include "../results/BoreholeResult.h"
#include "../results/WeightedAverageResult.h"
#include "BoreholeSectionAnalysis.h"
#include "MineralCalculationPart.h"

class WeightedAverageCalculator
{
public:
    // Средневзвешенное одного параметра ГОСТ только по фактически
    // учитываемой части полезного ископаемого (сухое + разрешённое обводнённое).
    double calculateGOSTParameterAverage(
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const BoreholeGeology& geology,
        const std::string& parameterName) const;

    // Все средневзвешенные параметры скважины только по учитываемому ПИ.
    WeightedAverageResult calculateBorehole(
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const BoreholeGeology& geology) const;

    WeightedAverageResult calculateBlock(
        const std::vector<BoreholeResult>& boreholeResults) const;

    // Старый совместимый интерфейс.
    WeightedAverageResult calculateBlock(
        const Block& block,
        const Deposit& deposit) const;

    // Полный результат скважины с учётом окончательной геологии.
    BoreholeResult calculateBoreholeResult(
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const BoreholeGeology& geology) const;

    // Полный результат по одной скважине только для выбранного подблока.
    BoreholeResult calculateBoreholeResult(
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const BoreholeGeology& geology,
        MineralCalculationPart part) const;

    // Толщина выбранной части ПИ в конкретной скважине.
    double calculateBoreholeThickness(
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const BoreholeGeology& geology,
        MineralCalculationPart part) const;

    // Старый совместимый интерфейс.
    BoreholeResult calculateBoreholeResult(
        const Borehole& borehole) const;

    double calculateGOSTParameterThickness(
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const BoreholeGeology& geology,
        const std::string& parameterName) const;

    double calculateBoreholeThickness(
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const BoreholeGeology& geology) const;

    // Старые совместимые методы.
    double calculateGOSTParameterAverage(
        const Borehole& borehole,
        const std::string& parameterName) const;

    WeightedAverageResult calculateBorehole(
        const Borehole& borehole) const;

    double calculateGOSTParameterThickness(
        const Borehole& borehole,
        const std::string& parameterName) const;

    double calculateBoreholeThickness(
        const Borehole& borehole) const;
};
