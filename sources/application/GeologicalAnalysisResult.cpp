#include "../../headers/application/GeologicalAnalysisResult.h"


const std::vector<BoreholeAnalysisResult>& GeologicalAnalysisResult::getBoreholes() const
{
    return boreholes;
}

std::vector<BoreholeAnalysisResult>& GeologicalAnalysisResult::getBoreholes()
{
    return boreholes;
}

void GeologicalAnalysisResult::setBoreholes(const std::vector<BoreholeAnalysisResult>& results)
{
    boreholes = results;
}

void GeologicalAnalysisResult::addBorehole(const BoreholeAnalysisResult& result)
{
    boreholes.push_back(result);
}