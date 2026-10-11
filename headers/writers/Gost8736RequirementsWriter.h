#pragma once

#include "../application/GeologicalAnalysisResult.h"
#include "../domain/Deposit.h"

#include <cstdint>

namespace OpenXLSX { class XLWorksheet; }

class Gost8736RequirementsWriter
{
public:
    static constexpr uint16_t SHEET_NUMBER = 17;
    static constexpr uint32_t START_ROW = 5;

    void write(OpenXLSX::XLWorksheet& sheet,
               const Deposit& deposit,
               const GeologicalAnalysisResult& result) const;
};
