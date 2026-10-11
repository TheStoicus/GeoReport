#pragma once

#include "Borehole.h"
#include "../settings/MainIndicatorsSettings.h"

#include <string>
#include <vector>

enum class BoreholeSectionType
{
    ExternalOverburden, // внешняя вскрыша
    UsefulMineral,      // полезное ископаемое
    InternalOverburden, // внутренняя вскрыша
    UnderlyingRock,     // подстилающие породы
    UnsampledInterval,  // интервал не опробован
    Ambiguous           // неоднозначный участок
};


class BoreholeSection
{
public:
    BoreholeSection() = default;

    BoreholeSection(double startDepth, double endDepth, BoreholeSectionType type);

    double getStartDepth() const;
    double getEndDepth() const;
    double getThickness() const;

    BoreholeSectionType getType() const;

    void setStartDepth(double value);
    void setEndDepth(double value);
    void setType(BoreholeSectionType type);

    bool isExternalOverburden() const;
    bool isUsefulMineral() const;
    bool isInternalOverburden() const;
    bool isUnderlyingRock() const;
    bool isUnsampledInterval() const;
    bool isAmbiguous() const;

    static std::string typeToString(BoreholeSectionType type);

private:
    double startDepth = 0.0;
    double endDepth = 0.0;

    BoreholeSectionType type = BoreholeSectionType::UnderlyingRock;
};


/*
 * Результат первичной классификации скважины.
 * sections: участки, классифицированные однозначно.
 * ambiguousSections: участки, для которых необходимо дополнительное решение перед построением окончательной модели.
 */
class SectionClassificationResult
{
public:
    const std::vector<BoreholeSection>& getSections() const;
    std::vector<BoreholeSection>& getSections();
    const std::vector<BoreholeSection>& getAmbiguousSections() const;
    std::vector<BoreholeSection>& getAmbiguousSections();

    bool hasAmbiguousSections() const;

private:
    std::vector<BoreholeSection> sections;
    std::vector<BoreholeSection> ambiguousSections;
};
