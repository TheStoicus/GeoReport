#pragma once

#include "../application/GeologicalAnalysisResult.h"
#include "../calculations/BoreholeSectionAnalysis.h"
#include "../domain/Deposit.h"
#include "../settings/ReserveSummarySettings.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace OpenXLSX
{
    class XLWorksheet;
}

/*
 * Заполняет сводную таблицу подсчёта запасов — лист 7 ("Т. 6").
 * Данные начинаются со строки 6.
 */
class ReserveSummaryWriter
{
public:
    static constexpr uint32_t START_ROW = 6;

    void write(
        OpenXLSX::XLWorksheet& sheet,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        const ReserveSummarySettings& settings) const;

private:
    struct CalculationRow
    {
        std::string name;
        double area = 0.0;
        std::string blockNumber;
        std::string subblock;
        std::string category;
        double averageOverburden = 0.0;
        double averagePRS = 0.0;
        double averageMineral = 0.0;
        double gravelAverage = 0.0;
        double overburdenVolume = 0.0;
        double prsVolume = 0.0;
        double mineralVolume = 0.0;
        double gravelVolume = 0.0;
        bool hasMineral = false;
        bool hasGravelAverage = false;
    };

    enum class MineralPart
    {
        Total,
        Dry,
        WaterBearing
    };

    static const BoreholeAnalysisResult* findBoreholeResult(
        const GeologicalAnalysisResult& analysisResult,
        const std::string& boreholeNumber);

    static double calculatePRSThickness(const Borehole& borehole);

    static double calculatePartThickness(
        const BoreholeGeology& geology,
        MineralPart part);

    static bool hasPart(
        const BoreholeGeology& geology,
        MineralPart part);

    static std::vector<const BoreholeAnalysisResult*> collectResultsForBlock(
        const Block& block,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult);

    static double calculateAverageOverburden(
        const std::vector<const BoreholeAnalysisResult*>& results,
        const Deposit& deposit);

    static double calculateAveragePRS(
        const std::vector<const BoreholeAnalysisResult*>& results,
        const Deposit& deposit);

    static double calculateAverageMineral(
        const std::vector<const BoreholeAnalysisResult*>& results,
        MineralPart part);

    static double calculateGravelAverage(
        const Block& block,
        const Deposit& deposit,
        const std::vector<const BoreholeAnalysisResult*>& results,
        MineralPart part,
        bool& hasValue);

    static bool hasPartInBlock(
        const std::vector<const BoreholeAnalysisResult*>& results,
        MineralPart part);

    static CalculationRow calculateRow(
        const Block& block,
        const ReserveSummaryBlockSettings& blockSettings,
        const ReserveSummarySettings& settings,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        MineralPart part,
        const std::string& rowName);

    static void writeRow(
        OpenXLSX::XLWorksheet& sheet,
        uint32_t row,
        const CalculationRow& value);

    static double calculateBlockArea(
        const Block& block,
        const Deposit& deposit,
        const ReserveSummarySettings& settings);

    static std::pair<std::string, std::string> parseBlockDesignation(
        const std::string& blockName);

    static double roundToTwoDecimals(double value);
};
