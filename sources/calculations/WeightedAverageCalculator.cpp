#include "../../headers/calculations/WeightedAverageCalculator.h"

#include "../../headers/utils/ParameterValue.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <string>

namespace
{
    constexpr double EPS = 1e-9;

    bool contains(
        const std::string& text,
        const std::vector<std::string>& fragments)
    {
        for (const std::string& fragment : fragments)
        {
            if (text.find(fragment) != std::string::npos)
                return true;
        }
        return false;
    }

    std::string getSoilTypeFromTKP(const Sample& sample)
    {
        const ParameterValue* parameter =
            sample.findTKPParameter("Наименование грунта");

        if (parameter == nullptr)
            return {};

        return getTextValue(*parameter);
    }

    bool isLegacyCalculationSample(const Sample& sample)
    {
        if (sample.isMonolith() || sample.isAgSample())
            return false;

        std::string soilType = getSoilTypeFromTKP(sample);
        if (soilType.empty())
            soilType = sample.getSoilType();

        if (contains(soilType, {"супесь", "Супесь", "СУПЕСЬ"}) ||
            contains(soilType, {"суглинок", "Суглинок", "СУГЛИНОК"}) ||
            contains(soilType, {"глина", "Глина", "ГЛИНА"}))
        {
            return false;
        }

        return sample.getThickness() > EPS;
    }

    // ------------------------------------------------------------------------
    // КЛЮЧЕВОЕ ПРАВИЛО
    // ------------------------------------------------------------------------
    // Для средневзвешенных учитывается не просто "ПИ" из SectionAnalysis,
    // а именно часть ПИ, которая вошла в окончательную добываемую модель:
    //
    //   UsefulMineral.drySections
    //   + UsefulMineral.waterBearingSections
    //
    // Поэтому часть исходного образца, превращённая в подстилающую породу
    // из-за DryOnly / сухой подушки / ограничения обводнённого ПИ,
    // автоматически получает нулевой вес.
    double countedUsefulOverlapThickness(
        const Sample& sample,
        const BoreholeGeology& geology,
        MineralCalculationPart part = MineralCalculationPart::TotalUsefulMineral)
    {
        if (sample.isMonolith() || sample.isAgSample())
            return 0.0;

        if (sample.getThickness() <= EPS)
            return 0.0;

        double thickness = 0.0;

        if (part != MineralCalculationPart::WaterBearingUsefulMineral)
        {
            for (const DryMineralSection& section :
                 geology.getUsefulMineral().getDrySections())
            {
                const double start =
                    std::max(sample.getStartDepth(), section.getRoofDepth());
                const double end =
                    std::min(sample.getEndDepth(), section.getBottomDepth());

                if (end > start + EPS)
                    thickness += end - start;
            }
        }

        if (part != MineralCalculationPart::DryUsefulMineral)
        {
            for (const WaterBearingMineralSection& section :
                 geology.getUsefulMineral().getWaterBearingSections())
            {
                const double start =
                    std::max(sample.getStartDepth(), section.getRoofDepth());
                const double end =
                    std::min(sample.getEndDepth(), section.getBottomDepth());

                if (end > start + EPS)
                    thickness += end - start;
            }
        }

        return thickness;
    }

    double calculateParameterAverageInternal(
        const Borehole& borehole,
        const BoreholeGeology* geology,
        const std::string& parameterName,
        MineralCalculationPart part = MineralCalculationPart::TotalUsefulMineral)
    {
        double weightedSum = 0.0;
        double totalThickness = 0.0;

        for (const Sample& sample : borehole.getSamples())
        {
            double weight = 0.0;

            if (geology != nullptr)
                weight = countedUsefulOverlapThickness(sample, *geology, part);
            else if (isLegacyCalculationSample(sample))
                weight = sample.getThickness();

            if (weight <= EPS)
                continue;

            const ParameterValue* parameter =
                sample.findGOSTParameter(parameterName);

            if (parameter == nullptr)
                continue;

            const auto numericValue = getNumericValue(*parameter);
            if (!numericValue.has_value())
                continue;

            weightedSum += numericValue.value() * weight;
            totalThickness += weight;
        }

        if (totalThickness <= EPS)
            return 0.0;

        return weightedSum / totalThickness;
    }

