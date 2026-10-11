#include "headers/excel/ExcelReader.h"

#include "headers/application/GeologicalAnalysisService.h"
#include "headers/application/GeologicalAnalysisResult.h"

#include "headers/settings/GroundwaterSettings.h"
#include "headers/settings/MainIndicatorsSettings.h"
#include "headers/settings/DevelopmentSettings.h"
#include "headers/settings/GroundwaterCalculationSettings.h"

#include "headers/excel/ExcelWriter.h"

#include "headers/graphics/dxf/DxfWriter.h"
#include "headers/graphics/dxf/DxfColumnWriter.h"
#include "headers/graphics/dxf/DxfGeologicalSectionWriter.h"
#include "headers/graphics/dwg/DwgPlanWriter.h"
#include "headers/graphics/dwg/DwgGeologicalSectionWriter.h"
#include "headers/graphics/cad/CadStandardsWriter.h"

#include "headers/settings/ConsoleSettingsReader.h"
#include "headers/settings/SectionSettings.h"
#include "headers/statistics/DepositStatistics.h"

#include <filesystem>
#include <iostream>
#include <string>


namespace
{
    std::filesystem::path findProjectRoot()
    {
        std::filesystem::path current = std::filesystem::current_path();

        for (int i = 0; i < 8; ++i)
        {
            if (std::filesystem::exists(current / "data/input/input.xlsx") &&
                std::filesystem::exists(current / "data/templates/template.xlsx"))
            {
                return current;
            }

            if (current == current.root_path())
                break;

            current = current.parent_path();
        }

        // CLion обычно запускает приложение из cmake-build-*/.
        current = std::filesystem::path(__FILE__).parent_path();
        return current;
    }
}

int main()
{
    try
    {
        const std::filesystem::path projectRoot = findProjectRoot();
        const std::filesystem::path inputPath = projectRoot / "data/input/input.xlsx";
        const std::filesystem::path templatePath = projectRoot / "data/templates/template.xlsx";
        const std::filesystem::path outputDirectory = projectRoot / "data/output";

        const std::filesystem::path excelOutputPath = outputDirectory / "result.xlsx";
        const std::filesystem::path statisticsOutputPath = outputDirectory / "deposit_statistics.txt";
        const std::filesystem::path dxfSitePlanPath = outputDirectory / "boreholes.dxf";
        const std::filesystem::path dxfColumnsPath = outputDirectory / "geological_columns.dxf";
        const std::filesystem::path dwgSectionPath = outputDirectory / "geological_lithological_section.dwg";
        const std::filesystem::path dwgBlockPlanPath = outputDirectory / "block_plan.dwg";

        std::filesystem::create_directories(outputDirectory);

        CadStandardsWriter cadStandardsWriter;
        cadStandardsWriter.write(outputDirectory);

        // 1. Исходные данные.
        ExcelReader reader;
        Deposit deposit = reader.read(inputPath.string());

        // 2. Пользовательские правила классификации.
        ConsoleSettingsReader settingsReader;
        MainIndicatorsSettings mainIndicatorsSettings = settingsReader.read(deposit);

        // 3. Правила грунтовых вод.
        GroundwaterCalculationSettings groundwaterCalculationSettings =
            settingsReader.readGroundwaterCalculationSettings();

        GroundwaterSettings groundwaterSettings =
            settingsReader.readGroundwaterSettings();

        // 4. Ограничения разработки.
        DevelopmentSettings developmentSettings =
            settingsReader.readDevelopmentSettings();

        // 5. Полный анализ.
        GeologicalAnalysisService analysisService;
        GeologicalAnalysisResult analysisResult =
            analysisService.analyze(
                deposit,
                mainIndicatorsSettings,
                groundwaterSettings,
                developmentSettings,
                groundwaterCalculationSettings);

        // 6. Данные для сводной таблицы запасов.
        // Площадь и категория определяются автоматически по данным блока
        // и координатам входящих в него скважин. Пользователь ничего не вводит.
        ReserveSummarySettings reserveSummarySettings =
            settingsReader.readReserveSummarySettings(deposit);

        // 7. Excel.
        ExcelWriter excelWriter;
        excelWriter.write(
            templatePath.string(),
            excelOutputPath.string(),
            deposit,
            analysisResult,
            reserveSummarySettings);

        // 8. Текстовая статистика по месторождению.
        DepositStatistics depositStatistics;
        depositStatistics.write(
            statisticsOutputPath,
            deposit,
            analysisResult);

        // 8. DXF-план скважин.
        DxfWriter dxfWriter;
        dxfWriter.write(
            dxfSitePlanPath.string(),
            deposit,
            analysisResult,
            reserveSummarySettings);

        // 9. DXF-геологические колонки.
        DxfColumnWriter columnWriter;
        columnWriter.write(
            dxfColumnsPath.string(),
            deposit,
            analysisResult);

        // 10. Пользователь задаёт количество разрезов и порядок скважин
        // для каждого разреза.
        SectionSettings sectionSettings =
            settingsReader.readSectionSettings(deposit);

        // 11. Геолого-литологические разрезы в DXF. Используем тот же
        // выбор скважин, который пользователь только что задал в консоли.
        const std::filesystem::path dxfSectionPath = outputDirectory / "geological_sections.dxf";
        DxfGeologicalSectionWriter dxfSectionWriter;
        dxfSectionWriter.write(
            dxfSectionPath.string(),
            deposit,
            analysisResult,
            sectionSettings);

        // 11. Третье графическое приложение — геолого-литологические
        // разрезы в DWG. Все выбранные пользователем разрезы находятся
        // в одном файле.
        DwgGeologicalSectionWriter sectionWriter;
        sectionWriter.write(
            dwgSectionPath,
            projectRoot,
            deposit,
            analysisResult,
            sectionSettings);

        // 12. Отдельное 2D-графическое приложение DWG.
        // Оно использует те же данные, что boreholes.dxf, и дополнительно
        // содержит контуры блоков. DWG создаётся через LibreDWG.
        DwgPlanWriter dwgWriter;
        dwgWriter.write(
            dwgBlockPlanPath,
            projectRoot,
            deposit,
            analysisResult,
            reserveSummarySettings);

        std::cout
            << "\n==============================================\n"
            << "Готово.\n"
            << "Excel: " << excelOutputPath << '\n'
            << "Статистика: " << statisticsOutputPath << '\n'
            << "DXF расположения скважин: " << dxfSitePlanPath << '\n'
            << "DXF геологических колонок: " << dxfColumnsPath << '\n'
            << "DXF геологических разрезов: " << dxfSectionPath << '\n'
            << "DWG геолого-литологических разрезов: " << dwgSectionPath << '\n'
            << "DWG план с контурами блоков: " << dwgBlockPlanPath << '\n'
            << "==============================================\n";

        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Ошибка: " << exception.what() << '\n';
        return 1;
    }
}
