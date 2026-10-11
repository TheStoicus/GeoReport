#pragma once

#include "../dxf/DxfDocument.h"
#include "../../application/GeologicalAnalysisResult.h"
#include "../../domain/Deposit.h"
#include "../../settings/ReserveSummarySettings.h"

/*
 * Общий построитель плана расположения скважин.
 *
 * Используется двумя графическими приложениями:
 *   1. boreholes.dxf  — скважины и показатели без контуров;
 *   2. block_plan.dwg — те же данные + контуры блоков.
 */
class BoreholePlanDrawingBuilder
{
public:
    void build(
        DxfDocument& document,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        const ReserveSummarySettings& reserveSummarySettings,
        bool includeBlockContours) const;
};
