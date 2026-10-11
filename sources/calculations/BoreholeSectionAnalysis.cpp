#include "../../headers/calculations/BoreholeSectionAnalysis.h"

#include <cmath>

BoreholeSectionAnalysis::BoreholeSectionAnalysis(const std::string& boreholeNumber)
    : boreholeNumber(boreholeNumber) {}

const std::string& BoreholeSectionAnalysis::getBoreholeNumber() const
{
    return boreholeNumber;
}

void BoreholeSectionAnalysis::setBoreholeNumber(const std::string& value)
{
    boreholeNumber = value;
}

const SectionClassificationResult& BoreholeSectionAnalysis::getClassification() const
{
    return classification;
}

SectionClassificationResult& BoreholeSectionAnalysis::getClassification()
{
    return classification;
}

const std::vector<SectionDecision>& BoreholeSectionAnalysis::getDecisions() const
{
    return decisions;
}

std::vector<SectionDecision>& BoreholeSectionAnalysis::getDecisions()
{
    return decisions;
}

const FinalBoreholeSections& BoreholeSectionAnalysis::getFinalSections() const
{
    return finalSections;
}

FinalBoreholeSections& BoreholeSectionAnalysis::getFinalSections()
{
    return finalSections;
}

bool BoreholeSectionAnalysis::hasAmbiguousSections() const
{
    return classification.hasAmbiguousSections();
}

bool BoreholeSectionAnalysis::isResolved() const
{
    return resolved;
}

void BoreholeSectionAnalysis::setClassification(const SectionClassificationResult& value)
{
    classification = value;
    resolved = false;
    finalSections = FinalBoreholeSections{};
}

void BoreholeSectionAnalysis::addDecision(const SectionDecision& decision)
{
    decisions.push_back(decision);
    resolved = false;
}

void BoreholeSectionAnalysis::resolve(const BoreholeSectionResolver& resolver)
{
    finalSections =resolver.resolve(classification, decisions);

    /*
     * Если неоднозначных участков нет, скважина считается полностью разрешённой.
     * Если неоднозначные участки есть, проверяем, что для каждого из них действительно принято решение.
     */
    if (!classification.hasAmbiguousSections())
    {
        resolved = true;
        return;
    }

    resolved = true;

    for (const BoreholeSection& ambiguousSection : classification.getAmbiguousSections())
    {
        bool decisionFound = false;

        for (const SectionDecision& decision : decisions)
        {
            if (std::abs(decision.getStartDepth() - ambiguousSection.getStartDepth()) < 1e-9 &&
                std::abs(decision.getEndDepth() - ambiguousSection.getEndDepth()) < 1e-9)
            {
                decisionFound = true;
                break;
            }
        }

        if (!decisionFound)
        {
            resolved = false;
            break;
        }
    }
}