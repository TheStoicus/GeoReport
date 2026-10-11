#include "../../../headers/graphics/cad/BoreholePlanDrawingBuilder.h"

#include "../../../headers/calculations/MineralCalculationPart.h"
#include "../../../headers/domain/Sample.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
    struct Point
    {
        double x = 0.0;
        double y = 0.0;
    };

    constexpr double BOREHOLE_RADIUS = 2.0;
    constexpr double TEXT_HEIGHT = 2.5;
    constexpr double LINE_LENGTH = 15.0;
    constexpr double TEXT_OFFSET = 3.0;
    constexpr double TEXT_VERTICAL_OFFSET = 1.5;
    constexpr double GROUNDWATER_OFFSET = 5.0;
    constexpr double EPS = 1e-9;

    std::pair<std::string, std::string> parseBlockDesignation(
        const std::string& blockName)
    {
        // Для графических обозначений категории всегда распознаём
        // по стандартным суффиксам B, C1 и C2. Это важно, например,
        // для IIIB: категория здесь именно B, а не IB.
        const std::string categories[] = {"C1", "C2", "B"};

        for (const std::string& category : categories)
        {
            if (blockName.size() > category.size() &&
                blockName.compare(
                    blockName.size() - category.size(),
                    category.size(),
                    category) == 0)
            {
                return {
                    blockName.substr(0, blockName.size() - category.size()),
                    category};
            }
        }

        return {blockName, {}};
    }

    std::string contourLayerName(const std::string& blockName)
    {
        const auto designation = parseBlockDesignation(blockName);

        std::string result = "BLOCK_" + designation.first;

        if (!designation.second.empty())
            result += "_" + designation.second;

        return result;
    }

    std::string contourLineType(const std::string& category)
    {
        if (category == "B")
            return "DASHED";

        if (category == "C1")
            return "DASHDOT";

        if (category == "C2")
            return "DASHDOTDOT";

        return "CONTINUOUS";
    }

    bool getBoreholePoint(
        const Deposit& deposit,
        const std::string& boreholeNumber,
        Point& point)
    {
        const Borehole* borehole = deposit.findBorehole(boreholeNumber);

        if (borehole == nullptr)
            return false;

        const Coordinate coordinate = borehole->getCoordinate();

        if (coordinate.system != CoordinateSystem::SK1963 ||
            !std::isfinite(coordinate.first) ||
            !std::isfinite(coordinate.second))
        {
            return false;
        }

        point = {coordinate.first, coordinate.second};
        return true;
    }

    double cross(const Point& a, const Point& b, const Point& c)
    {
        return (b.x - a.x) * (c.y - a.y) -
               (b.y - a.y) * (c.x - a.x);
    }

    std::vector<Point> buildAutomaticContour(
        const Block& block,
        const Deposit& deposit)
    {
        std::vector<Point> points;

        for (const std::string& boreholeNumber : block.getBoreholeNumbers())
        {
            Point point;

            if (getBoreholePoint(deposit, boreholeNumber, point))
                points.push_back(point);
        }

        if (points.size() < 3)
            return {};

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
            return {};

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
            return {};

        hull.pop_back();
        return hull;
    }

    std::vector<Point> buildManualContour(
        const Block& block,
        const Deposit& deposit,
        const ReserveSummarySettings& settings)
    {
        const ReserveSummaryBlockSettings* blockSettings =
            settings.findBlock(block.getName());

        if (blockSettings == nullptr)
            return {};

        std::vector<Point> points;

        for (const std::string& boreholeNumber :
             blockSettings->boundaryBoreholes)
        {
            Point point;

            if (getBoreholePoint(deposit, boreholeNumber, point))
                points.push_back(point);
        }

        if (points.size() < 3)
            return {};

        return points;
    }

    std::vector<Point> buildBlockContour(
        const Block& block,
        const Deposit& deposit,
        const ReserveSummarySettings& settings)
    {
        if (settings.getAreaMode() == BlockAreaMode::ManualBoundary)
        {
            return buildManualContour(block, deposit, settings);
        }

        return buildAutomaticContour(block, deposit);
    }

    std::string formatNumber(double value)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << value;
        return stream.str();
    }

    const BoreholeAnalysisResult* findAnalysisResult(
        const GeologicalAnalysisResult& analysisResult,
        const std::string& boreholeNumber)
    {
        for (const BoreholeAnalysisResult& result :
             analysisResult.getBoreholes())
        {
            if (result.getBoreholeNumber() == boreholeNumber)
                return &result;
        }

        return nullptr;
    }

}

