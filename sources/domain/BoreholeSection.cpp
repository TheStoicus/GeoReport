#include "../../headers/domain/BoreholeSection.h"

#include <algorithm>

BoreholeSection::BoreholeSection(double startDepth, double endDepth, BoreholeSectionType type)
    : startDepth(startDepth), endDepth(endDepth), type(type)
{
    if (this->startDepth < 0.0)
        this->startDepth = 0.0;

    if (this->endDepth < this->startDepth)
        this->endDepth = this->startDepth;
}

double BoreholeSection::getStartDepth() const
{
    return startDepth;
}

double BoreholeSection::getEndDepth() const
{
    return endDepth;
}

double BoreholeSection::getThickness() const
{
    return std::max(0.0, endDepth - startDepth);
}


BoreholeSectionType BoreholeSection::getType() const
{
    return type;
}

void BoreholeSection::setStartDepth(double value)
{
    startDepth = std::max(0.0, value);

    if (endDepth < startDepth)
        endDepth = startDepth;
}

void BoreholeSection::setEndDepth(double value)
{
    endDepth = std::max(startDepth, value);
}

void BoreholeSection::setType(BoreholeSectionType value)
{
    type = value;
}

bool BoreholeSection::isExternalOverburden() const
{
    return type == BoreholeSectionType::ExternalOverburden;
}

bool BoreholeSection::isUsefulMineral() const
{
    return type == BoreholeSectionType::UsefulMineral;
}

bool BoreholeSection::isInternalOverburden() const
{
    return type == BoreholeSectionType::InternalOverburden;
}

bool BoreholeSection::isUnderlyingRock() const
{
    return type == BoreholeSectionType::UnderlyingRock;
}

bool BoreholeSection::isUnsampledInterval() const
{
    return type == BoreholeSectionType::UnsampledInterval;
}

bool BoreholeSection::isAmbiguous() const
{
    return type == BoreholeSectionType::Ambiguous;
}

std::string BoreholeSection::typeToString(
    BoreholeSectionType type)
{
    switch (type)
    {
        case BoreholeSectionType::ExternalOverburden:
            return "Внешняя вскрыша";

        case BoreholeSectionType::UsefulMineral:
            return "Полезное ископаемое";

        case BoreholeSectionType::InternalOverburden:
            return "Внутренняя вскрыша";

        case BoreholeSectionType::UnderlyingRock:
            return "Подстилающие породы";

        case BoreholeSectionType::UnsampledInterval:
            return "ИНТЕРВАЛ НЕ ОПРОБОВАН";

        case BoreholeSectionType::Ambiguous:
            return "Неоднозначный участок";
    }

    return "Неизвестно";
}

const std::vector<BoreholeSection>& SectionClassificationResult::getSections() const
{
    return sections;
}

std::vector<BoreholeSection>& SectionClassificationResult::getSections()
{
    return sections;
}

const std::vector<BoreholeSection>& SectionClassificationResult::getAmbiguousSections() const
{
    return ambiguousSections;
}

std::vector<BoreholeSection>& SectionClassificationResult::getAmbiguousSections()
{
    return ambiguousSections;
}

bool SectionClassificationResult::hasAmbiguousSections() const
{
    return !ambiguousSections.empty();
}