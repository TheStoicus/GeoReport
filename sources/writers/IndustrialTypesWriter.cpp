#include "../../headers/writers/IndustrialTypesWriter.h"
#include "../../headers/writers/BlockDesignation.h"
#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/calculations/WeightedAverageCalculator.h"

#include <OpenXLSX.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <set>
#include <sstream>
#include <vector>

namespace
{
    constexpr double EPS = 1e-9;

    constexpr const char* SAND = "Грансостав песка <5 мм";
    constexpr const char* GRAVEL_10_5 = "Грансостав гравия 10-5 мм";
    constexpr const char* GRAVEL_OVER_5 = "Грансостав гравия >5 мм";

    // В текущем формате исходной лаборатории валуны представлены одной
    // суммарной фракцией >70 мм. Пока точных 70-150 мм нет, поэтому для
    // таблицы 13 используем имеющийся показатель >70 мм.
    constexpr const char* BOULDER_OVER_70 = "Грансостав гравия >70 мм";

    const BoreholeAnalysisResult* findResult(
        const GeologicalAnalysisResult& analysis,
        const std::string& number)
    {
        for (const auto& result : analysis.getBoreholes())
            if (result.getBoreholeNumber() == number)
                return &result;
        return nullptr;
    }

    double overlap(double a1, double a2, double b1, double b2)
    {
        const double left = std::max(a1, b1);
        const double right = std::min(a2, b2);
        return right > left + EPS ? right - left : 0.0;
    }
}

double IndustrialTypesWriter::CriterionResult::percentage() const
{
    return total > 0
        ? 100.0 * static_cast<double>(matching) / total
        : 0.0;
}

bool IndustrialTypesWriter::overlapsUsefulMineral(
    const Sample& sample,
    const BoreholeAnalysisResult& result)
{
    const auto& mineral = result.getGeology().getUsefulMineral();

    for (const auto& section : mineral.getDrySections())
    {
        if (overlap(sample.getStartDepth(), sample.getEndDepth(),
                    section.getRoofDepth(), section.getBottomDepth()) > EPS)
            return true;
    }

    for (const auto& section : mineral.getWaterBearingSections())
    {
        if (overlap(sample.getStartDepth(), sample.getEndDepth(),
                    section.getRoofDepth(), section.getBottomDepth()) > EPS)
            return true;
    }

    return false;
}

std::string IndustrialTypesWriter::baseSampleNumber(const std::string& number)
{
    // После разделения SampleSplitter создаёт фрагменты 25а, 25б, ... .
    // В таблице 13 одна исходная проба должна считаться одной пробой.
    static const std::vector<std::string> suffixes = {
        "а","б","в","г","д","е","ё","ж","з","и","й","к",
        "л","м","н","о","п","р","с","т","у","ф","х","ц",
        "ч","ш","щ","ъ","ы","ь","э","ю","я"
    };

    for (const auto& suffix : suffixes)
    {
        if (number.size() <= suffix.size())
            continue;

        const auto pos = number.size() - suffix.size();
        if (number.compare(pos, suffix.size(), suffix) != 0)
            continue;

        const std::string base = number.substr(0, pos);
        if (!base.empty() && std::all_of(
                base.begin(), base.end(),
                [](unsigned char c) { return std::isdigit(c) != 0; }))
            return base;
    }

    return number;
}

bool IndustrialTypesWriter::matchesType1(const Sample& sample)
{
    const auto* sand = sample.findGOSTParameter(SAND);
    const auto* gravel = sample.findGOSTParameter(GRAVEL_10_5);

    if (!sand || !gravel)
        return false;

    const auto sandValue = getNumericValue(*sand);
    const auto gravelValue = getNumericValue(*gravel);

    return sandValue && gravelValue &&
           *sandValue > 90.0 + EPS &&
           *gravelValue <= 10.0 + EPS;
}

bool IndustrialTypesWriter::matchesType2(const Sample& sample)
{
    const auto* gravel = sample.findGOSTParameter(GRAVEL_OVER_5);
    const auto* boulder = sample.findGOSTParameter(BOULDER_OVER_70);

    if (!gravel || !boulder)
        return false;

    const auto gravelValue = getNumericValue(*gravel);
    const auto boulderValue = getNumericValue(*boulder);

    return gravelValue && boulderValue &&
           *gravelValue >= 10.0 - EPS &&
           *gravelValue <= 90.0 + EPS &&
           *boulderValue <= 5.0 + EPS;
}

