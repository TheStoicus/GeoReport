#include "../../headers/writers/ReserveSummaryWriter.h"
#include "../../headers/writers/BlockDesignation.h"

#include "../../headers/calculations/WeightedAverageCalculator.h"
#include "../../headers/excel/ExcelWriteUtils.h"

#include <OpenXLSX.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace
{
    constexpr double EPS = 1e-9;
    constexpr double CUBIC_METERS_IN_THOUSAND = 1000.0;
    constexpr const char* GRAVEL_PARAMETER = "Грансостав гравия >5 мм";
}

const BoreholeAnalysisResult*
ReserveSummaryWriter::findBoreholeResult(
    const GeologicalAnalysisResult& analysisResult,
    const std::string& boreholeNumber)
{
    for (const BoreholeAnalysisResult& result : analysisResult.getBoreholes())
    {
        if (result.getBoreholeNumber() == boreholeNumber)
            return &result;
    }

    return nullptr;
}

double ReserveSummaryWriter::calculatePRSThickness(
    const Borehole& borehole)
{
    double firstSampleStart = std::numeric_limits<double>::infinity();

    for (const Sample& sample : borehole.getSamples())
    {
        if (sample.isMonolith() || sample.isAgSample())
            continue;

        if (sample.getEndDepth() <= sample.getStartDepth() + EPS)
            continue;

        firstSampleStart =
            std::min(firstSampleStart, sample.getStartDepth());
    }

    if (!std::isfinite(firstSampleStart))
        return 0.0;

    return std::max(0.0, firstSampleStart);
}

double ReserveSummaryWriter::calculatePartThickness(
    const BoreholeGeology& geology,
    MineralPart part)
{
    const UsefulMineral& useful = geology.getUsefulMineral();

    switch (part)
    {
        case MineralPart::Total:
            return useful.getTotalThickness();
        case MineralPart::Dry:
            return useful.getDryThickness();
        case MineralPart::WaterBearing:
            return useful.getWaterBearingThickness();
    }

    return 0.0;
}

bool ReserveSummaryWriter::hasPart(
    const BoreholeGeology& geology,
    MineralPart part)
{
    return calculatePartThickness(geology, part) > EPS;
}

std::vector<const BoreholeAnalysisResult*>
ReserveSummaryWriter::collectResultsForBlock(
    const Block& block,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult)
{
    std::vector<const BoreholeAnalysisResult*> results;

    for (const std::string& boreholeNumber : block.getBoreholeNumbers())
    {
        const Borehole* borehole = deposit.findBorehole(boreholeNumber);
        if (borehole == nullptr)
            continue;

        const BoreholeAnalysisResult* result =
            findBoreholeResult(analysisResult, boreholeNumber);

        if (result == nullptr || !result->getSectionAnalysis().isResolved())
            continue;

        results.push_back(result);
    }

    return results;
}

double ReserveSummaryWriter::calculateAverageOverburden(
    const std::vector<const BoreholeAnalysisResult*>& results,
    const Deposit& deposit)
{
    if (results.empty())
        return 0.0;

    double sum = 0.0;

    for (const BoreholeAnalysisResult* result : results)
    {
        const Borehole* borehole =
            deposit.findBorehole(result->getBoreholeNumber());

        const double prs =
            borehole == nullptr
                ? 0.0
                : calculatePRSThickness(*borehole);

        /*
         * Лист 7: "вскрышных пород — общая" включает ПРС,
         * а отдельная колонка показывает ПРС в том числе.
         */
        sum +=
            result->getIndicators().getOverburdenThickness() +
            prs;
    }

    return sum / static_cast<double>(results.size());
}

double ReserveSummaryWriter::calculateAveragePRS(
    const std::vector<const BoreholeAnalysisResult*>& results,
    const Deposit& deposit)
{
    if (results.empty())
        return 0.0;

    double sum = 0.0;
    std::size_t count = 0;

    for (const BoreholeAnalysisResult* result : results)
    {
        const Borehole* borehole =
            deposit.findBorehole(result->getBoreholeNumber());

        if (borehole == nullptr)
            continue;

        sum += calculatePRSThickness(*borehole);
        ++count;
    }

    if (count == 0)
        return 0.0;

    return sum / static_cast<double>(count);
}

