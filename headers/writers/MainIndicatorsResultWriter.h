#pragma once

#include "../calculations/BoreholeSectionAnalysis.h"
#include "../domain/Deposit.h"
#include "../domain/Borehole.h"
#include "../results/MainIndicators.h"

#include <cstdint>
#include <string>
#include <vector>

namespace OpenXLSX
{
    class XLWorksheet;
}

class MainIndicatorsResultWriter
{
public:
    static constexpr uint32_t START_ROW = 6;

    void write(OpenXLSX::XLWorksheet& sheet, const Deposit& deposit,
        const std::vector<BoreholeSectionAnalysis>& analyses,
        const std::vector<MainIndicators>& indicators) const;

private:
    static constexpr uint16_t COL_SERIAL = 1;
    static constexpr uint16_t COL_BLOCK = 2;
    static constexpr uint16_t COL_BOREHOLE = 3;
    static constexpr uint16_t COL_BOREHOLE_DEPTH = 4;
    static constexpr uint16_t COL_COLLAR_ELEVATION = 5;
    static constexpr uint16_t COL_OVERBURDEN = 6;
    static constexpr uint16_t COL_PRS = 7;
    static constexpr uint16_t COL_ROOF_ELEVATION = 8;
    static constexpr uint16_t COL_MINERAL_THICKNESS = 9;
    static constexpr uint16_t COL_DRY_MINERAL_THICKNESS = 10;
    static constexpr uint16_t COL_WATER_BEARING_MINERAL_THICKNESS = 11;
    static constexpr uint16_t COL_BOTTOM_ELEVATION = 12;
    static constexpr uint16_t COL_GROUNDWATER_DEPTH = 13;
    static constexpr uint16_t COL_GROUNDWATER_ELEVATION = 14;
    static constexpr uint16_t COL_UNDERLYING_THICKNESS = 15;
    static constexpr uint16_t COL_UNDERLYING_NAME = 18;
    static constexpr uint16_t COL_DRY_ROOF_ELEVATION = 19;
    static constexpr uint16_t COL_DRY_BOTTOM_ELEVATION = 20;
    static constexpr uint16_t COL_WATER_BEARING_ROOF_ELEVATION = 21;
    static constexpr uint16_t COL_WATER_BEARING_BOTTOM_ELEVATION = 22;

    const BoreholeSectionAnalysis* findAnalysis(const std::vector<BoreholeSectionAnalysis>& analyses,
        const std::string& boreholeNumber) const;

    std::string getBlockNames(const Deposit& deposit, const std::string& boreholeNumber) const;

    double calculatePRSThickness(const Borehole& borehole) const;

    void writeRow(OpenXLSX::XLWorksheet& sheet, uint32_t row, int serialNumber, 
        const std::string& blockNames, const MainIndicators& indicators, double prsThickness) const;

    void writeTotalsRow(OpenXLSX::XLWorksheet& sheet, uint32_t row, const std::vector<MainIndicators>& indicators,
        const std::vector<double>& prsThicknesses) const;

    void writeAverageRow(OpenXLSX::XLWorksheet& sheet, uint32_t row, const std::vector<MainIndicators>& indicators,
        const std::vector<double>& prsThicknesses) const;

    static double roundHalfUp(double value, int decimals);
    static double roundToOneDecimal(double value);
};