bool IndustrialTypesWriter::matchesType1(const WeightedAverageResult& result)
{
    const auto& values = result.getValues();
    const auto sand = values.find(SAND);
    const auto gravel = values.find(GRAVEL_10_5);

    return sand != values.end() && gravel != values.end() &&
           sand->second > 90.0 + EPS &&
           gravel->second <= 10.0 + EPS;
}

bool IndustrialTypesWriter::matchesType2(const WeightedAverageResult& result)
{
    const auto& values = result.getValues();
    const auto gravel = values.find(GRAVEL_OVER_5);
    const auto boulder = values.find(BOULDER_OVER_70);

    return gravel != values.end() && boulder != values.end() &&
           gravel->second >= 10.0 - EPS &&
           gravel->second <= 90.0 + EPS &&
           boulder->second <= 5.0 + EPS;
}

IndustrialTypesWriter::CriterionResult
IndustrialTypesWriter::calculateSamples(
    const Block& block,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysis,
    bool type1)
{
    CriterionResult result;
    std::set<std::string> usefulSamples;
    std::set<std::string> matchingSamples;

    for (const auto& number : block.getBoreholeNumbers())
    {
        const Borehole* borehole = deposit.findBorehole(number);
        const BoreholeAnalysisResult* boreholeResult = findResult(analysis, number);

        if (!borehole || !boreholeResult ||
            !boreholeResult->getSectionAnalysis().isResolved())
            continue;

        for (const auto& sample : borehole->getSamples())
        {
            if (sample.isMonolith() || sample.isAgSample() ||
                !overlapsUsefulMineral(sample, *boreholeResult))
                continue;

            const std::string base = baseSampleNumber(sample.getSampleNumber());
            usefulSamples.insert(base);

            if (type1 ? matchesType1(sample) : matchesType2(sample))
                matchingSamples.insert(base);
        }
    }

    result.total = static_cast<int>(usefulSamples.size());
    result.matching = static_cast<int>(matchingSamples.size());
    return result;
}

IndustrialTypesWriter::CriterionResult
IndustrialTypesWriter::calculateBoreholes(
    const Block& block,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysis,
    bool type1)
{
    CriterionResult result;
    WeightedAverageCalculator calculator;

    for (const auto& number : block.getBoreholeNumbers())
    {
        const Borehole* borehole = deposit.findBorehole(number);
        const BoreholeAnalysisResult* boreholeResult = findResult(analysis, number);

        if (!borehole || !boreholeResult ||
            !boreholeResult->getSectionAnalysis().isResolved())
            continue;

        bool hasUseful = false;
        for (const auto& sample : borehole->getSamples())
        {
            if (!sample.isMonolith() && !sample.isAgSample() &&
                overlapsUsefulMineral(sample, *boreholeResult))
            {
                hasUseful = true;
                break;
            }
        }

        if (!hasUseful)
            continue;

        ++result.total;

        const WeightedAverageResult weighted = calculator.calculateBorehole(
            *borehole,
            boreholeResult->getSectionAnalysis(),
            boreholeResult->getGeology());

        if (type1 ? matchesType1(weighted) : matchesType2(weighted))
            ++result.matching;
    }

    return result;
}

std::string IndustrialTypesWriter::formatCriterion(const CriterionResult& result)
{
    std::ostringstream out;
    out << result.matching << " ("
        << std::fixed << std::setprecision(2)
        << result.percentage() << "%)";
    return out.str();
}

void IndustrialTypesWriter::write(
    OpenXLSX::XLWorksheet& sheet,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult) const
{
    unsigned int column = FIRST_COLUMN;

    for (const auto& block : deposit.getBlocks())
    {
        const CriterionResult type1Samples =
            calculateSamples(block, deposit, analysisResult, true);
        const CriterionResult type1Boreholes =
            calculateBoreholes(block, deposit, analysisResult, true);
        const CriterionResult type2Samples =
            calculateSamples(block, deposit, analysisResult, false);
        const CriterionResult type2Boreholes =
            calculateBoreholes(block, deposit, analysisResult, false);

        ExcelWriteUtils::writeString(sheet, 2, column, BlockDesignation::format(block.getName()));
        ExcelWriteUtils::writeString(
            sheet, 3, column,
            std::to_string(type1Samples.total) + " / " +
            std::to_string(type1Boreholes.total));
        ExcelWriteUtils::writeString(
            sheet, 5, column, formatCriterion(type1Samples));
        ExcelWriteUtils::writeString(
            sheet, 7, column, formatCriterion(type1Boreholes));
        ExcelWriteUtils::writeString(
            sheet, 9, column, formatCriterion(type2Samples));
        ExcelWriteUtils::writeString(
            sheet, 11, column, formatCriterion(type2Boreholes));

        ++column;
    }
}
