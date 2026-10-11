#pragma once

#include "../application/GeologicalAnalysisResult.h"
#include "../domain/Deposit.h"
#include "../results/WeightedAverageResult.h"

#include <string>

namespace OpenXLSX { class XLWorksheet; }

class IndustrialTypesWriter
{
public:
    static constexpr unsigned int SHEET_NUMBER = 13;
    static constexpr unsigned int FIRST_COLUMN = 2;

    void write(
        OpenXLSX::XLWorksheet& sheet,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult) const;

private:
    struct CriterionResult
    {
        int matching = 0;
        int total = 0;
        double percentage() const;
    };

    static bool overlapsUsefulMineral(
        const Sample& sample,
        const BoreholeAnalysisResult& result);

    static std::string baseSampleNumber(const std::string& number);

    static bool matchesType1(const Sample& sample);
    static bool matchesType2(const Sample& sample);
    static bool matchesType1(const WeightedAverageResult& result);
    static bool matchesType2(const WeightedAverageResult& result);

    static CriterionResult calculateSamples(
        const Block& block,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysis,
        bool type1);

    static CriterionResult calculateBoreholes(
        const Block& block,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysis,
        bool type1);

    static std::string formatCriterion(const CriterionResult& result);
};
