#pragma once

#include "../application/GeologicalAnalysisResult.h"
#include "../domain/Deposit.h"
#include "../settings/MainIndicatorsSettings.h"
#include "../settings/GroundwaterSettings.h"
#include "../settings/GroundwaterCalculationSettings.h"
#include "../settings/DevelopmentSettings.h"
#include "../settings/ReserveSummarySettings.h"

#include <filesystem>
#include <string>

class DepositStatistics
{
public:
    void write(
        const std::filesystem::path& outputPath,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        const MainIndicatorsSettings& mainIndicatorsSettings,
        const GroundwaterSettings& groundwaterSettings,
        const GroundwaterCalculationSettings& groundwaterCalculationSettings,
        const DevelopmentSettings& developmentSettings,
        const ReserveSummarySettings& reserveSummarySettings) const;
};
