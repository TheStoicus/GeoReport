#pragma once

#include "../application/GeologicalAnalysisResult.h"
#include "../domain/Deposit.h"

namespace OpenXLSX
{
    class XLWorksheet;
}

class Gost23735Writer
{
public:
    static constexpr unsigned int SHEET_NUMBER = 14;
    static constexpr unsigned int START_ROW = 3;

    void write(
        OpenXLSX::XLWorksheet& sheet,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult) const;
};
