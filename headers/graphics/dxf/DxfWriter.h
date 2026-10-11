#pragma once

#include "../../application/GeologicalAnalysisResult.h"
#include "../../domain/Deposit.h"

#include "DxfDocument.h"
#include "../../settings/ReserveSummarySettings.h"

class DxfWriter
{
public:
    void write(
        const std::string& filePath,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        const ReserveSummarySettings& reserveSummarySettings) const;

private:
    static constexpr double BOREHOLE_RADIUS = 2.0;
    static constexpr double TEXT_HEIGHT = 2.5;
    static constexpr double LINE_LENGTH = 15.0; // Длина горизонтальной линии от центра скважины в каждую сторону.
    static constexpr double TEXT_OFFSET = 3.0; // Отступ текста от скважины по горизонтали.
    static constexpr double TEXT_VERTICAL_OFFSET = 1.5; // Расстояние от линии до текста.
    static constexpr double GROUNDWATER_OFFSET = 5.0;
};
