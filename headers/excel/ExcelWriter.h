#pragma once

#include "../application/GeologicalAnalysisResult.h"
#include "../domain/Deposit.h"
#include "../settings/ReserveSummarySettings.h"

#include <string>


class ExcelWriter
{
public:
    void write(
        const std::string& templatePath,
        const std::string& outputPath,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        const ReserveSummarySettings& reserveSummarySettings) const;
};