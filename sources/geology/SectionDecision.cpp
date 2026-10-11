#include "../../headers/geology/SectionDecision.h"

#include <algorithm>


SectionDecision::SectionDecision(
    double startDepth,
    double endDepth,
    AmbiguousSectionDecision decision)
    : startDepth(std::max(0.0, startDepth)),
      endDepth(std::max(0.0, endDepth)),
      decision(decision)
{
    if (endDepth < startDepth)
    {
        this->endDepth = this->startDepth;
    }
}


double SectionDecision::getStartDepth() const
{
    return startDepth;
}


double SectionDecision::getEndDepth() const
{
    return endDepth;
}


double SectionDecision::getThickness() const
{
    return std::max(
        0.0,
        endDepth - startDepth);
}


AmbiguousSectionDecision
SectionDecision::getDecision() const
{
    return decision;
}


const std::vector<BoreholeSection>&
FinalBoreholeSections::getSections() const
{
    return sections;
}


std::vector<BoreholeSection>&
FinalBoreholeSections::getSections()
{
    return sections;
}


void FinalBoreholeSections::addSection(
    const BoreholeSection& section)
{
    if (section.getThickness() <= 0.0)
    {
        return;
    }

    /*
     * Если новый участок непосредственно продолжает
     * предыдущий и имеет тот же тип, объединяем их.
     */
    if (!sections.empty())
    {
        BoreholeSection& previous = sections.back();

        if (previous.getType() == section.getType() &&
            previous.getEndDepth() ==
                section.getStartDepth())
        {
            previous.setEndDepth(
                section.getEndDepth());

            return;
        }
    }

    sections.push_back(section);
}


double FinalBoreholeSections::getUsefulMineralThickness() const
{
    double thickness = 0.0;

    for (const BoreholeSection& section : sections)
    {
        if (section.isUsefulMineral())
        {
            thickness += section.getThickness();
        }
    }

    return thickness;
}