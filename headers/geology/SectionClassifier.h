#pragma once

#include "../domain/Borehole.h"
#include "../domain/BoreholeSection.h"
#include "../settings/MainIndicatorsSettings.h"

#include <vector>


class SectionClassifier
{
public:

    /*
     * Основная классификация скважины.
     */
    SectionClassificationResult classify(
        const Borehole& borehole,
        const MainIndicatorsSettings& settings) const;


private:

    /*
     * Классификация по образцам.
     *
     * Используется общая геометрия интервалов проб.
     */
    SectionClassificationResult classifyBySamples(
        const Borehole& borehole,
        const MainIndicatorsSettings& settings) const;


    /*
     * Отдельная точка расширения для режима
     * определения вскрыши по типам пород.
     */
    void classifyByRockTypes(
        const Borehole& borehole,
        const MainIndicatorsSettings& settings,
        SectionClassificationResult& result) const;


    /*
     * Добавляет обычный участок.
     *
     * Соседние участки одного типа объединяются.
     */
    void addSection(
        std::vector<BoreholeSection>& sections,
        double startDepth,
        double endDepth,
        BoreholeSectionType type) const;


    /*
     * Добавляет неоднозначный участок.
     */
    void addAmbiguousSection(
        std::vector<BoreholeSection>& sections,
        double startDepth,
        double endDepth) const;


    /*
     * Добавляет неопробованные интервалы.
     */
    void addUnsampledIntervals(
        const Borehole& borehole,
        std::vector<BoreholeSection>& sections) const;
};