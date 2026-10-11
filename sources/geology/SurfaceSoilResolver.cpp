#include "../../headers/geology/SurfaceSoilResolver.h"

#include <cmath>

std::optional<SurfaceSoil>
SurfaceSoilResolver::resolve(
    const BoreholeSectionAnalysis& analysis) const
{
    constexpr double EPS = 1e-9;

    /*
     * ПРС — это НЕ фиксированные 0.50 м от поверхности.
     *
     * ПРС определяется исключительно по фактическому верхнему
     * неопробованному интервалу скважины.
     *
     * Поэтому возможны, например:
     *
     *     0.0–0.2 м -> ПРС
     *     0.0–0.3 м -> ПРС
     *     0.0–0.5 м -> ПРС
     *
     * Если проба начинается непосредственно с 0.0 м, ПРС отсутствует.
     *
     * Очень важно отличать UnsampledInterval от ExternalOverburden:
     * фиксированная вскрыша 0.0–0.8 м не должна автоматически превращаться
     * в ПРС 0.0–0.5 м.
     */
    for (const BoreholeSection& section :
         analysis.getFinalSections().getSections())
    {
        if (!section.isUnsampledInterval())
            continue;

        if (std::abs(section.getStartDepth()) > EPS)
            continue;

        if (section.getEndDepth() <= EPS)
            continue;

        return SurfaceSoil(
            0.0,
            section.getEndDepth());
    }

    return std::nullopt;
}
