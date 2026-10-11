#include "../../headers/writers/Gost8736RequirementsWriter.h"
#include "../../headers/calculations/WeightedAverageCalculator.h"
#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/domain/Borehole.h"
#include "../../headers/normative/NormativeAnalysis.h"

#include <OpenXLSX.hpp>
#include <iomanip>
#include <set>
#include <sstream>

namespace
{
    constexpr double EPS = 1e-9;

    const BoreholeAnalysisResult* findResult(
        const GeologicalAnalysisResult& result,
        const std::string& number)
    {
        for (const auto& item : result.getBoreholes())
            if (item.getBoreholeNumber() == number)
                return &item;
        return nullptr;
    }

    std::string value(std::size_t count, std::size_t total)
    {
        std::ostringstream out;
        out << count << " (" << std::fixed << std::setprecision(2)
            << (total > 0 ? 100.0 * static_cast<double>(count) / total : 0.0)
            << "%)";
        return out.str();
    }
}

void Gost8736RequirementsWriter::write(
    OpenXLSX::XLWorksheet& sheet,
    const Deposit& deposit,
    const GeologicalAnalysisResult& result) const
{
    WeightedAverageCalculator calculator;
    uint32_t row = START_ROW;

    for (const auto& block : deposit.getBlocks())
    {
        std::set<std::string> unique;
        std::size_t total = 0;
        std::size_t residue063 = 0;
        std::size_t less016 = 0;
        std::size_t clayey = 0;

        for (const auto& number : block.getBoreholeNumbers())
        {
            if (!unique.insert(number).second)
                continue;

            const Borehole* borehole = deposit.findBorehole(number);
            const auto* boreholeResult = findResult(result, number);

            if (!borehole || !boreholeResult ||
                boreholeResult->getIndicators().getUsefulMineralThickness() <= EPS)
                continue;

            ++total;

            const double module = calculator.calculateGOSTParameterAverage(
                *borehole, boreholeResult->getSectionAnalysis(),
                boreholeResult->getGeology(), "Модуль крупности песка");
            const double s063 = calculator.calculateGOSTParameterAverage(
                *borehole, boreholeResult->getSectionAnalysis(),
                boreholeResult->getGeology(), "Грансостав песка 0.63 мм");
            const double less = calculator.calculateGOSTParameterAverage(
                *borehole, boreholeResult->getSectionAnalysis(),
                boreholeResult->getGeology(), "Грансостав песка <0.16 мм");
            const double clay = calculator.calculateGOSTParameterAverage(
                *borehole, boreholeResult->getSectionAnalysis(),
                boreholeResult->getGeology(), "Глинистые в породе, %");

            if (NormativeAnalysis::gost8736FullResidue063(module, s063))
                ++residue063;
            if (NormativeAnalysis::gost8736ClassIILess016(module, less))
                ++less016;
            if (NormativeAnalysis::gost8736ClassIIClayey(module, clay))
                ++clayey;
        }

        ExcelWriteUtils::writeString(
            sheet, row, 1,
            block.getName() + " (" + std::to_string(total) + ")");
        ExcelWriteUtils::writeString(sheet, row, 2, value(residue063, total));
        ExcelWriteUtils::writeString(sheet, row, 3, value(less016, total));
        ExcelWriteUtils::writeString(sheet, row, 4, value(clayey, total));
        ++row;
    }
}