double ReserveSummaryWriter::calculateAverageMineral(
    const std::vector<const BoreholeAnalysisResult*>& results,
    MineralPart part)
{
    if (results.empty())
        return 0.0;

    double sum = 0.0;
    std::size_t count = 0;

    for (const BoreholeAnalysisResult* result : results)
    {
        const double thickness =
            calculatePartThickness(result->getGeology(), part);

        if (part != MineralPart::Total && thickness <= EPS)
            continue;

        sum += thickness;
        ++count;
    }

    if (count == 0)
        return 0.0;

    return sum / static_cast<double>(count);
}

double ReserveSummaryWriter::calculateGravelAverage(
    const Block& block,
    const Deposit& deposit,
    const std::vector<const BoreholeAnalysisResult*>& results,
    MineralPart part,
    bool& hasValue)
{
    hasValue = false;

    WeightedAverageCalculator calculator;
    std::vector<BoreholeResult> boreholeResults;

    for (const BoreholeAnalysisResult* analysisResult : results)
    {
        const Borehole* borehole =
            deposit.findBorehole(analysisResult->getBoreholeNumber());

        if (borehole == nullptr)
            continue;

        MineralCalculationPart calculationPart =
            MineralCalculationPart::TotalUsefulMineral;

        if (part == MineralPart::Dry)
            calculationPart = MineralCalculationPart::DryUsefulMineral;
        else if (part == MineralPart::WaterBearing)
            calculationPart = MineralCalculationPart::WaterBearingUsefulMineral;

        const BoreholeResult boreholeResult =
            calculator.calculateBoreholeResult(
                *borehole,
                analysisResult->getSectionAnalysis(),
                analysisResult->getGeology(),
                calculationPart);

        if (boreholeResult.getTotalThickness() <= EPS)
            continue;

        if (boreholeResult
                .getParameterThickness(GRAVEL_PARAMETER) <= EPS)
        {
            continue;
        }

        boreholeResults.push_back(boreholeResult);
    }

    (void)block;

    if (boreholeResults.empty())
        return 0.0;

    const WeightedAverageResult result =
        calculator.calculateBlock(boreholeResults);

    const auto it =
        result.getValues().find(GRAVEL_PARAMETER);

    if (it == result.getValues().end())
        return 0.0;

    hasValue = true;
    return it->second;
}

bool ReserveSummaryWriter::hasPartInBlock(
    const std::vector<const BoreholeAnalysisResult*>& results,
    MineralPart part)
{
    for (const BoreholeAnalysisResult* result : results)
    {
        if (hasPart(result->getGeology(), part))
            return true;
    }

    return false;
}

ReserveSummaryWriter::CalculationRow
ReserveSummaryWriter::calculateRow(
    const Block& block,
    const ReserveSummaryBlockSettings& blockSettings,
    const ReserveSummarySettings& settings,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult,
    MineralPart part,
    const std::string& rowName)
{
    CalculationRow result;
    const auto designation = parseBlockDesignation(block.getName());
    result.blockNumber = designation.first;
    result.category = designation.second;
    result.name = rowName;
    result.area = calculateBlockArea(block, deposit, settings);

    const auto boreholeResults =
        collectResultsForBlock(block, deposit, analysisResult);

    result.averageOverburden =
        calculateAverageOverburden(boreholeResults, deposit);

    result.averagePRS =
        calculateAveragePRS(boreholeResults, deposit);

    result.averageMineral =
        calculateAverageMineral(boreholeResults, part);

    result.hasMineral =
        hasPartInBlock(boreholeResults, part);

    result.gravelAverage =
        calculateGravelAverage(
            block,
            deposit,
            boreholeResults,
            part,
            result.hasGravelAverage);

    /*
     * Таблица 7 содержит объёмы в тыс. м3.
     * Поэтому площадь (м2) * средняя мощность (м) / 1000.
     */
    result.overburdenVolume =
        result.area * result.averageOverburden /
        CUBIC_METERS_IN_THOUSAND;

    result.prsVolume =
        result.area * result.averagePRS /
        CUBIC_METERS_IN_THOUSAND;

    result.mineralVolume =
        result.area * result.averageMineral /
        CUBIC_METERS_IN_THOUSAND;

    if (result.hasGravelAverage)
    {
        result.gravelVolume =
            result.mineralVolume *
            result.gravelAverage / 100.0;
    }

    return result;
}