void BoreholePlanDrawingBuilder::build(
    DxfDocument& document,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult,
    const ReserveSummarySettings& reserveSummarySettings,
    bool includeBlockContours) const
{
    document.addLayer(DxfLayer("BOREHOLES", 7));
    document.addLayer(DxfLayer("BOREHOLE_NUMBERS", 7));
    document.addLayer(DxfLayer("ELEVATIONS", 7));
    document.addLayer(DxfLayer("INDICATORS", 7));

    if (includeBlockContours)
    {
        for (const Block& block : deposit.getBlocks())
        {
            const auto designation = parseBlockDesignation(block.getName());
            const std::string layerName = contourLayerName(block.getName());
            const std::string lineType = contourLineType(designation.second);

            document.addLayer(DxfLayer(layerName, 7, lineType));

            const std::vector<Point> contour =
                buildBlockContour(block, deposit, reserveSummarySettings);

            if (contour.size() < 3)
                continue;

            // Контур блока — единая замкнутая полилиния.
            // Координаты передаются в CAD в том же порядке X/Y,
            // что и для скважин: CAD X = SK1963 Y, CAD Y = SK1963 X.
            std::vector<std::pair<double, double>> polylinePoints;
            polylinePoints.reserve(contour.size());

            for (const Point& point : contour)
            {
                polylinePoints.emplace_back(point.y, point.x);
            }

            document.addPolyline(
                layerName,
                polylinePoints,
                true,
                lineType);
        }
    }

    for (const Borehole& borehole : deposit.getBoreholes())
    {
        const Coordinate coordinate = borehole.getCoordinate();

        if (coordinate.system != CoordinateSystem::SK1963)
            continue;

        const double cadX = coordinate.second;
        const double cadY = coordinate.first;

        document.addCircle(
            "BOREHOLES",
            cadX,
            cadY,
            BOREHOLE_RADIUS);

        document.addLine(
            "INDICATORS",
            cadX - LINE_LENGTH,
            cadY,
            cadX,
            cadY);

        document.addLine(
            "INDICATORS",
            cadX,
            cadY,
            cadX + LINE_LENGTH,
            cadY);

        const BoreholeAnalysisResult* boreholeResult =
            findAnalysisResult(
                analysisResult,
                borehole.getBoreholeNumber());

        document.addText(
            "BOREHOLE_NUMBERS",
            cadX - LINE_LENGTH + TEXT_OFFSET,
            cadY + TEXT_VERTICAL_OFFSET,
            TEXT_HEIGHT,
            borehole.getBoreholeNumber());

        document.addText(
            "ELEVATIONS",
            cadX - LINE_LENGTH + TEXT_OFFSET,
            cadY - TEXT_VERTICAL_OFFSET - TEXT_HEIGHT,
            TEXT_HEIGHT,
            formatNumber(borehole.getCollarElevation()));

        double overburdenThickness = 0.0;
        double prsThickness = 0.0;
        double usefulMineralThickness = 0.0;
        double dryMineralThickness = 0.0;
        double waterBearingMineralThickness = 0.0;
        double groundwaterDepth = borehole.getGroundwaterDepth();

        if (boreholeResult != nullptr)
        {
            const MainIndicators& indicators =
                boreholeResult->getIndicators();

            overburdenThickness = indicators.getOverburdenThickness();
            usefulMineralThickness = indicators.getUsefulMineralThickness();
            dryMineralThickness = indicators.getDryMineralThickness();
            waterBearingMineralThickness =
                indicators.getWaterBearingMineralThickness();
            groundwaterDepth = indicators.getGroundwaterDepth();

        }

        double firstSampleStart = std::numeric_limits<double>::infinity();

        for (const Sample& sample : borehole.getSamples())
        {
            if (sample.isMonolith() || sample.isAgSample())
                continue;

            if (sample.getEndDepth() <= sample.getStartDepth() + EPS)
                continue;

            firstSampleStart = std::min(
                firstSampleStart,
                std::max(0.0, sample.getStartDepth()));
        }

        if (std::isfinite(firstSampleStart))
            prsThickness = firstSampleStart;

        document.addText(
            "INDICATORS",
            cadX + TEXT_OFFSET,
            cadY + TEXT_VERTICAL_OFFSET,
            TEXT_HEIGHT,
            formatNumber(overburdenThickness) +
                " (" + formatNumber(prsThickness) + ")");

        document.addText(
            "INDICATORS",
            cadX + TEXT_OFFSET,
            cadY - TEXT_VERTICAL_OFFSET - TEXT_HEIGHT,
            TEXT_HEIGHT,
            formatNumber(usefulMineralThickness));

        document.addText(
            "INDICATORS",
            cadX + TEXT_OFFSET,
            cadY - TEXT_VERTICAL_OFFSET - TEXT_HEIGHT - 3.0,
            TEXT_HEIGHT,
            formatNumber(dryMineralThickness));

        document.addText(
            "INDICATORS",
            cadX + TEXT_OFFSET,
            cadY - TEXT_VERTICAL_OFFSET - TEXT_HEIGHT - 6.0,
            TEXT_HEIGHT,
            formatNumber(waterBearingMineralThickness));

        document.addText(
            "INDICATORS",
            cadX - TEXT_HEIGHT,
            cadY - GROUNDWATER_OFFSET,
            TEXT_HEIGHT,
            formatNumber(groundwaterDepth));
    }
}
