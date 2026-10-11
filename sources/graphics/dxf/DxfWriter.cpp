#include "../../../headers/graphics/dxf/DxfWriter.h"

#include "../../../headers/graphics/cad/BoreholePlanDrawingBuilder.h"

void DxfWriter::write(
    const std::string& filePath,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult,
    const ReserveSummarySettings& reserveSummarySettings) const
{
    DxfDocument document;
    BoreholePlanDrawingBuilder builder;

    // boreholes.dxf намеренно остаётся планом без контуров блоков.
    // Контуры теперь находятся только во втором графическом приложении DWG.
    builder.build(
        document,
        deposit,
        analysisResult,
        reserveSummarySettings,
        false);

    document.save(filePath);
}
