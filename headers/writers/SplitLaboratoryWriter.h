#pragma once

#include "../domain/Deposit.h"
#include "../application/GeologicalAnalysisResult.h"

namespace OpenXLSX
{
    class XLDocument;
    class XLWorksheet;
}

class SplitLaboratoryWriter
{
public:
    void write(
        OpenXLSX::XLDocument& document,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult) const;

private:
    void writeGost(
        OpenXLSX::XLWorksheet& sheet,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult) const;

    void writeTkp(
        OpenXLSX::XLWorksheet& sheet,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult) const;
};
