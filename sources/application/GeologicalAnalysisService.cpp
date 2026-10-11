#include "../../headers/application/GeologicalAnalysisService.h"

#include "../../headers/calculations/BoreholeSectionAnalyzer.h"
#include "../../headers/calculations/GroundwaterAnalysis.h"
#include "../../headers/calculations/MainIndicatorsCalculator.h"
#include "../../headers/calculations/SampleSplitter.h"
#include "../../headers/geology/BoreholeGeologyResolver.h"

#include <iostream>
#include <string>

GeologicalAnalysisResult GeologicalAnalysisService::analyze(
    Deposit& deposit,
    const MainIndicatorsSettings& mainIndicatorsSettings,
    const GroundwaterSettings& groundwaterSettings,
    const DevelopmentSettings& developmentSettings,
    const GroundwaterCalculationSettings& groundwaterCalculationSettings) const
{
    GeologicalAnalysisResult result;

    BoreholeSectionAnalyzer sectionAnalyzer;
    std::vector<BoreholeSectionAnalysis> analyses =
        sectionAnalyzer.analyze(deposit, mainIndicatorsSettings);

    for (const BoreholeSectionAnalysis& analysis : analyses)
    {
        std::cout << "\nСкважина " << analysis.getBoreholeNumber()
                  << " — окончательные участки:\n";

        for (const BoreholeSection& section :
             analysis.getFinalSections().getSections())
        {
            std::cout << "  "
                      << section.getStartDepth() << " - "
                      << section.getEndDepth() << " м — "
                      << BoreholeSection::typeToString(section.getType())
                      << '\n';
        }
    }

    // Настройка режима УГВ применяется здесь, до расчёта ПИ.
    GroundwaterAnalysis groundwaterAnalysis;
    groundwaterAnalysis.analyze(
        deposit.getBoreholes(),
        groundwaterCalculationSettings);

    BoreholeGeologyResolver geologyResolver;
    MainIndicatorsCalculator indicatorsCalculator;

    int nextSampleNumber = 1;

    for (const BoreholeSectionAnalysis& analysis : analyses)
    {
        if (!analysis.isResolved())
            continue;

        Borehole* borehole = deposit.findBorehole(analysis.getBoreholeNumber());
        if (borehole == nullptr)
            continue;

        const BoreholeGeology geology =
            geologyResolver.resolve(
                *borehole,
                analysis,
                groundwaterAnalysis,
                groundwaterSettings,
                developmentSettings);

        /*
         * Только после формирования полной геологической модели
         * мы знаем все границы, по которым должен быть разделён
         * лабораторный образец:
         *
         *   вскрыша / ПИ
         *   сухое ПИ / обводнённое ПИ
         *   ПИ / подстилающие породы
         *
         * Поэтому SampleSplitter вызывается здесь, а не раньше.
         */
        SampleSplitter sampleSplitter;
        sampleSplitter.split(*borehole, geology);

        // После геологического разделения всем образцам присваиваются
        // новые сквозные номера: 1, 2, 3 ... по порядку скважин.
        // Признаки монолита и АГ сохранены внутри Sample и не зависят
        // от нового номера.
        for (Sample& sample : borehole->getSamplesModifiable())
            sample.setSampleNumber(std::to_string(nextSampleNumber++));

        const MainIndicators indicators =
            indicatorsCalculator.calculate(
                *borehole,
                geology,
                groundwaterAnalysis);

        BoreholeAnalysisResult boreholeResult(
            analysis.getBoreholeNumber());
        boreholeResult.setSectionAnalysis(analysis);
        boreholeResult.setGeology(geology);
        boreholeResult.setIndicators(indicators);

        result.addBorehole(boreholeResult);
    }

    return result;
}
