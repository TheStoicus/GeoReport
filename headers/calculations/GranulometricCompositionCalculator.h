#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "../domain/Block.h"
#include "../domain/Deposit.h"
#include "../results/BoreholeResult.h"
#include "../results/WeightedAverageResult.h"
#include "BoreholeSectionAnalysis.h"
#include "MineralCalculationPart.h"
#include "../geology/BoreholeGeology.h"
#include "../application/GeologicalAnalysisResult.h"

struct GranulometricParameterRange
{
    bool hasValue = false;
    double minimum = 0.0;
    double maximum = 0.0;
};

struct GranulometricCompositionBlockResult
{
    std::string blockName;

    std::size_t sampleCount = 0;
    std::vector<GranulometricParameterRange> sampleRanges;

    std::size_t boreholeCount = 0;
    std::vector<GranulometricParameterRange> boreholeRanges;

    WeightedAverageResult blockWeightedAverage;
};

class GranulometricCompositionCalculator
{
public:
    // Новый основной интерфейс: расчёт только по окончательно выделенному ПИ.
    GranulometricCompositionBlockResult calculate(
        const Block& block,
        const Deposit& deposit,
        const std::vector<BoreholeSectionAnalysis>& analyses,
        const std::vector<BoreholeAnalysisResult>& geologyResults) const;

    // Расчёт отдельного сухого или обводнённого подблока.
    GranulometricCompositionBlockResult calculate(
        const Block& block,
        const Deposit& deposit,
        const std::vector<BoreholeSectionAnalysis>& analyses,
        const std::vector<BoreholeAnalysisResult>& geologyResults,
        MineralCalculationPart part) const;

    // Совместимый интерфейс старой версии проекта.
    GranulometricCompositionBlockResult calculate(
        const Block& block,
        const Deposit& deposit) const;

private:
    static const std::vector<std::string>& getParameterNames();

    static std::vector<GranulometricParameterRange> calculateSampleRanges(
        const Block& block,
        const Deposit& deposit,
        const std::vector<BoreholeSectionAnalysis>* analyses,
        const std::vector<BoreholeAnalysisResult>* geologyResults,
        std::size_t& sampleCount,
        MineralCalculationPart part = MineralCalculationPart::TotalUsefulMineral);

    static std::vector<GranulometricParameterRange> calculateBoreholeRanges(
        const std::vector<BoreholeResult>& boreholeResults,
        std::size_t& boreholeCount);

    static bool isMonolith(const Sample& sample);
    static bool isCalculationSample(const Sample& sample);

    static void addValue(
        GranulometricParameterRange& range,
        double value);
};