    double calculateParameterThicknessInternal(
        const Borehole& borehole,
        const BoreholeGeology* geology,
        const std::string& parameterName,
        MineralCalculationPart part = MineralCalculationPart::TotalUsefulMineral)
    {
        double totalThickness = 0.0;

        for (const Sample& sample : borehole.getSamples())
        {
            double weight = 0.0;

            if (geology != nullptr)
                weight = countedUsefulOverlapThickness(sample, *geology, part);
            else if (isLegacyCalculationSample(sample))
                weight = sample.getThickness();

            if (weight <= EPS)
                continue;

            const ParameterValue* parameter =
                sample.findGOSTParameter(parameterName);

            if (parameter == nullptr ||
                !getNumericValue(*parameter).has_value())
            {
                continue;
            }

            totalThickness += weight;
        }

        return totalThickness;
    }

    double calculateTotalThicknessInternal(
        const Borehole& borehole,
        const BoreholeGeology* geology,
        MineralCalculationPart part = MineralCalculationPart::TotalUsefulMineral)
    {
        double totalThickness = 0.0;

        for (const Sample& sample : borehole.getSamples())
        {
            if (geology != nullptr)
                totalThickness += countedUsefulOverlapThickness(sample, *geology, part);
            else if (isLegacyCalculationSample(sample))
                totalThickness += sample.getThickness();
        }

        return totalThickness;
    }

    WeightedAverageResult calculateBoreholeInternal(
        const Borehole& borehole,
        const BoreholeGeology* geology,
        MineralCalculationPart part = MineralCalculationPart::TotalUsefulMineral)
    {
        WeightedAverageResult result;
        std::set<std::string> parameterNames;

        for (const Sample& sample : borehole.getSamples())
        {
            const double weight =
                geology != nullptr
                    ? countedUsefulOverlapThickness(sample, *geology, part)
                    : (isLegacyCalculationSample(sample)
                           ? sample.getThickness()
                           : 0.0);

            if (weight <= EPS)
                continue;

            for (const auto& [parameterName, parameterValue] :
                 sample.getGOSTParameters())
            {
                if (getNumericValue(parameterValue).has_value())
                    parameterNames.insert(parameterName);
            }
        }

        for (const std::string& parameterName : parameterNames)
        {
            const double parameterThickness =
                calculateParameterThicknessInternal(
                    borehole,
                    geology,
                    parameterName,
                    part);

            if (parameterThickness <= EPS)
                continue;

            result.setValue(
                parameterName,
                calculateParameterAverageInternal(
                    borehole,
                    geology,
                    parameterName,
                    part));
        }

        return result;
    }
}

// ============================================================================
// Новый геологически корректный интерфейс
// ============================================================================

double WeightedAverageCalculator::calculateGOSTParameterAverage(
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const BoreholeGeology& geology,
    const std::string& parameterName) const
{
    (void)analysis;
    return calculateParameterAverageInternal(
        borehole,
        &geology,
        parameterName);
}

WeightedAverageResult WeightedAverageCalculator::calculateBorehole(
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const BoreholeGeology& geology) const
{
    (void)analysis;
    return calculateBoreholeInternal(
        borehole,
        &geology);
}

double WeightedAverageCalculator::calculateGOSTParameterThickness(
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const BoreholeGeology& geology,
    const std::string& parameterName) const
{
    (void)analysis;
    return calculateParameterThicknessInternal(
        borehole,
        &geology,
        parameterName);
}

double WeightedAverageCalculator::calculateBoreholeThickness(
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const BoreholeGeology& geology) const
{
    (void)analysis;
    return calculateTotalThicknessInternal(
        borehole,
        &geology);
}

BoreholeResult WeightedAverageCalculator::calculateBoreholeResult(
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const BoreholeGeology& geology) const
{
    BoreholeResult result;
    result.setBoreholeNumber(borehole.getBoreholeNumber());
    result.setTotalThickness(
        calculateBoreholeThickness(borehole, analysis, geology));

    const WeightedAverageResult weightedAverage =
        calculateBorehole(borehole, analysis, geology);

    result.setWeightedAverage(weightedAverage);

    for (const auto& [parameterName, value] : weightedAverage.getValues())
    {
        (void)value;

        const double parameterThickness =
            calculateGOSTParameterThickness(
                borehole,
                analysis,
                geology,
                parameterName);

        if (parameterThickness > EPS)
            result.setParameterThickness(parameterName, parameterThickness);
    }

    return result;
}

