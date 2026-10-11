#include "graphics/dxf/DxfColumnWriter.h"

#include "domain/Borehole.h"
#include "domain/Sample.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    std::string formatNumber(double value)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << value;
        return stream.str();
    }

    std::string formatScaleElevation(double value)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(0) << value;
        return stream.str();
    }

    const BoreholeAnalysisResult* findBoreholeResult(
        const GeologicalAnalysisResult& analysisResult,
        const std::string& boreholeNumber)
    {
        for (const auto& result : analysisResult.getBoreholes())
        {
            if (result.getBoreholeNumber() == boreholeNumber)
            {
                return &result;
            }
        }

        return nullptr;
    }

    void addDepthBoundary(
        DxfDocument& document,
        double xLeft,
        double xRight,
        double depth,
        double collarElevation,
        double depthScale,
        double textHeight,
        double textOffset,
        double depthMarkLength)
    {
        const double y = -depth * depthScale;

        document.addLine(
            "COLUMN_DEPTH",
            xLeft,
            y,
            xLeft + depthMarkLength,
            y);

        document.addText(
            "COLUMN_DEPTH",
            xLeft - textOffset,
            y - textHeight / 2.0,
            textHeight,
            formatNumber(collarElevation - depth));

        document.addText(
            "COLUMN_DEPTH",
            xRight + textOffset,
            y - textHeight / 2.0,
            textHeight,
            formatNumber(depth));
    }

    void addSampleInterval(
        DxfDocument& document,
        const Sample& sample,
        double xLeft,
        double xRight,
        double depthScale)
    {
        double startDepth = sample.getStartDepth();
        double endDepth = sample.getEndDepth();

        if (endDepth < startDepth)
        {
            std::swap(startDepth, endDepth);
        }

        if (endDepth <= startDepth)
        {
            return;
        }

        const double yTop =
            -startDepth * depthScale;

        const double yBottom =
            -endDepth * depthScale;

        /*
         * Интервал образца теперь показывается непосредственно
         * на стволе скважины.
         *
         * Отдельные подписи:
         *   - номер образца;
         *   - интервал;
         *   - тип породы
         *
         * больше не выводятся справа от колонки.
         * Кровля и подошва интервала уже подписываются
         * общей системой отметок/границ колонки.
         */

        document.addLine(
            "COLUMN_SAMPLES",
            xLeft,
            yTop,
            xRight,
            yTop);

        document.addLine(
            "COLUMN_SAMPLES",
            xLeft,
            yBottom,
            xRight,
            yBottom);

        constexpr double SAMPLE_MARK_LENGTH = 1.5;

        document.addLine(
            "COLUMN_SAMPLES",
            xLeft - SAMPLE_MARK_LENGTH,
            yTop,
            xLeft,
            yTop);

        document.addLine(
            "COLUMN_SAMPLES",
            xRight,
            yTop,
            xRight + SAMPLE_MARK_LENGTH,
            yTop);

        document.addLine(
            "COLUMN_SAMPLES",
            xLeft - SAMPLE_MARK_LENGTH,
            yBottom,
            xLeft,
            yBottom);

        document.addLine(
            "COLUMN_SAMPLES",
            xRight,
            yBottom,
            xRight + SAMPLE_MARK_LENGTH,
            yBottom);
    }
}

