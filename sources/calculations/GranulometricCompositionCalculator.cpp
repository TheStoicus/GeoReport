#include "../../headers/calculations/GranulometricCompositionCalculator.h"

#include "../../headers/utils/ParameterValue.h"
#include "../../headers/calculations/WeightedAverageCalculator.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace
{
    constexpr double EPS = 1e-9;

    const std::vector<std::string> PARAMETERS =
    {
        "Грансостав гравия >5 мм",
        "Грансостав гравия >70 мм",
        "Грансостав гравия 70-40 мм",
        "Грансостав гравия 40-20 мм",
        "Грансостав гравия 20-10 мм",
        "Грансостав гравия 10-5 мм",
        "Грансостав песка 2.5 мм",
        "Грансостав песка 1.25 мм",
        "Грансостав песка 0.63 мм",
        "Грансостав песка 0.315 мм",
        "Грансостав песка 0.16 мм",
        "Грансостав песка <0.16 мм",
        "Глинистые в песке, %",
        "Глинистые в гравии, %",
        "Глинистые в породе, %",
        "Модуль крупности песка"
    };

    bool containsAnyCaseVariant(
        const std::string& text,
        const std::string& lower,
        const std::string& capitalized,
        const std::string& upper)
    {
        return text.find(lower) != std::string::npos ||
               text.find(capitalized) != std::string::npos ||
               text.find(upper) != std::string::npos;
    }

    std::string getSoilTypeFromTKP(const Sample& sample)
    {
        const ParameterValue* parameter =
            sample.findTKPParameter("Наименование грунта");

        if (parameter == nullptr)
            return {};

        return getTextValue(*parameter);
    }

    const BoreholeSectionAnalysis* findAnalysis(
        const std::vector<BoreholeSectionAnalysis>& analyses,
        const std::string& boreholeNumber)
    {
        for (const BoreholeSectionAnalysis& analysis : analyses)
        {
            if (analysis.getBoreholeNumber() == boreholeNumber)
                return &analysis;
        }

        return nullptr;
    }

    double countedUsefulOverlap(
        const Sample& sample,
        const BoreholeGeology& geology,
        MineralCalculationPart part = MineralCalculationPart::TotalUsefulMineral)
    {
        if (sample.isMonolith() || sample.isAgSample())
            return 0.0;

        double result = 0.0;

        if (part != MineralCalculationPart::WaterBearingUsefulMineral)
        {
            for (const DryMineralSection& section :
                 geology.getUsefulMineral().getDrySections())
            {
                const double start = std::max(sample.getStartDepth(), section.getRoofDepth());
                const double end = std::min(sample.getEndDepth(), section.getBottomDepth());
                if (end > start + EPS)
                    result += end - start;
            }
        }

        if (part != MineralCalculationPart::DryUsefulMineral)
        {
            for (const WaterBearingMineralSection& section :
                 geology.getUsefulMineral().getWaterBearingSections())
            {
                const double start = std::max(sample.getStartDepth(), section.getRoofDepth());
                const double end = std::min(sample.getEndDepth(), section.getBottomDepth());
                if (end > start + EPS)
                    result += end - start;
            }
        }

        return result;
    }
}

const std::vector<std::string>& GranulometricCompositionCalculator::getParameterNames()
{
    return PARAMETERS;
}

bool GranulometricCompositionCalculator::isMonolith(const Sample& sample)
{
    return sample.isMonolith();
}

bool GranulometricCompositionCalculator::isCalculationSample(const Sample& sample)
{
    if (sample.isMonolith() || sample.isAgSample())
        return false;

    if (sample.getThickness() <= EPS)
        return false;

    const std::string soilType = getSoilTypeFromTKP(sample);

    if (containsAnyCaseVariant(soilType, "супесь", "Супесь", "СУПЕСЬ") ||
        containsAnyCaseVariant(soilType, "суглинок", "Суглинок", "СУГЛИНОК") ||
        containsAnyCaseVariant(soilType, "глина", "Глина", "ГЛИНА"))
    {
        return false;
    }

    return true;
}

void GranulometricCompositionCalculator::addValue(
    GranulometricParameterRange& range,
    double value)
{
    if (!range.hasValue)
    {
        range.hasValue = true;
        range.minimum = value;
        range.maximum = value;
        return;
    }

    range.minimum = std::min(range.minimum, value);
    range.maximum = std::max(range.maximum, value);
}

std::vector<GranulometricParameterRange>
GranulometricCompositionCalculator::calculateSampleRanges(
    const Block& block,
    const Deposit& deposit,
    const std::vector<BoreholeSectionAnalysis>* analyses,
    const std::vector<BoreholeAnalysisResult>* geologyResults,
    std::size_t& sampleCount,
    MineralCalculationPart part)
{
    std::vector<GranulometricParameterRange> ranges(PARAMETERS.size());
    sampleCount = 0;

    for (const std::string& boreholeNumber : block.getBoreholeNumbers())
    {
        const Borehole* borehole = deposit.findBorehole(boreholeNumber);
        if (borehole == nullptr)
            continue;

        const BoreholeSectionAnalysis* analysis =
            analyses == nullptr ? nullptr : findAnalysis(*analyses, boreholeNumber);

        for (const Sample& sample : borehole->getSamples())
        {
            if (!isCalculationSample(sample))
                continue;

            if (analysis != nullptr && geologyResults != nullptr)
            {
                const BoreholeGeology* geology = nullptr;

                for (const BoreholeAnalysisResult& boreholeResult : *geologyResults)
                {
                    if (boreholeResult.getBoreholeNumber() == boreholeNumber)
                    {
                        geology = &boreholeResult.getGeology();
                        break;
                    }
                }

                if (geology == nullptr)
                    continue;

                // В раздел "по пробам" попадают только полностью
                // учитываемые фрагменты добываемого ПИ.
                const double usefulThickness =
                    countedUsefulOverlap(sample, *geology, part);

                if (usefulThickness <= EPS ||
                    std::abs(usefulThickness - sample.getThickness()) > EPS)
                {
                    continue;
                }
            }

            ++sampleCount;

            for (std::size_t i = 0; i < PARAMETERS.size(); ++i)
            {
                const ParameterValue* parameter =
                    sample.findGOSTParameter(PARAMETERS[i]);

                if (parameter == nullptr)
                    continue;

                const auto numericValue = getNumericValue(*parameter);
                if (!numericValue.has_value())
                    continue;

                addValue(ranges[i], *numericValue);
            }
        }
    }

    return ranges;
}