BoreholeResult WeightedAverageCalculator::calculateBoreholeResult(
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const BoreholeGeology& geology,
    MineralCalculationPart part) const
{
    BoreholeResult result;
    result.setBoreholeNumber(borehole.getBoreholeNumber());
    result.setTotalThickness(
        calculateBoreholeThickness(borehole, analysis, geology, part));

    const WeightedAverageResult weightedAverage =
        calculateBoreholeInternal(borehole, &geology, part);

    result.setWeightedAverage(weightedAverage);

    for (const auto& [parameterName, value] : weightedAverage.getValues())
    {
        (void)value;

        const double parameterThickness =
            calculateParameterThicknessInternal(
                borehole,
                &geology,
                parameterName,
                part);

        if (parameterThickness > EPS)
            result.setParameterThickness(parameterName, parameterThickness);
    }

    return result;
}

double WeightedAverageCalculator::calculateBoreholeThickness(
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const BoreholeGeology& geology,
    MineralCalculationPart part) const
{
    (void)analysis;
    return calculateTotalThicknessInternal(
        borehole,
        &geology,
        part);
}

// ============================================================================
// Старые интерфейсы
// ============================================================================

double WeightedAverageCalculator::calculateGOSTParameterAverage(
    const Borehole& borehole,
    const std::string& parameterName) const
{
    return calculateParameterAverageInternal(
        borehole,
        nullptr,
        parameterName);
}

WeightedAverageResult WeightedAverageCalculator::calculateBorehole(
    const Borehole& borehole) const
{
    return calculateBoreholeInternal(borehole, nullptr);
}

double WeightedAverageCalculator::calculateGOSTParameterThickness(
    const Borehole& borehole,
    const std::string& parameterName) const
{
    return calculateParameterThicknessInternal(
        borehole,
        nullptr,
        parameterName);
}

double WeightedAverageCalculator::calculateBoreholeThickness(
    const Borehole& borehole) const
{
    return calculateTotalThicknessInternal(
        borehole,
        nullptr);
}

BoreholeResult WeightedAverageCalculator::calculateBoreholeResult(
    const Borehole& borehole) const
{
    BoreholeResult result;
    result.setBoreholeNumber(borehole.getBoreholeNumber());
    result.setTotalThickness(calculateBoreholeThickness(borehole));

    const WeightedAverageResult weightedAverage =
        calculateBorehole(borehole);

    result.setWeightedAverage(weightedAverage);

    for (const auto& [parameterName, value] : weightedAverage.getValues())
    {
        (void)value;
        const double parameterThickness =
            calculateGOSTParameterThickness(
                borehole,
                parameterName);

        if (parameterThickness > EPS)
            result.setParameterThickness(parameterName, parameterThickness);
    }

    return result;
}

WeightedAverageResult WeightedAverageCalculator::calculateBlock(
    const std::vector<BoreholeResult>& boreholeResults) const
{
    WeightedAverageResult result;
    std::set<std::string> parameterNames;

    for (const BoreholeResult& boreholeResult : boreholeResults)
    {
        for (const auto& [parameterName, value] :
             boreholeResult.getWeightedAverage().getValues())
        {
            (void)value;
            parameterNames.insert(parameterName);
        }
    }

    for (const std::string& parameterName : parameterNames)
    {
        double weightedSum = 0.0;
        double totalThickness = 0.0;

        for (const BoreholeResult& boreholeResult : boreholeResults)
        {
            const double parameterThickness =
                boreholeResult.getParameterThickness(parameterName);

            if (parameterThickness <= EPS)
                continue;

            const auto it =
                boreholeResult.getWeightedAverage().getValues().find(parameterName);

            if (it == boreholeResult.getWeightedAverage().getValues().end())
                continue;

            weightedSum += it->second * parameterThickness;
            totalThickness += parameterThickness;
        }

        if (totalThickness > EPS)
        {
            result.setValue(
                parameterName,
                weightedSum / totalThickness);
        }
    }

    return result;
}

WeightedAverageResult WeightedAverageCalculator::calculateBlock(
    const Block& block,
    const Deposit& deposit) const
{
    std::vector<BoreholeResult> results;

    for (const std::string& number : block.getBoreholeNumbers())
    {
        const Borehole* borehole = deposit.findBorehole(number);
        if (borehole == nullptr)
            continue;

        const BoreholeResult result =
            calculateBoreholeResult(*borehole);

        if (result.getTotalThickness() > EPS)
            results.push_back(result);
    }

    return calculateBlock(results);
}