void DxfColumnWriter::write(
    const std::string& filePath,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult) const
{
    DxfDocument document;

    /*
     * ------------------------------------------------------------
     * Слои
     * ------------------------------------------------------------
     */

    document.addLayer(
        DxfLayer("COLUMN_BORDER", 7));

    document.addLayer(
        DxfLayer("COLUMN_DEPTH", 7));

    document.addLayer(
        DxfLayer("COLUMN_NUMBERS", 7));

    document.addLayer(
        DxfLayer("COLUMN_INDICATORS", 7));

    document.addLayer(
        DxfLayer("COLUMN_SAMPLES", 7));

    /*
     * Уровень грунтовых вод:
     *
     *   ACI 5 = синий;
     *   DASHED = штриховая линия.
     *
     * На этом этапе отдельная условная легенда для воды
     * не создаётся — только графическое обозначение
     * непосредственно на колонках.
     */
    document.addLayer(
        DxfLayer(
            "COLUMN_GROUNDWATER",
            5,
            "DASHED"));

    double currentX = 0.0;

    /*
     * ------------------------------------------------------------
     * Обходим все скважины
     * ------------------------------------------------------------
     */

    for (const Borehole& borehole :
         deposit.getBoreholes())
    {
        const BoreholeAnalysisResult* boreholeResult =
            findBoreholeResult(
                analysisResult,
                borehole.getBoreholeNumber());

        if (boreholeResult == nullptr)
        {
            continue;
        }

        const BoreholeGeology& geology =
            boreholeResult->getGeology();

        const double xLeft =
            currentX;

        const double xRight =
            currentX + COLUMN_WIDTH;

        /*
         * --------------------------------------------------------
         * Номер скважины
         * --------------------------------------------------------
         */

        document.addText(
            "COLUMN_NUMBERS",
            xLeft,
            4.0,
            TEXT_HEIGHT,
            borehole.getBoreholeNumber());

        /*
         * --------------------------------------------------------
         * Собираем геологические границы
         * --------------------------------------------------------
         */

        std::vector<double> depths;

        depths.push_back(0.0);

        /*
         * Поверхностный слой.
         */

        if (geology.hasSurfaceSoil())
        {
            const auto& surfaceSoil =
                geology.getSurfaceSoil();

            if (surfaceSoil.has_value())
            {
                depths.push_back(
                    surfaceSoil->getRoofDepth());

                depths.push_back(
                    surfaceSoil->getBottomDepth());
            }
        }

        /*
         * Вскрышные породы.
         */

        for (const auto& section :
             geology.getOverburden())
        {
            depths.push_back(
                section.getRoofDepth());

            depths.push_back(
                section.getBottomDepth());
        }

        /*
         * Полезное ископаемое.
         *
         * Сухая и обводнённая части находятся
         * внутри UsefulMineral.
         */

        const UsefulMineral& usefulMineral =
            geology.getUsefulMineral();

        for (const auto& section :
             usefulMineral.getDrySections())
        {
            depths.push_back(
                section.getRoofDepth());

            depths.push_back(
                section.getBottomDepth());
        }

        for (const auto& section :
             usefulMineral.getWaterBearingSections())
        {
            depths.push_back(
                section.getRoofDepth());

            depths.push_back(
                section.getBottomDepth());
        }

        /*
         * Подстилающие породы.
         */

        for (const auto& section :
             geology.getUnderlyingRock())
        {
            depths.push_back(
                section.getRoofDepth());

            depths.push_back(
                section.getBottomDepth());
        }

        /*
         * --------------------------------------------------------
         * Удаляем некорректные значения
         * --------------------------------------------------------
         */

        depths.erase(
            std::remove_if(
                depths.begin(),
                depths.end(),
                [](double depth)
                {
                    return !std::isfinite(depth) ||
                           depth < 0.0;
                }),
            depths.end());

        /*
         * --------------------------------------------------------
         * Сортировка
         * --------------------------------------------------------
         */

        std::sort(
            depths.begin(),
            depths.end());

        /*
         * --------------------------------------------------------
         * Удаляем почти одинаковые глубины
         * --------------------------------------------------------
         */

        std::vector<double> uniqueDepths;

        for (double depth : depths)
        {
            if (uniqueDepths.empty() ||
                std::abs(
                    uniqueDepths.back() - depth) > 0.001)
            {
                uniqueDepths.push_back(depth);
            }
        }

        depths = uniqueDepths;

        /*
         * --------------------------------------------------------
         * Добавляем забой скважины
         * --------------------------------------------------------
         */

        const double totalDepth =
            borehole.getTotalDepth();

        if (totalDepth > 0.0)
        {
            depths.push_back(totalDepth);
        }

        std::sort(
            depths.begin(),
            depths.end());

        uniqueDepths.clear();

        for (double depth : depths)
        {
            if (uniqueDepths.empty() ||
                std::abs(
                    uniqueDepths.back() - depth) > 0.001)
            {
                uniqueDepths.push_back(depth);
            }
        }

        depths = uniqueDepths;

        /*
         * --------------------------------------------------------
         * Максимальная глубина столбика
         * --------------------------------------------------------
         */

        double maxDepth =
            totalDepth;

        for (double depth : depths)
        {
            maxDepth =
                std::max(maxDepth, depth);
        }

        if (maxDepth <= 0.0)
        {
            currentX += COLUMN_SPACING;
            continue;
        }

        /*
         * --------------------------------------------------------
         * Рамка геологического столбика
         * --------------------------------------------------------
         */

        const double yTop = 0.0;

        const double yBottom =
            -maxDepth * DEPTH_SCALE;

        document.addLine(
            "COLUMN_BORDER",
            xLeft,
            yTop,
            xLeft,
            yBottom);

        document.addLine(
            "COLUMN_BORDER",
            xRight,
            yTop,
            xRight,
            yBottom);

        document.addLine(
            "COLUMN_BORDER",
            xLeft,
            yTop,
            xRight,
            yTop);

        document.addLine(
            "COLUMN_BORDER",
            xLeft,
            yBottom,
            xRight,
            yBottom);

        /*
         * --------------------------------------------------------
         * Геологические границы
         * --------------------------------------------------------
         */

        for (double depth : depths)
        {
            if (depth < 0.0 ||
                depth > maxDepth)
            {
                continue;
            }

            addDepthBoundary(
                document,
                xLeft,
                xRight,
                depth,
                borehole.getCollarElevation(),
                DEPTH_SCALE,
                TEXT_HEIGHT,
                TEXT_OFFSET,
                DEPTH_MARK_LENGTH);
        }

        /*
         * --------------------------------------------------------
         * Вертикальная шкала абсолютных отметок
         * --------------------------------------------------------
         *
         * Шкала строится с шагом ровно 1 м.
         *
         * Верхняя отметка:
         *     ceil(отметка устья)
         *
         * Нижняя отметка:
         *     floor(отметка забоя)
         *
         * Например:
         *
         *     устье = 181.3 м
         *     забой = 169.8 м
         *
         * шкала будет от 182 до 169 м.
         *
         * Шкала может немного выходить за рамку колонки сверху/снизу —
         * это намеренно: крайние целые отметки должны присутствовать.
         *
         * Подписи абсолютных отметок располагаются СЛЕВА от шкалы.
         */

        const double collarElevation =
            borehole.getCollarElevation();

        const double bottomElevation =
            collarElevation - maxDepth;

        const double scaleTopElevation =
            std::ceil(collarElevation - 1e-9);

        const double scaleBottomElevation =
            std::floor(bottomElevation + 1e-9);

        const double scaleX =
            xLeft - 8.0;

        const double scaleTopY =
            (scaleTopElevation - collarElevation) * DEPTH_SCALE;

        const double scaleBottomY =
            (scaleBottomElevation - collarElevation) * DEPTH_SCALE;

        if (scaleTopElevation >= scaleBottomElevation)
        {
            document.addLine(
                "COLUMN_DEPTH",
                scaleX,
                scaleTopY,
                scaleX,
                scaleBottomY);

            for (double elevation = scaleTopElevation;
                 elevation >= scaleBottomElevation - 1e-9;
                 elevation -= 1.0)
            {
                const double y =
                    (elevation - collarElevation) * DEPTH_SCALE;

                document.addLine(
                    "COLUMN_DEPTH",
                    scaleX,
                    y,
                    scaleX + 2.0,
                    y);

                document.addText(
                    "COLUMN_DEPTH",
                    scaleX - TEXT_OFFSET - 7.0,
                    y - TEXT_HEIGHT / 2.0,
                    TEXT_HEIGHT,
                    formatScaleElevation(elevation));
            }
        }

        /*
         * --------------------------------------------------------
         * УРОВЕНЬ ГРУНТОВЫХ ВОД
         * --------------------------------------------------------
         *
         * Для графики используется ФАКТИЧЕСКИЙ уровень
         * конкретной скважины.
         *
         * Это намеренно отличается от расчётов полезного
         * ископаемого: расчёты могут использовать средний
         * уровень УГВ по месторождению, а на геологической
         * колонке показывается фактический УГВ данной скважины.
         */

        if (borehole.getGroundwaterDepth() > 0.0 &&
            std::isfinite(
                borehole.getGroundwaterElevation()))
        {
            const double groundwaterElevation =
                borehole.getGroundwaterElevation();

            const double groundwaterDepth =
                collarElevation - groundwaterElevation;

            if (groundwaterDepth >= 0.0 &&
                groundwaterDepth <= maxDepth)
            {
                const double groundwaterY =
                    -groundwaterDepth * DEPTH_SCALE;

                /*
                 * Штриховая линия проходит через весь ствол
                 * и немного выступает вправо для читаемости.
                 */
                document.addLine(
                    "COLUMN_GROUNDWATER",
                    xLeft - 2.0,
                    groundwaterY,
                    xRight + 8.0,
                    groundwaterY);

                document.addText(
                    "COLUMN_GROUNDWATER",
                    xRight + 10.0,
                    groundwaterY + TEXT_HEIGHT * 0.2,
                    TEXT_HEIGHT,
                    "УГВ " +
                        formatNumber(groundwaterElevation) +
                        " м");
            }
        }

        /*
         * --------------------------------------------------------
         * ОБРАЗЦЫ
         * --------------------------------------------------------
         *
         * Каждый образец отображается по своему
         * фактическому интервалу.
         */

        for (const Sample& sample :
             borehole.getSamples())
        {
            /*
             * Пропускаем некорректные интервалы.
             */

            if (sample.getEndDepth() <=
                sample.getStartDepth())
            {
                continue;
            }

            /*
             * Образец полностью ниже забоя
             * показывать не нужно.
             */

            if (sample.getStartDepth() >= maxDepth)
            {
                continue;
            }

            addSampleInterval(
                document,
                sample,
                xLeft,
                xRight,
                DEPTH_SCALE);
        }

        /*
         * --------------------------------------------------------
         * Следующая скважина
         * --------------------------------------------------------
         */

        currentX += COLUMN_SPACING;
    }

    /*
     * ------------------------------------------------------------
     * Сохраняем DXF
     * ------------------------------------------------------------
     */

    document.save(filePath);
}