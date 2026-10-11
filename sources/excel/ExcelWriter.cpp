#include "../../headers/excel/ExcelWriter.h"

#include "../../headers/writers/BlockResultWriter.h"
#include "../../headers/writers/BoreholeResultWriter.h"
#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/writers/GranulometricCompositionWriter.h"
#include "../../headers/writers/MainIndicatorsResultWriter.h"
#include "../../headers/writers/SplitLaboratoryWriter.h"
#include "../../headers/writers/ReserveSummaryWriter.h"
#include "../../headers/settings/ReserveSummarySettings.h"
#include "../../headers/writers/AnaliticsWriter.h"
#include "../../headers/writers/IndustrialTypesWriter.h"
#include "../../headers/writers/Gost23735Writer.h"
#include "../../headers/writers/SandGroupDistributionWriter.h"
#include "../../headers/writers/Gost8736RequirementsWriter.h"

#include <OpenXLSX.hpp>

#include <filesystem>
#include <vector>


void ExcelWriter::write(
    const std::string& templatePath,
    const std::string& outputPath,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult,
    const ReserveSummarySettings& reserveSummarySettings) const
{
    /*
     * Результат всегда создаётся на основе шаблона.
     * template.xlsx остаётся неизменным.
     * Все результаты записываются в result.xlsx.
     */
    // Сначала формируем результат во временном файле.
    // Это не позволяет оставить старый/частично записанный result.xlsx,
    // если библиотека OpenXLSX завершит сохранение с ошибкой.
    const std::filesystem::path finalPath(outputPath);
    const std::filesystem::path temporaryPath =
        finalPath.parent_path() / (finalPath.stem().string() + ".tmp.xlsx");

    std::filesystem::copy_file(
        templatePath,
        temporaryPath,
        std::filesystem::copy_options::overwrite_existing);


    OpenXLSX::XLDocument document;

    document.open(temporaryPath.string());


    // ============================================================
    // Подготовка данных геологического анализа
    // ============================================================

    /*
     * GeologicalAnalysisResult хранит полный результат
     * по каждой скважине:
     *
     *   BoreholeSectionAnalysis
     *   BoreholeGeology
     *   MainIndicators
     *
     * MainIndicatorsResultWriter пока работает с двумя
     * отдельными векторами, поэтому формируем их здесь.
     */

    std::vector<BoreholeSectionAnalysis> analyses;

    std::vector<MainIndicators> indicators;

    analyses.reserve(
        analysisResult.getBoreholes().size());

    indicators.reserve(
        analysisResult.getBoreholes().size());


    for (const BoreholeAnalysisResult& boreholeResult :
         analysisResult.getBoreholes())
    {
        analyses.push_back(
            boreholeResult.getSectionAnalysis());

        indicators.push_back(
            boreholeResult.getIndicators());
    }


    // ============================================================
    // Лист 1 — основные показатели
    // ============================================================

    OpenXLSX::XLWorksheet mainIndicatorsSheet =
        document.workbook().worksheet(1);

    MainIndicatorsResultWriter mainIndicatorsWriter;

    mainIndicatorsWriter.write(
        mainIndicatorsSheet,
        deposit,
        analyses,
        indicators);


    // Лабораторные листы после разделения проб.
    SplitLaboratoryWriter splitLaboratoryWriter;
    splitLaboratoryWriter.write(
        document,
        deposit,
        analysisResult);


    // ============================================================
    // Лист 4 — результаты по пробам / скважинам
    // ============================================================

    OpenXLSX::XLWorksheet boreholeSheet =
        document.workbook().worksheet(4);

    BoreholeResultWriter boreholeWriter;

    uint32_t boreholeRow =
        BoreholeResultWriter::START_ROW;

    int boreholeSerialNumber = 1;

    for (const Borehole& borehole :
         deposit.getBoreholes())
    {
        const BoreholeSectionAnalysis* analysis = nullptr;

        for (const BoreholeSectionAnalysis& candidate : analyses)
        {
            if (candidate.getBoreholeNumber() == borehole.getBoreholeNumber())
            {
                analysis = &candidate;
                break;
            }
        }

        if (analysis == nullptr || !analysis->isResolved())
            continue;

        const BoreholeGeology* geology = nullptr;

        for (const BoreholeAnalysisResult& candidate :
             analysisResult.getBoreholes())
        {
            if (candidate.getBoreholeNumber() ==
                borehole.getBoreholeNumber())
            {
                geology = &candidate.getGeology();
                break;
            }
        }

        if (geology == nullptr)
            continue;

        boreholeWriter.write(
            boreholeSheet,
            borehole,
            *analysis,
            *geology,
            boreholeRow,
            boreholeSerialNumber);
    }


    // ============================================================
    // Лист 5 — результаты по блокам
    // ============================================================

    OpenXLSX::XLWorksheet blockSheet =
        document.workbook().worksheet(5);

    BlockResultWriter blockWriter;

    blockWriter.write(
        blockSheet,
        deposit,
        analyses,
        analysisResult.getBoreholes());


    // ============================================================
    // Лист 7 — сводная таблица подсчёта запасов
    // ============================================================

    OpenXLSX::XLWorksheet reserveSummarySheet =
        document.workbook().worksheet(7);

    ReserveSummaryWriter reserveSummaryWriter;

    reserveSummaryWriter.write(
        reserveSummarySheet,
        deposit,
        analysisResult,
        reserveSummarySettings);

    // ============================================================
    // Лист 9 — гранулометрический состав
    // ============================================================

    OpenXLSX::XLWorksheet granulometricSheet =
        document.workbook().worksheet(9);

    GranulometricCompositionWriter granulometricWriter;

    granulometricWriter.write(
        granulometricSheet,
        deposit,
        analyses,
        analysisResult.getBoreholes());


    // ============================================================
    // Лист 13 — соответствие промышленным типам
    // ============================================================
    {
        OpenXLSX::XLWorksheet industrialTypesSheet =
            document.workbook().worksheet(IndustrialTypesWriter::SHEET_NUMBER);
        IndustrialTypesWriter industrialTypesWriter;
        industrialTypesWriter.write(
            industrialTypesSheet,
            deposit,
            analysisResult);
    }

    // ============================================================
    // Лист 14 — соответствие пересечений ГОСТ 23735-2014
    // ============================================================
    {
        OpenXLSX::XLWorksheet gost23735Sheet =
            document.workbook().worksheet(Gost23735Writer::SHEET_NUMBER);
        Gost23735Writer gost23735Writer;
        gost23735Writer.write(
            gost23735Sheet,
            deposit,
            analysisResult);
    }

    // ============================================================
    // Лист 16 — распределение песков по модулю крупности ГОСТ 8736
    // ============================================================
    {
        OpenXLSX::XLWorksheet sheet =
            document.workbook().worksheet(SandGroupDistributionWriter::SHEET_NUMBER);
        SandGroupDistributionWriter writer;
        writer.write(sheet, deposit, analysisResult);
    }

    // ============================================================
    // Лист 17 — соответствие требованиям ГОСТ 8736
    // ============================================================
    {
        OpenXLSX::XLWorksheet sheet =
            document.workbook().worksheet(Gost8736RequirementsWriter::SHEET_NUMBER);
        Gost8736RequirementsWriter writer;
        writer.write(sheet, deposit, analysisResult);
    }

    // ============================================================
    // Лист 15 — соответствие образцов требованиям ТНПА
    // ============================================================
    {
        OpenXLSX::XLWorksheet analiticsSheet =
            document.workbook().worksheet(AnaliticsWriter::SHEET_NUMBER);
        AnaliticsWriter analiticsWriter;
        analiticsWriter.write(analiticsSheet, deposit, analysisResult);
    }

    // ============================================================
    // Сохранение
    // ============================================================

    document.save();
    document.close();

    // Только после успешного save() заменяем итоговый файл.
    std::filesystem::rename(temporaryPath, finalPath);
}

