#pragma once

#include "../application/GeologicalAnalysisResult.h"
#include "../domain/Deposit.h"

#include <filesystem>
#include <string>

class DepositStatistics
{
public:
    void write(
        const std::filesystem::path& outputPath,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult) const;
};