std::vector<GranulometricParameterRange>
GranulometricCompositionCalculator::calculateBoreholeRanges(
    const std::vector<BoreholeResult>& boreholeResults,
    std::size_t& boreholeCount)
{
    std::vector<GranulometricParameterRange> ranges(PARAMETERS.size());
    boreholeCount = 0;

    for (const BoreholeResult& boreholeResult : boreholeResults)
    {
        if (boreholeResult.getTotalThickness() <= EPS)
            continue;

        ++boreholeCount;

        const auto& values =
            boreholeResult.getWeightedAverage().getValues();

        for (std::size_t i = 0; i < PARAMETERS.size(); ++i)
        {
            const auto it = values.find(PARAMETERS[i]);
            if (it == values.end())
                continue;

            if (boreholeResult.getParameterThickness(PARAMETERS[i]) <= EPS)
                continue;

            addValue(ranges[i], it->second);
        }
    }

    return ranges;
}

GranulometricCompositionBlockResult
GranulometricCompositionCalculator::calculate(
    const Block& block,
    const Deposit& deposit,
    const std::vector<BoreholeSectionAnalysis>& analyses,
    const std::vector<BoreholeAnalysisResult>& geologyResults) const
{
    return calculate(
        block,
        deposit,
        analyses,
        geologyResults,
        MineralCalculationPart::TotalUsefulMineral);
}

GranulometricCompositionBlockResult
GranulometricCompositionCalculator::calculate(
    const Block& block,
    const Deposit& deposit,
    const std::vector<BoreholeSectionAnalysis>& analyses,
    const std::vector<BoreholeAnalysisResult>& geologyResults,
    MineralCalculationPart part) const
{
    GranulometricCompositionBlockResult result;
    result.blockName = block.getName();

    result.sampleRanges = calculateSampleRanges(
        block,
        deposit,
        &analyses,
        &geologyResults,
        result.sampleCount,
        part);

    WeightedAverageCalculator weightedAverageCalculator;
    std::vector<BoreholeResult> boreholeResults;

    for (const std::string& boreholeNumber : block.getBoreholeNumbers())
    {
        const Borehole* borehole = deposit.findBorehole(boreholeNumber);
        if (borehole == nullptr)
            continue;

        const BoreholeSectionAnalysis* analysis =
            findAnalysis(analyses, boreholeNumber);

        if (analysis == nullptr || !analysis->isResolved())
            continue;

        const BoreholeGeology* geology = nullptr;
        for (const BoreholeAnalysisResult& boreholeResult : geologyResults)
        {
            if (boreholeResult.getBoreholeNumber() == boreholeNumber)
            {
                geology = &boreholeResult.getGeology();
                break;
            }
        }

        if (geology == nullptr)
            continue;

        const BoreholeResult boreholeResult =
            weightedAverageCalculator.calculateBoreholeResult(
                *borehole,
                *analysis,
                *geology,
                part);

        if (boreholeResult.getTotalThickness() <= EPS)
            continue;

        boreholeResults.push_back(boreholeResult);
    }

    result.boreholeRanges = calculateBoreholeRanges(
        boreholeResults,
        result.boreholeCount);

    result.blockWeightedAverage =
        weightedAverageCalculator.calculateBlock(boreholeResults);

    return result;
}

GranulometricCompositionBlockResult
GranulometricCompositionCalculator::calculate(
    const Block& block,
    const Deposit& deposit) const
{
    GranulometricCompositionBlockResult result;
    result.blockName = block.getName();

    result.sampleRanges = calculateSampleRanges(
        block,
        deposit,
        nullptr,
        nullptr,
        result.sampleCount);

    WeightedAverageCalculator weightedAverageCalculator;
    std::vector<BoreholeResult> boreholeResults;

    for (const std::string& boreholeNumber : block.getBoreholeNumbers())
    {
        const Borehole* borehole = deposit.findBorehole(boreholeNumber);
        if (borehole == nullptr)
            continue;

        const BoreholeResult boreholeResult =
            weightedAverageCalculator.calculateBoreholeResult(*borehole);

        if (boreholeResult.getTotalThickness() > EPS)
            boreholeResults.push_back(boreholeResult);
    }

    result.boreholeRanges = calculateBoreholeRanges(
        boreholeResults,
        result.boreholeCount);

    result.blockWeightedAverage =
        weightedAverageCalculator.calculateBlock(boreholeResults);

    return result;
}
