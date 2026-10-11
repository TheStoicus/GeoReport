#pragma once

#include "GeologicalAnalysisResult.h"

#include "../domain/Deposit.h"
#include "../settings/MainIndicatorsSettings.h"
#include "../settings/GroundwaterSettings.h"
#include "../settings/DevelopmentSettings.h"
#include "../settings/GroundwaterCalculationSettings.h"

class GeologicalAnalysisService
{
public:
    /*
     * Выполняет полный геологический анализ всех скважин месторождения.
     *
     * Вход:
     *   deposit - исходные данные скважин, проб и блоков;
     *   mainIndicatorsSettings - правила определения полезного ископаемого;
     *   groundwaterSettings - правила работы с УГВ;
     *   developmentSettings - правила разработки месторождения.
     *
     * В процессе анализа пробы скважин могут быть разделены
     * по границам окончательных геологических участков.
     *
     * Поэтому deposit передаётся по ссылке без const.
     *
     * Выход:
     *   GeologicalAnalysisResult - содержащий все результаты анализа.
     */
    GeologicalAnalysisResult analyze(
        Deposit& deposit,
        const MainIndicatorsSettings& mainIndicatorsSettings,
        const GroundwaterSettings& groundwaterSettings,
        const DevelopmentSettings& developmentSettings,
        const GroundwaterCalculationSettings& groundwaterCalculationSettings) const;
};
