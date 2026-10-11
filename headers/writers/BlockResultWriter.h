#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../domain/Block.h"
#include "../domain/Deposit.h"
#include "../calculations/BoreholeSectionAnalysis.h"
#include "../application/GeologicalAnalysisResult.h"
#include "../results/WeightedAverageResult.h"
#include "../calculations/MineralCalculationPart.h"

namespace OpenXLSX
{
    class XLWorksheet;
}

class WeightedAverageCalculator;

class BlockResultWriter
{
public:
    void write(
        OpenXLSX::XLWorksheet& sheet,
        const Deposit& deposit,
        const std::vector<BoreholeSectionAnalysis>& analyses,
        const std::vector<BoreholeAnalysisResult>& geologyResults) const;

private:
    static constexpr uint32_t START_ROW = 6;
    static constexpr uint16_t COL_SERIAL = 1;
    static constexpr uint16_t COL_BOREHOLE_OR_BLOCK = 2;
    static constexpr uint16_t COL_START_DEPTH = 3;
    static constexpr uint16_t COL_END_DEPTH = 4;
    static constexpr uint16_t COL_THICKNESS = 5;
    static constexpr uint16_t COL_FIRST_PARAMETER = 6;

    bool hasPartInBlock(
        const Block& block,
        const Deposit& deposit,
        const std::vector<BoreholeSectionAnalysis>& analyses,
        const std::vector<BoreholeAnalysisResult>& geologyResults,
        const WeightedAverageCalculator& calculator,
        MineralCalculationPart part) const;

    uint32_t writeBlockResult(
        OpenXLSX::XLWorksheet& sheet,
        uint32_t row,
        uint32_t& serialNumber,
        const Block& block,
        const Deposit& deposit,
        const std::vector<BoreholeSectionAnalysis>& analyses,
        const std::vector<BoreholeAnalysisResult>& geologyResults,
        const WeightedAverageCalculator& calculator,
        MineralCalculationPart part,
        const std::string& title) const;

    static double overlap(
        double sampleStart,
        double sampleEnd,
        double sectionStart,
        double sectionEnd);
};
