#pragma once

#include "../application/GeologicalAnalysisResult.h"
#include "../domain/Deposit.h"

namespace OpenXLSX
{
    class XLWorksheet;
}

class AnaliticsWriter
{
public:
    static constexpr unsigned int SHEET_NUMBER = 15;
    static constexpr unsigned int START_ROW = 5;

    void write(
        OpenXLSX::XLWorksheet& sheet,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult) const;
};
