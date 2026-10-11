#pragma once

#include <cstdint>

#include "../domain/Borehole.h"
#include "../geology/BoreholeGeology.h"
#include "../calculations/BoreholeSectionAnalysis.h"
#include "../results/WeightedAverageResult.h"

namespace OpenXLSX
{
    class XLWorksheet;
}

class BoreholeResultWriter
{
public:
    static constexpr uint32_t START_ROW = 6;

    void write(
        OpenXLSX::XLWorksheet& sheet,
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const BoreholeGeology& geology,
        uint32_t& row,
        int& serialNumber) const;

private:
    void writeSample(
        OpenXLSX::XLWorksheet& sheet,
        const Borehole& borehole,
        const Sample& sample,
        uint32_t row,
        int serialNumber) const;

    void writeWeightedAverage(
        OpenXLSX::XLWorksheet& sheet,
        uint32_t row,
        const WeightedAverageResult& result) const;

    static double getGOSTValue(
        const Sample& sample,
        const std::string& parameterName);

    static bool isMonolith(const Sample& sample);

    static double countedUsefulOverlapThickness(
        const Sample& sample,
        const BoreholeGeology& geology);
};
