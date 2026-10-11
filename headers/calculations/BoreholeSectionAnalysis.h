#pragma once

#include "../geology/BoreholeSectionResolver.h"
#include "../geology/SectionClassifier.h"
#include "../geology/SectionDecision.h"

#include <string>
#include <vector>

class BoreholeSectionAnalysis
{
public:
    BoreholeSectionAnalysis() = default;

    explicit BoreholeSectionAnalysis(const std::string& boreholeNumber);

    const std::string& getBoreholeNumber() const;

    void setBoreholeNumber(const std::string& boreholeNumber);
    void setClassification(const SectionClassificationResult& classification);

    const SectionClassificationResult& getClassification() const;
    SectionClassificationResult& getClassification();
    const std::vector<SectionDecision>& getDecisions() const;
    std::vector<SectionDecision>& getDecisions();
    const FinalBoreholeSections& getFinalSections() const;
    FinalBoreholeSections& getFinalSections();

    bool hasAmbiguousSections() const;

    bool isResolved() const;

    void addDecision(const SectionDecision& decision);

    void resolve(const BoreholeSectionResolver& resolver);

private:
    std::string boreholeNumber;

    SectionClassificationResult classification;

    std::vector<SectionDecision> decisions;

    FinalBoreholeSections finalSections;

    bool resolved = false;
};
