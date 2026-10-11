#include "../../headers/geology/UnderlyingRockResolver.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr double EPS = 1e-9;
}

std::vector<UnderlyingRock> UnderlyingRockResolver::resolve(
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const UsefulMineral& countedUsefulMineral) const
{
    std::vector<UnderlyingRock> result;

    const auto& finalSections = analysis.getFinalSections().getSections();

    // 1. Обычные подстилающие породы, явно классифицированные ниже ПИ.
    for (const BoreholeSection& section : finalSections)
    {
        if (!section.isUnderlyingRock())
            continue;

        addInterval(
            result,
            section.getStartDepth(),
            section.getEndDepth(),
            findUnderlyingRockName(borehole, section.getStartDepth()));
    }

    // 2. Часть ПИ, которая не вошла в расчётную добываемую полезную толщу,
    //    тоже становится подстилающей. Это критично для:
    //      - режима DryOnly;
    //      - сухой подушки над УГВ;
    //      - ограничения добычи обводнённого ПИ оборудованием.
    for (const BoreholeSection& section : finalSections)
    {
        if (!section.isUsefulMineral())
            continue;

        const double start = section.getStartDepth();
        const double end = section.getEndDepth();

        std::vector<std::pair<double, double>> counted;

        for (const DryMineralSection& dry : countedUsefulMineral.getDrySections())
        {
            const double a = std::max(start, dry.getRoofDepth());
            const double b = std::min(end, dry.getBottomDepth());
            if (b > a + EPS)
                counted.emplace_back(a, b);
        }

        for (const WaterBearingMineralSection& water : countedUsefulMineral.getWaterBearingSections())
        {
            const double a = std::max(start, water.getRoofDepth());
            const double b = std::min(end, water.getBottomDepth());
            if (b > a + EPS)
                counted.emplace_back(a, b);
        }

        std::sort(counted.begin(), counted.end());

        double cursor = start;
        for (const auto& interval : counted)
        {
            if (interval.first > cursor + EPS)
            {
                addInterval(
                    result,
                    cursor,
                    interval.first,
                    findUnderlyingRockName(borehole, cursor));
            }

            cursor = std::max(cursor, interval.second);
        }

        if (end > cursor + EPS)
        {
            addInterval(
                result,
                cursor,
                end,
                findUnderlyingRockName(borehole, cursor));
        }
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const UnderlyingRock& left, const UnderlyingRock& right)
        {
            if (left.getRoofDepth() != right.getRoofDepth())
                return left.getRoofDepth() < right.getRoofDepth();
            return left.getBottomDepth() < right.getBottomDepth();
        });

    return result;
}

void UnderlyingRockResolver::addInterval(
    std::vector<UnderlyingRock>& result,
    double startDepth,
    double endDepth,
    const std::string& name)
{
    if (endDepth <= startDepth + EPS)
        return;

    if (!result.empty())
    {
        UnderlyingRock& previous = result.back();
        if (std::abs(previous.getBottomDepth() - startDepth) < EPS &&
            previous.getName() == name)
        {
            previous.setBottomDepth(endDepth);
            return;
        }
    }

    result.emplace_back(startDepth, endDepth, name);
}

std::string UnderlyingRockResolver::findUnderlyingRockName(
    const Borehole& borehole,
    double depth)
{
    const Sample* best = nullptr;

    for (const Sample& sample : borehole.getSamples())
    {
        if (sample.isMonolith() || sample.isAgSample())
            continue;

        if (sample.getEndDepth() <= depth + EPS)
            continue;

        if (best == nullptr || sample.getStartDepth() < best->getStartDepth())
            best = &sample;
    }

    return best == nullptr ? std::string{} : best->getSoilType();
}
