#include "../../headers/application/BoreholeAnalysisResult.h"


BoreholeAnalysisResult::BoreholeAnalysisResult(const std::string& boreholeNumber)
    : boreholeNumber(boreholeNumber){}

const std::string& BoreholeAnalysisResult::getBoreholeNumber() const
{
    return boreholeNumber;
}

void BoreholeAnalysisResult::setBoreholeNumber(const std::string& boreholeNumber)
{
    this->boreholeNumber = boreholeNumber;
}

const BoreholeSectionAnalysis& BoreholeAnalysisResult::getSectionAnalysis() const
{
    return sectionAnalysis;
}

BoreholeSectionAnalysis& BoreholeAnalysisResult::getSectionAnalysis()
{
    return sectionAnalysis;
}

void BoreholeAnalysisResult::setSectionAnalysis(const BoreholeSectionAnalysis& analysis)
{
    sectionAnalysis = analysis;
}

const BoreholeGeology& BoreholeAnalysisResult::getGeology() const
{
    return geology;
}

BoreholeGeology& BoreholeAnalysisResult::getGeology()
{
    return geology;
}

void BoreholeAnalysisResult::setGeology(const BoreholeGeology& geology)
{
    this->geology = geology;
}


const MainIndicators& BoreholeAnalysisResult::getIndicators() const
{
    return indicators;
}

MainIndicators& BoreholeAnalysisResult::getIndicators()
{
    return indicators;
}

void BoreholeAnalysisResult::setIndicators(const MainIndicators& indicators)
{
    this->indicators = indicators;
}