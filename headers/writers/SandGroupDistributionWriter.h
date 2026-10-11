#pragma once

#include "../application/GeologicalAnalysisResult.h"
#include "../domain/Deposit.h"

#include <cstdint>
#include <string>

namespace OpenXLSX { class XLWorksheet; }

class SandGroupDistributionWriter
{
public:
    static constexpr uint16_t SHEET_NUMBER = 16;

    void write(
        OpenXLSX::XLWorksheet& sheet,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult) const;

private:
    static constexpr uint32_t BLOCK_ROW = 5;
    static constexpr uint16_t FIRST_BLOCK_COLUMN = 3;

    void writeGroup(
        OpenXLSX::XLWorksheet& sheet,
        uint32_t sampleRow,
        uint32_t intersectionRow,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        const std::string& className) const;
};
