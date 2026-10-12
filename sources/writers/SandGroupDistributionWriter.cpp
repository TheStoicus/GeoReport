#include "../../headers/writers/SandGroupDistributionWriter.h"
#include "../../headers/writers/BlockDesignation.h"
#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/calculations/WeightedAverageCalculator.h"
#include "../../headers/utils/SandClassification.h"
#include "../../headers/utils/ParameterValue.h"
#include "../../headers/domain/Borehole.h"
#include "../../headers/domain/Sample.h"
#include "../../headers/geology/BoreholeGeology.h"

#include <OpenXLSX.hpp>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace
{
    constexpr double EPS = 1e-9;

    std::string cellValue(std::size_t count, std::size_t total)
    {
        std::ostringstream out;
        out << count << " (" << std::fixed << std::setprecision(2)
            << (total > 0 ? 100.0 * static_cast<double>(count) / total : 0.0)
            << "%)";
        return out.str();
    }

    const BoreholeAnalysisResult* findResult(
        const GeologicalAnalysisResult& result,
        const std::string& number)
    {
        for (const auto& item : result.getBoreholes())
            if (item.getBoreholeNumber() == number)
                return &item;
        return nullptr;
    }

    double usefulThickness(
        const Sample& sample,
        const BoreholeGeology& geology)
    {
        double result = 0.0;
        for (const auto& section : geology.getUsefulMineral().getDrySections())
        {
            const double a = std::max(sample.getStartDepth(), section.getRoofDepth());
            const double b = std::min(sample.getEndDepth(), section.getBottomDepth());
            if (b > a + EPS) result += b - a;
        }
        for (const auto& section : geology.getUsefulMineral().getWaterBearingSections())
        {
            const double a = std::max(sample.getStartDepth(), section.getRoofDepth());
            const double b = std::min(sample.getEndDepth(), section.getBottomDepth());
            if (b > a + EPS) result += b - a;
        }
        return result;
    }

    GostSandClass classFromModule(double module)
    {
        if (module > 3.0 && module <= 3.5) return GostSandClass::IncreasedCoarseness;
        if (module > 2.5 && module <= 3.0) return GostSandClass::Coarse;
        if (module > 2.0 && module <= 2.5) return GostSandClass::Medium;
        if (module > 1.5 && module <= 2.0) return GostSandClass::Small;
        if (module > 1.0 && module <= 1.5) return GostSandClass::VerySmall;
        if (module > 0.7 && module <= 1.0) return GostSandClass::Fine;
        if (module >= 0.0 && module <= 0.7) return GostSandClass::VeryFine;
        return GostSandClass::Unknown;
    }

    std::size_t sampleCount(
        const Block& block,
        const Deposit& deposit,
        const GeologicalAnalysisResult& result,
        GostSandClass wanted)
    {
        std::size_t count = 0;
        for (const auto& number : block.getBoreholeNumbers())
        {
            const Borehole* borehole = deposit.findBorehole(number);
            const auto* boreholeResult = findResult(result, number);
            if (!borehole || !boreholeResult) continue;
            for (const auto& sample : borehole->getSamples())
            {
                if (sample.getThickness() <= EPS) continue;
                if (usefulThickness(sample, boreholeResult->getGeology()) <= EPS) continue;
                if (sample.getGostSandClass() == wanted) ++count;
            }
        }
        return count;
    }

    std::size_t intersectionCount(
        const Block& block,
        const Deposit& deposit,
        const GeologicalAnalysisResult& result,
        GostSandClass wanted)
    {
        std::size_t count = 0;
        WeightedAverageCalculator calculator;
        for (const auto& number : block.getBoreholeNumbers())
        {
            const Borehole* borehole = deposit.findBorehole(number);
            const auto* boreholeResult = findResult(result, number);
            if (!borehole || !boreholeResult) continue;
            if (boreholeResult->getIndicators().getUsefulMineralThickness() <= EPS) continue;
            const double module = calculator.calculateGOSTParameterAverage(
                *borehole,
                boreholeResult->getSectionAnalysis(),
                boreholeResult->getGeology(),
                "Модуль крупности песка");
            if (classFromModule(module) == wanted) ++count;
        }
        return count;
    }

    std::size_t totalUsefulSamples(
        const Block& block,
        const Deposit& deposit,
        const GeologicalAnalysisResult& result)
    {
        std::size_t total = 0;
        for (const auto& number : block.getBoreholeNumbers())
        {
            const Borehole* borehole = deposit.findBorehole(number);
            const auto* r = findResult(result, number);
            if (!borehole || !r) continue;
            for (const auto& sample : borehole->getSamples())
                if (sample.getThickness() > EPS && usefulThickness(sample, r->getGeology()) > EPS)
                    ++total;
        }
        return total;
    }

    std::size_t totalUsefulIntersections(
        const Block& block,
        const Deposit& deposit,
        const GeologicalAnalysisResult& result)
    {
        std::size_t total = 0;
        std::set<std::string> unique;
        for (const auto& number : block.getBoreholeNumbers())
        {
            if (!unique.insert(number).second) continue;
            const Borehole* borehole = deposit.findBorehole(number);
            const auto* r = findResult(result, number);
            if (borehole && r && r->getIndicators().getUsefulMineralThickness() > EPS) ++total;
        }
        return total;
    }

    GostSandClass wantedClass(const std::string& name)
    {
        if (name == "Повышенной крупности") return GostSandClass::IncreasedCoarseness;
        if (name == "Крупный") return GostSandClass::Coarse;
        if (name == "Средний") return GostSandClass::Medium;
        if (name == "Мелкий") return GostSandClass::Small;
        if (name == "Очень мелкий") return GostSandClass::VerySmall;
        if (name == "Тонкий") return GostSandClass::Fine;
        return GostSandClass::VeryFine;
    }
}

