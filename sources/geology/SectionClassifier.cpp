#include "../../headers/geology/SectionClassifier.h"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace
{
    constexpr double EPS = 1e-9;

    bool valid(double start, double end)
    {
        return end > start + EPS;
    }

    bool same(double a, double b)
    {
        return std::abs(a - b) < EPS;
    }

    bool covers(const Sample& sample, double start, double end)
    {
        return sample.getStartDepth() <= start + EPS &&
               sample.getEndDepth() >= end - EPS;
    }

    bool ignored(const Sample& sample, const MainIndicatorsSettings& settings)
    {
        if (sample.isMonolith() || sample.isAgSample())
            return true;

        return sample.isSurfaceSample() &&
               settings.getSurfaceSampleMode() == SurfaceSampleMode::Exclude;
    }

    enum class Participation
    {
        Useful,
        Overburden,
        Underlying,
        Other
    };

    Participation participation(
        const Sample& sample,
        const MainIndicatorsSettings& settings)
    {
        if (settings.isUnderlyingSample(sample))
            return Participation::Underlying;

        if (settings.getOverburdenMode() == OverburdenMode::ByRockType &&
            settings.isOverburdenSample(sample))
            return Participation::Overburden;

        if (settings.isUseful(sample))
            return Participation::Useful;

        return Participation::Other;
    }
}

SectionClassificationResult SectionClassifier::classify(
    const Borehole& borehole,
    const MainIndicatorsSettings& settings) const
{
    return classifyBySamples(borehole, settings);
}

