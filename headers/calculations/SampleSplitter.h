#pragma once

#include "../domain/Borehole.h"
#include "../geology/BoreholeGeology.h"

#include <string>
#include <cstddef>

class SampleSplitter
{
public:
    /*
     * Разделяет лабораторные образцы по границам окончательной
     * расчётной геологической модели.
     *
     * Разделение выполняется по границам:
     *   - ПРС / геологического разреза;
     *   - вскрыши / ПИ;
     *   - сухого / обводнённого ПИ;
     *   - ПИ / подстилающих пород;
     *   - других интервалов, реально присутствующих в BoreholeGeology.
     *
     * В результате один исходный образец может быть разделён,
     * например, на:
     *
     *   4.0–5.0  → сухое ПИ
     *   5.0–6.0  → обводнённое ПИ
     *   6.0–7.0  → подстилающая порода
     *
     * Первая часть сохраняет исходный номер образца,
     * последующие получают суффиксы а, б, в, ...
     */
    void split(
        Borehole& borehole,
        const BoreholeGeology& geology) const;

private:
    static constexpr double DEPTH_EPSILON = 1e-9;

    static Sample createFragment(
        const Sample& source,
        double startDepth,
        double endDepth,
        const std::string& sampleNumber);

    static std::string makeFragmentNumber(
        const std::string& originalNumber,
        std::size_t fragmentIndex);

    static bool isInside(
        double value,
        double startDepth,
        double endDepth);
};
