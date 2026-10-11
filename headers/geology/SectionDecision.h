#pragma once

#include "../domain/BoreholeSection.h"

#include <vector>

enum class AmbiguousSectionDecision
{
    UsefulMineral,
    InternalOverburden
};


class SectionDecision
{
public:
    SectionDecision(double startDepth, double endDepth, AmbiguousSectionDecision decision);

    double getStartDepth() const;
    double getEndDepth() const;
    double getThickness() const;
    AmbiguousSectionDecision getDecision() const;

private:
    double startDepth = 0.0;
    double endDepth = 0.0;

    AmbiguousSectionDecision decision = AmbiguousSectionDecision::InternalOverburden;
};

/*
 * Окончательная модель участков скважины.
 * Здесь уже нет Ambiguous.
 * Все участки имеют окончательный тип:
 *   ExternalOverburden
 *   UsefulMineral
 *   InternalOverburden
 *   UnderlyingRock
 */
class FinalBoreholeSections
{
public:
    const std::vector<BoreholeSection>& getSections() const;
    std::vector<BoreholeSection>& getSections();
    double getUsefulMineralThickness() const;

    void addSection(const BoreholeSection& section);

private:
    std::vector<BoreholeSection> sections;
};