void ReserveSummaryWriter::writeRow(
    OpenXLSX::XLWorksheet& sheet,
    uint32_t row,
    const CalculationRow& value)
{
    // 1 — номер блока/подблока и категория запасов.
    // Например: "III B", "III B — сухое ПИ", "IV C1".
    ExcelWriteUtils::writeString(
        sheet, row, 1,
        value.name);

    // 2 — площадь блока.
    ExcelWriteUtils::writeDouble(
        sheet, row, 2,
        roundToTwoDecimals(value.area));

    // 3 — средняя мощность вскрышных пород.
    ExcelWriteUtils::writeDouble(
        sheet, row, 3,
        roundToTwoDecimals(value.averageOverburden));

    // 4 — средняя мощность ПРС.
    ExcelWriteUtils::writeDouble(
        sheet, row, 4,
        roundToTwoDecimals(value.averagePRS));

    // 5 — средняя мощность полезного ископаемого.
    ExcelWriteUtils::writeDouble(
        sheet, row, 5,
        roundToTwoDecimals(value.averageMineral));

    // 6 — ПГС по ГОСТ 31426-2010 пока не рассчитываем.

    // 7 — средневзвешенное содержание гравия >5 мм.
    if (value.hasGravelAverage)
    {
        ExcelWriteUtils::writeDouble(
            sheet, row, 7,
            roundToTwoDecimals(value.gravelAverage));
    }

    // 8 — объём вскрышных пород, тыс. м3.
    ExcelWriteUtils::writeDouble(
        sheet, row, 8,
        roundToTwoDecimals(value.overburdenVolume));

    // 9 — объём ПРС, тыс. м3.
    ExcelWriteUtils::writeDouble(
        sheet, row, 9,
        roundToTwoDecimals(value.prsVolume));

    // 10 — объём полезного ископаемого, тыс. м3.
    ExcelWriteUtils::writeDouble(
        sheet, row, 10,
        roundToTwoDecimals(value.mineralVolume));

    // 11 — ПГС по ГОСТ 31426-2010 пока не рассчитываем.

    // 12 — объём гравия >5 мм, тыс. м3.
    if (value.hasGravelAverage)
    {
        ExcelWriteUtils::writeDouble(
            sheet, row, 12,
            roundToTwoDecimals(value.gravelVolume));
    }

    // 13 — геологический коэффициент вскрыши пока не рассчитываем.

    // 14 — отношение объёма вскрышных пород к объёму ПИ.
    if (value.mineralVolume > EPS)
    {
        ExcelWriteUtils::writeDouble(
            sheet, row, 14,
            roundToTwoDecimals(
                value.overburdenVolume /
                value.mineralVolume));
    }
}

double ReserveSummaryWriter::calculateBlockArea(
    const Block& block,
    const Deposit& deposit,
    const ReserveSummarySettings& settings)
{
    struct Point
    {
        double x = 0.0;
        double y = 0.0;
    };

    auto getPoint = [&deposit](const std::string& boreholeNumber,
                               Point& point) -> bool
    {
        const Borehole* borehole =
            deposit.findBorehole(boreholeNumber);

        if (borehole == nullptr)
            return false;

        const Coordinate coordinate =
            borehole->getCoordinate();

        if (coordinate.system != CoordinateSystem::SK1963 ||
            !std::isfinite(coordinate.first) ||
            !std::isfinite(coordinate.second))
        {
            return false;
        }

        point = {coordinate.first, coordinate.second};
        return true;
    };

    std::vector<Point> points;

    /*
     * В ручном режиме порядок скважин уже является порядком обхода
     * фактического контура блока. Именно этот полигон используется
     * и для площади, и для DXF-плана.
     */
    if (settings.getAreaMode() == BlockAreaMode::ManualBoundary)
    {
        const ReserveSummaryBlockSettings* blockSettings =
            settings.findBlock(block.getName());

        if (blockSettings != nullptr)
        {
            for (const std::string& boreholeNumber :
                 blockSettings->boundaryBoreholes)
            {
                Point point;

                if (getPoint(boreholeNumber, point))
                    points.push_back(point);
            }
        }

        if (points.size() < 3)
            return 0.0;
    }
    else
    {
        /*
         * Автоматический режим: выпуклая оболочка всех скважин.
         * Это безопасный автоматический вариант, но он может давать
         * завышенную площадь, если фактический контур блока вогнутый.
         */
        for (const std::string& boreholeNumber :
             block.getBoreholeNumbers())
        {
            Point point;

            if (getPoint(boreholeNumber, point))
                points.push_back(point);
        }

        if (points.size() < 3)
            return 0.0;

        std::sort(
            points.begin(),
            points.end(),
            [](const Point& a, const Point& b)
            {
                if (a.x != b.x)
                    return a.x < b.x;

                return a.y < b.y;
            });

        points.erase(
            std::unique(
                points.begin(),
                points.end(),
                [](const Point& a, const Point& b)
                {
                    return a.x == b.x && a.y == b.y;
                }),
            points.end());

        if (points.size() < 3)
            return 0.0;

        const auto cross =
            [](const Point& a, const Point& b, const Point& c)
            {
                return (b.x - a.x) * (c.y - a.y) -
                       (b.y - a.y) * (c.x - a.x);
            };

        std::vector<Point> hull;
        hull.reserve(points.size() * 2);

        for (const Point& point : points)
        {
            while (hull.size() >= 2 &&
                   cross(hull[hull.size() - 2], hull.back(), point) <= EPS)
            {
                hull.pop_back();
            }

            hull.push_back(point);
        }

        const std::size_t lowerSize = hull.size();

        for (std::size_t i = points.size(); i-- > 0;)
        {
            const Point& point = points[i];

            while (hull.size() > lowerSize &&
                   cross(hull[hull.size() - 2], hull.back(), point) <= EPS)
            {
                hull.pop_back();
            }

            hull.push_back(point);
        }

        if (hull.size() <= 1)
            return 0.0;

        hull.pop_back();
        points = std::move(hull);
    }

    double doubleArea = 0.0;

    for (std::size_t i = 0; i < points.size(); ++i)
    {
        const Point& a = points[i];
        const Point& b = points[(i + 1) % points.size()];

        doubleArea += a.x * b.y - b.x * a.y;
    }

    return std::abs(doubleArea) * 0.5;
}