SectionClassificationResult SectionClassifier::classifyBySamples(
    const Borehole& borehole,
    const MainIndicatorsSettings& settings) const
{
    SectionClassificationResult result;
    const auto& samples = borehole.getSamples();

    std::vector<double> boundaries;
    for (const Sample& sample : samples)
    {
        if (ignored(sample, settings))
            continue;

        if (!valid(sample.getStartDepth(), sample.getEndDepth()))
            continue;

        boundaries.push_back(sample.getStartDepth());
        boundaries.push_back(sample.getEndDepth());
    }

    if (boundaries.size() < 2)
        return result;

    std::sort(boundaries.begin(), boundaries.end());

    std::vector<double> uniqueBoundaries;
    for (double depth : boundaries)
    {
        if (uniqueBoundaries.empty() || !same(uniqueBoundaries.back(), depth))
            uniqueBoundaries.push_back(depth);
    }

    double firstUseful = -1.0;
    double lastUseful = -1.0;

    for (const Sample& sample : samples)
    {
        if (ignored(sample, settings) || !settings.isUseful(sample))
            continue;

        if (!valid(sample.getStartDepth(), sample.getEndDepth()))
            continue;

        if (firstUseful < 0.0 || sample.getStartDepth() < firstUseful)
            firstUseful = sample.getStartDepth();

        if (lastUseful < 0.0 || sample.getEndDepth() > lastUseful)
            lastUseful = sample.getEndDepth();
    }

    for (std::size_t i = 0; i + 1 < uniqueBoundaries.size(); ++i)
    {
        const double start = uniqueBoundaries[i];
        const double end = uniqueBoundaries[i + 1];
        if (!valid(start, end))
            continue;

        bool useful = false;
        bool overburden = false;
        bool underlying = false;
        bool other = false;

        for (const Sample& sample : samples)
        {
            if (ignored(sample, settings) || !covers(sample, start, end))
                continue;

            switch (participation(sample, settings))
            {
                case Participation::Useful: useful = true; break;
                case Participation::Overburden: overburden = true; break;
                case Participation::Underlying: underlying = true; break;
                case Participation::Other: other = true; break;
            }
        }

        if (!useful && !overburden && !underlying && !other)
            continue;

        // Полезное имеет приоритет над обычной породой.
        if (useful)
        {
            addSection(result.getSections(), start, end,
                       BoreholeSectionType::UsefulMineral);
            continue;
        }

        // Явно выбранная подстилающая порода действительно является
        // подстилающей только ниже полезного ископаемого.
        if (underlying && lastUseful >= 0.0 && start >= lastUseful - EPS)
        {
            addSection(result.getSections(), start, end,
                       BoreholeSectionType::UnderlyingRock);
            continue;
        }

        // Если ПИ нет вообще, все обычные интервалы надрабатываемой
        // толщи относятся к внешней вскрыше; это не создаёт ложной
        // подстилающей породы без установленной подошвы ПИ.
        if (firstUseful < 0.0)
        {
            addSection(result.getSections(), start, end,
                       BoreholeSectionType::ExternalOverburden);
            continue;
        }

        // Выбранные вскрышные и прочие неполезные породы выше ПИ — внешняя вскрыша.
        if (end <= firstUseful + EPS)
        {
            addSection(result.getSections(), start, end,
                       BoreholeSectionType::ExternalOverburden);
            continue;
        }

        // Всё, что находится ниже последнего полезного интервала,
        // является подстилающей породой.
        if (start >= lastUseful - EPS)
        {
            addSection(result.getSections(), start, end,
                       BoreholeSectionType::UnderlyingRock);
            continue;
        }

        // Неполезная порода внутри контура ПИ — неоднозначна.
        addAmbiguousSection(result.getAmbiguousSections(), start, end);
    }

    // Фиксированная внешняя вскрыша должна работать даже тогда,
    // когда исходная проба пересекает заданную границу.
    if (settings.isFixedOverburdenEnabled())
    {
        const double limit = settings.getFixedOverburdenThickness();
        std::vector<BoreholeSection> corrected;
        std::vector<BoreholeSection> correctedAmbiguous;

        for (const BoreholeSection& section : result.getSections())
        {
            const double start = section.getStartDepth();
            const double end = section.getEndDepth();

            if (end <= limit + EPS)
            {
                addSection(corrected, start, end,
                           BoreholeSectionType::ExternalOverburden);
            }
            else if (start < limit - EPS)
            {
                addSection(corrected, start, limit,
                           BoreholeSectionType::ExternalOverburden);
                addSection(corrected, limit, end, section.getType());
            }
            else
            {
                addSection(corrected, start, end, section.getType());
            }
        }

        for (const BoreholeSection& section : result.getAmbiguousSections())
        {
            const double start = section.getStartDepth();
            const double end = section.getEndDepth();

            if (end <= limit + EPS)
            {
                addSection(corrected, start, end,
                           BoreholeSectionType::ExternalOverburden);
            }
            else if (start < limit - EPS)
            {
                addSection(corrected, start, limit,
                           BoreholeSectionType::ExternalOverburden);
                addAmbiguousSection(correctedAmbiguous, limit, end);
            }
            else
            {
                addAmbiguousSection(correctedAmbiguous, start, end);
            }
        }

        result.getSections() = std::move(corrected);
        result.getAmbiguousSections() = std::move(correctedAmbiguous);
    }

    // Если полезное ископаемое начинается непосредственно под ПРС,
    // допускается задать минимальную расчётную мощность внешней вскрыши.
    // Это реализует правило вида: ПРС 0.5 м + минимальная вскрыша 0.8 м
    // перед началом учитываемого ПИ.
    if (settings.getOverburdenMode() == OverburdenMode::ByRockType &&
        settings.getMinimumOverburdenThickness() > 0.0 &&
        firstUseful >= 0.0 && firstUseful <= 0.5 + EPS)
    {
        const double limit = settings.getMinimumOverburdenThickness();
        std::vector<BoreholeSection> corrected;
        std::vector<BoreholeSection> correctedAmbiguous;

        for (const BoreholeSection& section : result.getSections())
        {
            const double start = section.getStartDepth();
            const double end = section.getEndDepth();

            if (end <= limit + EPS)
            {
                addSection(corrected, start, end, BoreholeSectionType::ExternalOverburden);
            }
            else if (start < limit - EPS)
            {
                addSection(corrected, start, limit, BoreholeSectionType::ExternalOverburden);
                addSection(corrected, limit, end, section.getType());
            }
            else
            {
                addSection(corrected, start, end, section.getType());
            }
        }

        for (const BoreholeSection& section : result.getAmbiguousSections())
        {
            const double start = section.getStartDepth();
            const double end = section.getEndDepth();

            if (end <= limit + EPS)
            {
                addSection(corrected, start, end, BoreholeSectionType::ExternalOverburden);
            }
            else if (start < limit - EPS)
            {
                addSection(corrected, start, limit, BoreholeSectionType::ExternalOverburden);
                addAmbiguousSection(correctedAmbiguous, limit, end);
            }
            else
            {
                addAmbiguousSection(correctedAmbiguous, start, end);
            }
        }

        result.getSections() = std::move(corrected);
        result.getAmbiguousSections() = std::move(correctedAmbiguous);
    }

    addUnsampledIntervals(borehole, result.getSections());

    std::sort(result.getSections().begin(), result.getSections().end(),
              [](const BoreholeSection& a, const BoreholeSection& b)
              {
                  if (!same(a.getStartDepth(), b.getStartDepth()))
                      return a.getStartDepth() < b.getStartDepth();
                  return a.getEndDepth() < b.getEndDepth();
              });

    return result;
}

