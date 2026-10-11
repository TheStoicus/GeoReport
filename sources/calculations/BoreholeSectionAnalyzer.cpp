#include "../../headers/calculations/BoreholeSectionAnalyzer.h"

#include "../../headers/geology/SectionClassifier.h"
#include "../../headers/geology/SectionDecisionProvider.h"
#include "../../headers/geology/BoreholeSectionResolver.h"


// ============================================================================
// Анализ всех скважин
// ============================================================================

std::vector<BoreholeSectionAnalysis>
BoreholeSectionAnalyzer::analyze(
    const Deposit& deposit,
    const MainIndicatorsSettings& settings) const
{
    std::vector<BoreholeSectionAnalysis> results;


    const auto& boreholes =
        deposit.getBoreholes();


    results.reserve(
        boreholes.size());


    for (const Borehole& borehole :
         boreholes)
    {
        results.push_back(
            analyzeBorehole(
                borehole,
                settings));
    }


    return results;
}


// ============================================================================
// Анализ одной скважины
// ============================================================================

BoreholeSectionAnalysis
BoreholeSectionAnalyzer::analyzeBorehole(
    const Borehole& borehole,
    const MainIndicatorsSettings& settings) const
{
    /*
     * Создаём результат анализа конкретной скважины.
     */
    BoreholeSectionAnalysis result(
        borehole.getBoreholeNumber());


    // ========================================================================
    // 1. Первичная классификация
    // ========================================================================

    SectionClassifier classifier;


    SectionClassificationResult classification =
        classifier.classify(
            borehole,
            settings);


    result.setClassification(
        classification);


    // ========================================================================
    // 2. Получение решений для неоднозначных участков
    // ========================================================================

    /*
     * SectionDecisionProvider отвечает только за получение
     * решений по неоднозначным участкам.
     *
     * На текущем этапе внутри него находится тестовое правило:
     *
     *     скважина 26
     *     интервал 4.9–6.3 м
     *     → InternalOverburden
     *
     * В дальнейшем этот источник будет заменён
     * полноценным механизмом пользовательского выбора.
     */
    if (classification.hasAmbiguousSections())
    {
        SectionDecisionProvider decisionProvider;


        const std::vector<SectionDecision> decisions =
            decisionProvider.getDecisions(
                result);


        /*
         * Сохраняем полученные решения
         * внутри результата анализа.
         */
        for (const SectionDecision& decision :
             decisions)
        {
            result.addDecision(
                decision);
        }
    }


    // ========================================================================
    // 3. Построение окончательной модели
    // ========================================================================

    /*
     * Даже если неоднозначных участков нет,
     * вызываем resolver.
     *
     * Это позволяет иметь единый жизненный цикл:
     *
     *     Classification
     *           ↓
     *     Decisions (если нужны)
     *           ↓
     *     Final Sections
     *
     * Если неоднозначный участок не получил решения,
     * resolver просто не включит его в окончательную модель.
     */
    BoreholeSectionResolver resolver;


    result.resolve(
        resolver);


    return result;
}