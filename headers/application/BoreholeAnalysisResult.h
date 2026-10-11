#pragma once

#include "../calculations/BoreholeSectionAnalysis.h"
#include "../geology/BoreholeGeology.h"
#include "../results/MainIndicators.h"

#include <string>

class BoreholeAnalysisResult
{
public:
    BoreholeAnalysisResult() = default;

    explicit BoreholeAnalysisResult(const std::string& boreholeNumber);

    const std::string& getBoreholeNumber() const;
    const BoreholeSectionAnalysis& getSectionAnalysis() const;
    BoreholeSectionAnalysis& getSectionAnalysis();
    const BoreholeGeology& getGeology() const;
    BoreholeGeology& getGeology();
    const MainIndicators& getIndicators() const;
    MainIndicators& getIndicators();

    void setBoreholeNumber(const std::string& boreholeNumber);
    void setSectionAnalysis(const BoreholeSectionAnalysis& analysis);
    void setGeology(const BoreholeGeology& geology);
    void setIndicators(const MainIndicators& indicators);

private:
    std::string boreholeNumber;

    BoreholeSectionAnalysis sectionAnalysis;

    BoreholeGeology geology;

    MainIndicators indicators;
};
