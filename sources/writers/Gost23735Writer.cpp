#include "../../headers/writers/Gost23735Writer.h"
#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/calculations/WeightedAverageCalculator.h"
#include "../../headers/normative/NormativeAnalysis.h"

#include <OpenXLSX.hpp>
#include <set>
#include <sstream>
#include <iomanip>

namespace
{
    constexpr double EPS = 1e-9;

    const BoreholeAnalysisResult* findAnalysis(
        const GeologicalAnalysisResult& analysis,
        const std::string& number)
    {
        for (const auto& item : analysis.getBoreholes())
            if (item.getBoreholeNumber() == number)
                return &item;
        return nullptr;
    }
}

void Gost23735Writer::write(
    OpenXLSX::XLWorksheet& sheet,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult) const
{
    WeightedAverageCalculator calculator;
    uint32_t row = START_ROW;

    for (const auto& block : deposit.getBlocks())
    {
        std::set<std::string> uniqueIntersections;
        std::vector<std::string> matchingIntersections;

        for (const auto& number : block.getBoreholeNumbers())
        {
            if (!uniqueIntersections.insert(number).second)
                continue;

            const Borehole* borehole = deposit.findBorehole(number);
            const auto* analysis = findAnalysis(analysisResult, number);
            if (!borehole || !analysis ||
                analysis->getIndicators().getUsefulMineralThickness() <= EPS)
                continue;

            const double module = calculator.calculateGOSTParameterAverage(
                *borehole, analysis->getSectionAnalysis(), analysis->getGeology(),
                "Модуль крупности песка");
            const double gravel5 = calculator.calculateGOSTParameterAverage(
                *borehole, analysis->getSectionAnalysis(), analysis->getGeology(),
                "Грансостав гравия >5 мм");
            const double less016 = calculator.calculateGOSTParameterAverage(
                *borehole, analysis->getSectionAnalysis(), analysis->getGeology(),
                "Грансостав песка <0.16 мм");
            const double clayey = calculator.calculateGOSTParameterAverage(
                *borehole, analysis->getSectionAnalysis(), analysis->getGeology(),
                "Глинистые в породе, %");

            if (NormativeAnalysis::gost23735Values(
                    module, gravel5, less016, clayey))
            {
                matchingIntersections.push_back(number);
            }
        }

        const std::size_t total = uniqueIntersections.size();
        const std::size_t matching = matchingIntersections.size();
        const double percentage = total > 0
            ? 100.0 * static_cast<double>(matching) / total
            : 0.0;

        std::ostringstream countText;
        countText << matching << " (" << std::fixed << std::setprecision(2)
                  << percentage << "%)";

        std::ostringstream numbers;
        for (std::size_t i = 0; i < matchingIntersections.size(); ++i)
        {
            if (i > 0) numbers << ", ";
            numbers << matchingIntersections[i];
        }

        ExcelWriteUtils::writeString(
            sheet, row, 1,
            block.getName() + " (" + std::to_string(total) + ")");
        ExcelWriteUtils::writeString(sheet, row, 2, countText.str());
        ExcelWriteUtils::writeString(sheet, row, 3, numbers.str());
        ++row;
    }
}
