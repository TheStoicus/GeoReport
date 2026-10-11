#include "../../headers/calculations/SampleSplitter.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <utility>

namespace
{
    const std::vector<std::string> FRAGMENT_SUFFIXES =
    {
        "а", "б", "в", "г", "д", "е", "ё", "ж", "з", "и", "й", "к"
    };

    void addBoundary(
        std::vector<double>& boundaries,
        double value)
    {
        if (!std::isfinite(value))
            return;

        boundaries.push_back(value);
    }
}

void SampleSplitter::split(
    Borehole& borehole,
    const BoreholeGeology& geology) const
{
    std::vector<Sample>& samples =
        borehole.getSamplesModifiable();

    if (samples.empty())
        return;

    std::vector<double> boundaries;

    // ------------------------------------------------------------
    // Все реальные границы геологической модели.
    // ------------------------------------------------------------

    for (const SurfaceSoil& surfaceSoil :
         geology.hasSurfaceSoil()
             ? std::vector<SurfaceSoil>{*geology.getSurfaceSoil()}
             : std::vector<SurfaceSoil>{})
    {
        addBoundary(boundaries, surfaceSoil.getRoofDepth());
        addBoundary(boundaries, surfaceSoil.getBottomDepth());
    }

    for (const Overburden& section : geology.getOverburden())
    {
        addBoundary(boundaries, section.getRoofDepth());
        addBoundary(boundaries, section.getBottomDepth());
    }

    for (const DryMineralSection& section :
         geology.getUsefulMineral().getDrySections())
    {
        addBoundary(boundaries, section.getRoofDepth());
        addBoundary(boundaries, section.getBottomDepth());
    }

    for (const WaterBearingMineralSection& section :
         geology.getUsefulMineral().getWaterBearingSections())
    {
        addBoundary(boundaries, section.getRoofDepth());
        addBoundary(boundaries, section.getBottomDepth());
    }

    for (const UnderlyingRock& section : geology.getUnderlyingRock())
    {
        addBoundary(boundaries, section.getRoofDepth());
        addBoundary(boundaries, section.getBottomDepth());
    }

    if (boundaries.empty())
        return;

    std::sort(boundaries.begin(), boundaries.end());

    boundaries.erase(
        std::unique(
            boundaries.begin(),
            boundaries.end(),
            [](double first, double second)
            {
                return std::abs(first - second) <
                       SampleSplitter::DEPTH_EPSILON;
            }),
        boundaries.end());

    std::vector<Sample> result;
    result.reserve(samples.size());

    for (const Sample& sample : samples)
    {
        const double sampleStart = sample.getStartDepth();
        const double sampleEnd = sample.getEndDepth();

        if (sampleEnd <= sampleStart + DEPTH_EPSILON)
        {
            result.push_back(sample);
            continue;
        }

        /*
         * АГ-пробы не разделяем.
         * Они полностью исключены из геологического расчёта.
         */
        if (sample.isAgSample())
        {
            result.push_back(sample);
            continue;
        }

        std::vector<double> sampleBoundaries;

        for (double boundary : boundaries)
        {
            if (isInside(boundary, sampleStart, sampleEnd))
                sampleBoundaries.push_back(boundary);
        }

        if (sampleBoundaries.empty())
        {
            result.push_back(sample);
            continue;
        }

        double currentStart = sampleStart;

        for (std::size_t i = 0;
             i <= sampleBoundaries.size();
             ++i)
        {
            const double currentEnd =
                (i < sampleBoundaries.size())
                    ? sampleBoundaries[i]
                    : sampleEnd;

            if (currentEnd <= currentStart + DEPTH_EPSILON)
            {
                currentStart = currentEnd;
                continue;
            }

            result.push_back(
                createFragment(
                    sample,
                    currentStart,
                    currentEnd,
                    makeFragmentNumber(
                        sample.getSampleNumber(),
                        i)));

            currentStart = currentEnd;
        }
    }

    samples = std::move(result);
}

Sample SampleSplitter::createFragment(
    const Sample& source,
    double startDepth,
    double endDepth,
    const std::string& sampleNumber)
{
    Sample fragment(
        sampleNumber,
        startDepth,
        endDepth);

    fragment.setGostSoilType(source.getGostSoilType());
    fragment.setTkpSoilType(source.getTkpSoilType());

    for (const auto& [name, value] : source.getGOSTParameters())
    {
        fragment.setGOSTParameter(name, value);
    }

    for (const auto& [name, value] : source.getTKPParameters())
    {
        fragment.setTKPParameter(name, value);
    }

    return fragment;
}

std::string SampleSplitter::makeFragmentNumber(
    const std::string& originalNumber,
    std::size_t fragmentIndex)
{
    if (fragmentIndex == 0)
        return originalNumber;

    const std::size_t suffixIndex = fragmentIndex - 1;

    if (suffixIndex < FRAGMENT_SUFFIXES.size())
        return originalNumber + FRAGMENT_SUFFIXES[suffixIndex];

    return originalNumber + "_" + std::to_string(fragmentIndex);
}

bool SampleSplitter::isInside(
    double value,
    double startDepth,
    double endDepth)
{
    return value > startDepth + DEPTH_EPSILON &&
           value < endDepth - DEPTH_EPSILON;
}
