#pragma once

#include "../../application/GeologicalAnalysisResult.h"
#include "../../domain/Deposit.h"
#include "../../settings/SectionSettings.h"
#include "DxfDocument.h"

class DxfGeologicalSectionWriter
{
public:
    void write(
        const std::string& filePath,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        const SectionSettings& sectionSettings) const;

private:
    static constexpr double VERTICAL_SCALE = 10.0;
    static constexpr double BOREHOLE_HALF_WIDTH = 2.5;
    static constexpr double TEXT_HEIGHT = 2.5;
    static constexpr double SCALE_OFFSET = 12.0;
};