std::pair<std::string, std::string>
ReserveSummaryWriter::parseBlockDesignation(
    const std::string& blockName)
{
    return BlockDesignation::parse(blockName);
}

double ReserveSummaryWriter::roundToTwoDecimals(double value)
{
    if (!std::isfinite(value))
        return value;

    const double factor = 100.0;
    const double epsilon =
        1e-9 * std::max(1.0, std::abs(value));

    if (value >= 0.0)
    {
        return std::floor(
                   (value + epsilon) * factor + 0.5) /
               factor;
    }

    return std::ceil(
               (value - epsilon) * factor - 0.5) /
           factor;
}

void ReserveSummaryWriter::write(
    OpenXLSX::XLWorksheet& sheet,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult,
    const ReserveSummarySettings& settings) const
{
    uint32_t row = START_ROW;

    for (const Block& block : deposit.getBlocks())
    {
        const ReserveSummaryBlockSettings* blockSettings =
            settings.findBlock(block.getName());

        if (blockSettings == nullptr)
            continue;

        const auto boreholeResults =
            collectResultsForBlock(block, deposit, analysisResult);

        if (boreholeResults.empty())
            continue;

        /*
         * Основная строка — исходный блок, всё принятое ПИ.
         */
        const auto designation =
            parseBlockDesignation(block.getName());

        writeRow(
            sheet,
            row,
            calculateRow(
                block,
                *blockSettings,
                settings,
                deposit,
                analysisResult,
                MineralPart::Total,
                "Блок " + designation.first + " " + designation.second));

        ++row;

        const bool hasDry =
            hasPartInBlock(boreholeResults, MineralPart::Dry);

        const bool hasWater =
            hasPartInBlock(boreholeResults, MineralPart::WaterBearing);

        /*
         * Если в блоке есть обе части ПИ, формируем две строки-подблока.
         * Площадь и категория наследуются от исходного блока.
         */
        if (hasDry && hasWater)
        {
            writeRow(
                sheet,
                row,
                calculateRow(
                    block,
                    *blockSettings,
                    settings,
                    deposit,
                    analysisResult,
                    MineralPart::Dry,
                    "Подблок " + designation.first + " " + designation.second + " — сухое ПИ"));

            ++row;

            writeRow(
                sheet,
                row,
                calculateRow(
                    block,
                    *blockSettings,
                    settings,
                    deposit,
                    analysisResult,
                    MineralPart::WaterBearing,
                    "Подблок " + designation.first + " " + designation.second + " — обводнённое ПИ"));

            ++row;
        }
    }
}
