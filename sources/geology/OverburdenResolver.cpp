#include "../../headers/geology/OverburdenResolver.h"

#include <cmath>

std::vector<Overburden> OverburdenResolver::resolve(
    const BoreholeSectionAnalysis& analysis) const
{
    constexpr double EPS = 1e-9;

    std::vector<Overburden> result;

    for (const BoreholeSection& section :
         analysis.getFinalSections().getSections())
    {
        /*
         * Неопробованный верхний интервал является ПРС и хранится
         * отдельным объектом SurfaceSoil. В обычную вскрышу его
         * повторно включать нельзя.
         */
        if (section.isUnsampledInterval())
            continue;

        if (!section.isExternalOverburden() &&
            !section.isInternalOverburden())
            continue;

        const double start = section.getStartDepth();
        const double end = section.getEndDepth();

        if (end <= start + EPS)
            continue;

        result.push_back(
            createOverburden(section));
    }

    return result;
}

Overburden OverburdenResolver::createOverburden(
    const BoreholeSection& section) const
{
    return Overburden(
        section.getStartDepth(),
        section.getEndDepth(),
        getOverburdenType(section));
}

OverburdenType OverburdenResolver::getOverburdenType(
    const BoreholeSection& section) const
{
    if (section.isInternalOverburden())
        return OverburdenType::Internal;

    return OverburdenType::Surface;
}
