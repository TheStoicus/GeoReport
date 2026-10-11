#pragma once

#include "BoreholeSectionAnalysis.h"

#include "../domain/Deposit.h"
#include "../settings/MainIndicatorsSettings.h"

#include <vector>


class BoreholeSectionAnalyzer
{
public:

    /*
     * Анализирует все скважины месторождения.
     */
    std::vector<BoreholeSectionAnalysis> analyze(
        const Deposit& deposit,
        const MainIndicatorsSettings& settings) const;


    /*
     * Анализирует одну скважину.
     *
     * Последовательность:
     *
     * 1. Классификация интервалов.
     * 2. Получение решений для неоднозначных интервалов.
     * 3. Построение окончательной модели.
     */
    BoreholeSectionAnalysis analyzeBorehole(
        const Borehole& borehole,
        const MainIndicatorsSettings& settings) const;
};
