#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../calculations/GranulometricCompositionCalculator.h"

namespace OpenXLSX
{
    class XLWorksheet;
}

class GranulometricCompositionWriter
{
public:
    static constexpr uint32_t START_ROW = 7;

    void write(OpenXLSX::XLWorksheet& sheet, const Deposit& deposit,
        const std::vector<BoreholeSectionAnalysis>& analyses,
        const std::vector<BoreholeAnalysisResult>& geologyResults) const;

private:
    // В третьей таблице 16 показателей. Запись начинается с первого столбца: A ... P
    static constexpr uint16_t COL_LABEL = 1;
    static constexpr uint16_t COL_FIRST_PARAMETER = 1;
    static constexpr uint16_t COL_LAST_PARAMETER = 16;

    uint32_t writeBlock(OpenXLSX::XLWorksheet& sheet, uint32_t row, const GranulometricCompositionBlockResult& result) const;
    void writeRangeRow( OpenXLSX::XLWorksheet& sheet, uint32_t row, const std::vector<GranulometricParameterRange>& ranges) const;
    void writeBlockAverageRow(OpenXLSX::XLWorksheet& sheet, uint32_t row, const WeightedAverageResult& result) const;

    // Записывает текст в первую ячейку строки и объединяет всю строку таблицы A:P.
    static void writeMergedRow(OpenXLSX::XLWorksheet& sheet, uint32_t row, const std::string& text);
    static std::string formatRange(const GranulometricParameterRange& range);
};