void SectionClassifier::classifyByRockTypes(
    const Borehole& borehole,
    const MainIndicatorsSettings& settings,
    SectionClassificationResult& result) const
{
    // В текущей архитектуре тип породы является частью общей
    // классификации по пробам, поэтому отдельного второго прохода нет.
    (void)borehole;
    (void)settings;
    (void)result;
}

void SectionClassifier::addSection(
    std::vector<BoreholeSection>& sections,
    double startDepth,
    double endDepth,
    BoreholeSectionType type) const
{
    if (!valid(startDepth, endDepth))
        return;

    if (!sections.empty())
    {
        BoreholeSection& previous = sections.back();
        if (previous.getType() == type && same(previous.getEndDepth(), startDepth))
        {
            previous.setEndDepth(endDepth);
            return;
        }
    }

    sections.emplace_back(startDepth, endDepth, type);
}

void SectionClassifier::addAmbiguousSection(
    std::vector<BoreholeSection>& sections,
    double startDepth,
    double endDepth) const
{
    if (!valid(startDepth, endDepth))
        return;

    if (!sections.empty())
    {
        BoreholeSection& previous = sections.back();
        if (previous.isAmbiguous() && same(previous.getEndDepth(), startDepth))
        {
            previous.setEndDepth(endDepth);
            return;
        }
    }

    sections.emplace_back(startDepth, endDepth, BoreholeSectionType::Ambiguous);
}

void SectionClassifier::addUnsampledIntervals(
    const Borehole& borehole,
    std::vector<BoreholeSection>& sections) const
{
    std::vector<std::pair<double, double>> intervals;

    for (const Sample& sample : borehole.getSamples())
    {
        if (sample.isMonolith() || sample.isAgSample() ||
            !valid(sample.getStartDepth(), sample.getEndDepth()))
            continue;

        intervals.emplace_back(sample.getStartDepth(), sample.getEndDepth());
    }

    if (intervals.empty())
        return;

    std::sort(intervals.begin(), intervals.end(),
              [](const auto& a, const auto& b)
              {
                  if (!same(a.first, b.first))
                      return a.first < b.first;
                  return a.second < b.second;
              });

    // Верхний интервал без обычной пробы — ПРС/внешняя вскрыша.
    if (intervals.front().first > EPS)
        addSection(sections, 0.0, intervals.front().first,
                   BoreholeSectionType::ExternalOverburden);

    // Внутренние разрывы опробования сохраняем явно.
    double coveredEnd = intervals.front().second;
    for (std::size_t i = 1; i < intervals.size(); ++i)
    {
        if (intervals[i].first > coveredEnd + EPS)
        {
            addSection(sections, coveredEnd, intervals[i].first,
                       BoreholeSectionType::UnsampledInterval);
        }

        coveredEnd = std::max(coveredEnd, intervals[i].second);
    }

    std::sort(sections.begin(), sections.end(),
              [](const BoreholeSection& a, const BoreholeSection& b)
              {
                  if (!same(a.getStartDepth(), b.getStartDepth()))
                      return a.getStartDepth() < b.getStartDepth();
                  return a.getEndDepth() < b.getEndDepth();
              });
}