void SandGroupDistributionWriter::write(
    OpenXLSX::XLWorksheet& sheet,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult) const
{
    const std::vector<std::string> names = {
        "Повышенной крупности", "Крупный", "Средний", "Мелкий",
        "Очень мелкий", "Тонкий", "Очень тонкий"};

    // Строка 3 — номера блоков; строка 4 — количество проб / пересечений;
    // строка 5 — сквозная нумерация колонок.
    for (std::size_t i = 0; i < deposit.getBlocks().size(); ++i)
    {
        const auto& block = deposit.getBlocks()[i];
        const uint16_t col = static_cast<uint16_t>(FIRST_BLOCK_COLUMN + i);
        ExcelWriteUtils::writeString(sheet, 3, col, BlockDesignation::format(block.getName()));
        const std::size_t samples = totalUsefulSamples(block, deposit, analysisResult);
        const std::size_t intersections = totalUsefulIntersections(block, deposit, analysisResult);
        ExcelWriteUtils::writeString(sheet, 4, col,
            std::to_string(samples) + " / " + std::to_string(intersections));
        ExcelWriteUtils::writeDouble(sheet, 5, col, static_cast<double>(i + 1));
    }

    for (std::size_t i = 0; i < names.size(); ++i)
    {
        const uint32_t sampleLabelRow = 6 + static_cast<uint32_t>(i) * 4;
        const uint32_t sampleRow = sampleLabelRow + 1;
        const uint32_t intersectionLabelRow = sampleRow + 1;
        const uint32_t intersectionRow = intersectionLabelRow + 1;
        const GostSandClass cls = wantedClass(names[i]);

        for (std::size_t b = 0; b < deposit.getBlocks().size(); ++b)
        {
            const auto& block = deposit.getBlocks()[b];
            const std::size_t samples = sampleCount(block, deposit, analysisResult, cls);
            const std::size_t totalSamples = totalUsefulSamples(block, deposit, analysisResult);
            const std::size_t intersections = intersectionCount(block, deposit, analysisResult, cls);
            const std::size_t totalIntersections = totalUsefulIntersections(block, deposit, analysisResult);
            const uint16_t col = static_cast<uint16_t>(FIRST_BLOCK_COLUMN + b);
            ExcelWriteUtils::writeString(sheet, sampleLabelRow, col, "По образцам");
            ExcelWriteUtils::writeString(sheet, sampleRow, col, cellValue(samples, totalSamples));
            ExcelWriteUtils::writeString(sheet, intersectionLabelRow, col, "По пересечениям");
            ExcelWriteUtils::writeString(sheet, intersectionRow, col, cellValue(intersections, totalIntersections));
        }
    }
}
