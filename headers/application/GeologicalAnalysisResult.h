#pragma once

#include "BoreholeAnalysisResult.h"

#include <vector>

class GeologicalAnalysisResult
{
public:
    const std::vector<BoreholeAnalysisResult>& getBoreholes() const;
    std::vector<BoreholeAnalysisResult>& getBoreholes();

    void setBoreholes(const std::vector<BoreholeAnalysisResult>& results);

    void addBorehole(const BoreholeAnalysisResult& result);

private:
    std::vector<BoreholeAnalysisResult> boreholes;
};
