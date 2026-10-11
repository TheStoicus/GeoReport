#include "../../../headers/graphics/dwg/DwgGeologicalSectionWriter.h"

#include "../../../headers/domain/Borehole.h"
#include "../../../headers/domain/Sample.h"
#include "../../../headers/graphics/dxf/DxfDocument.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
    constexpr double VERTICAL_SCALE = 10.0;
    constexpr double BOREHOLE_HALF_WIDTH = 2.5;
    constexpr double TEXT_HEIGHT = 2.5;
    constexpr double TITLE_HEIGHT = 4.0;
    constexpr double LABEL_OFFSET = 5.0;
    constexpr double SCALE_OFFSET = 12.0;
    constexpr double SECTION_GAP = 40.0;
    constexpr double EPS = 1e-9;

    struct LithologyInterval
    {
        double topElevation = 0.0;
        double bottomElevation = 0.0;
        double topDepth = 0.0;
        double bottomDepth = 0.0;
        std::string key;
    };

    struct SectionBorehole
    {
        const Borehole* borehole = nullptr;
        const BoreholeAnalysisResult* result = nullptr;
        double x = 0.0;
        std::vector<LithologyInterval> intervals;
    };

    bool isRegularFile(const std::filesystem::path& path)
    {
        std::error_code error;
        return std::filesystem::exists(path, error) &&
               std::filesystem::is_regular_file(path, error);
    }

    std::string trim(const std::string& value)
    {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return {};

        const auto last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    std::string lithologyKey(const Sample& sample)
    {
        const std::string value = trim(sample.getSoilType());
        return value.empty() ? "Не определено" : value;
    }

    std::string formatNumber(double value, int precision = 2)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(precision) << value;
        return stream.str();
    }

    std::string formatElevation(double value)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(0) << value;
        return stream.str();
    }

    const BoreholeAnalysisResult* findResult(
        const GeologicalAnalysisResult& analysisResult,
        const std::string& boreholeNumber)
    {
        for (const auto& result : analysisResult.getBoreholes())
        {
            if (result.getBoreholeNumber() == boreholeNumber)
                return &result;
        }
        return nullptr;
    }

    const Borehole* findBorehole(
        const Deposit& deposit,
        const std::string& number)
    {
        return deposit.findBorehole(number);
    }

    double planDistance(const Borehole& first, const Borehole& second)
    {
        const Coordinate a = first.getCoordinate();
        const Coordinate b = second.getCoordinate();

        const double dx = b.second - a.second;
        const double dy = b.first - a.first;
        return std::sqrt(dx * dx + dy * dy);
    }

    std::vector<LithologyInterval> collectIntervals(const Borehole& borehole)
    {
        std::vector<LithologyInterval> intervals;

        const double collar = borehole.getCollarElevation();
        const double totalDepth = borehole.getTotalDepth();

        for (const Sample& sample : borehole.getSamples())
        {
            const double topDepth = std::max(0.0, sample.getStartDepth());
            const double bottomDepth =
                std::min(totalDepth, sample.getEndDepth());

            if (bottomDepth <= topDepth + EPS)
                continue;

            intervals.push_back({
                collar - topDepth,
                collar - bottomDepth,
                topDepth,
                bottomDepth,
                lithologyKey(sample)});
        }

        return intervals;
    }

    void addIntervalConnection(
        DxfDocument& document,
        const std::string& layer,
        double x1,
        double x2,
        double top1,
        double bottom1,
        double top2,
        double bottom2,
        double maxCollar,
        double baseY)
    {
        const double top = std::min(top1, top2);
        const double bottom = std::max(bottom1, bottom2);

        if (bottom >= top - EPS)
            return;

        document.addPolyline(
            layer,
            {
                {x1, baseY + (top1 - maxCollar) * VERTICAL_SCALE},
                {x2, baseY + (top2 - maxCollar) * VERTICAL_SCALE},
                {x2, baseY + (bottom2 - maxCollar) * VERTICAL_SCALE},
                {x1, baseY + (bottom1 - maxCollar) * VERTICAL_SCALE}
            },
            true);
    }

    void addBorehole(
        DxfDocument& document,
        const SectionBorehole& item,
        double maxCollar,
        double baseY,
        const std::map<std::string, std::string>& lithologyLayers)
    {
        const double x = item.x;
        const double topElevation = item.borehole->getCollarElevation();
        const double bottomElevation =
            topElevation - item.borehole->getTotalDepth();

        document.addLine(
            "SECTION_BOREHOLES",
            x,
            baseY + (topElevation - maxCollar) * VERTICAL_SCALE,
            x,
            baseY + (bottomElevation - maxCollar) * VERTICAL_SCALE);

        document.addText(
            "SECTION_LABELS",
            x - BOREHOLE_HALF_WIDTH,
            baseY + (topElevation - maxCollar) * VERTICAL_SCALE + LABEL_OFFSET,
            TEXT_HEIGHT,
            "Скв. " + item.borehole->getBoreholeNumber());

        document.addText(
            "SECTION_LABELS",
            x - BOREHOLE_HALF_WIDTH,
            baseY + (topElevation - maxCollar) * VERTICAL_SCALE + 1.0,
            TEXT_HEIGHT,
            "Отм. " + formatNumber(topElevation) + " м");

        for (const LithologyInterval& interval : item.intervals)
        {
            const double yTop =
                baseY + (interval.topElevation - maxCollar) * VERTICAL_SCALE;
            const double yBottom =
                baseY + (interval.bottomElevation - maxCollar) * VERTICAL_SCALE;

            const std::string& layer = lithologyLayers.at(interval.key);

            document.addLine(
                layer,
                x - BOREHOLE_HALF_WIDTH,
                yTop,
                x + BOREHOLE_HALF_WIDTH,
                yTop);

            document.addLine(
                layer,
                x - BOREHOLE_HALF_WIDTH,
                yBottom,
                x + BOREHOLE_HALF_WIDTH,
                yBottom);

            // Подписываем кровлю и подошву каждого интервала в глубинах.
            document.addText(
                "SECTION_DEPTH_LABELS",
                x + BOREHOLE_HALF_WIDTH + 1.5,
                yTop - TEXT_HEIGHT / 2.0,
                TEXT_HEIGHT,
                formatNumber(interval.topDepth, 2) + " м");

            document.addText(
                "SECTION_DEPTH_LABELS",
                x + BOREHOLE_HALF_WIDTH + 1.5,
                yBottom - TEXT_HEIGHT / 2.0,
                TEXT_HEIGHT,
                formatNumber(interval.bottomDepth, 2) + " м");

            const Sample* samplePtr = nullptr;
            for (const Sample& sample : item.borehole->getSamples())
            {
                if (std::abs(sample.getStartDepth() - interval.topDepth) < EPS &&
                    std::abs(sample.getEndDepth() - interval.bottomDepth) < EPS)
                {
                    samplePtr = &sample;
                    break;
                }
            }

            std::string intervalLabel =
                samplePtr != nullptr ? samplePtr->getSampleNumber() : "";
            const std::string rockType =
                samplePtr != nullptr ? samplePtr->getRockType() : interval.key;

            if (!rockType.empty())
            {
                if (!intervalLabel.empty())
                    intervalLabel += " | ";
                intervalLabel += rockType;
            }

            if (!intervalLabel.empty())
            {
                const double yCenter = (yTop + yBottom) / 2.0;
                document.addText(
                    "SECTION_LITHO_LABELS",
                    x + BOREHOLE_HALF_WIDTH + 8.0,
                    yCenter - TEXT_HEIGHT / 2.0,
                    TEXT_HEIGHT,
                    intervalLabel);
            }
        }

        if (item.borehole->getGroundwaterDepth() > 0.0 &&
            std::isfinite(item.borehole->getGroundwaterElevation()))
        {
            const double gwElevation =
                item.borehole->getGroundwaterElevation();

            if (gwElevation <= topElevation + EPS &&
                gwElevation >= bottomElevation - EPS)
            {
                const double y =
                    baseY + (gwElevation - maxCollar) * VERTICAL_SCALE;

                document.addLine(
                    "SECTION_GROUNDWATER",
                    x - BOREHOLE_HALF_WIDTH - 3.0,
                    y,
                    x + BOREHOLE_HALF_WIDTH + 3.0,
                    y);

                document.addText(
                    "SECTION_GROUNDWATER",
                    x + BOREHOLE_HALF_WIDTH + 4.0,
                    y + 0.4,
                    TEXT_HEIGHT,
                    "УГВ " + formatNumber(gwElevation) + " м");
            }
        }
    }

    double sectionTopY(double maxCollar, double scaleTop, double baseY)
    {
        return baseY + (scaleTop - maxCollar) * VERTICAL_SCALE;
    }
}

std::string DwgGeologicalSectionWriter::quoteShellArgument(
    const std::string& value)
{
    std::string result = "'";

    for (const char character : value)
    {
        if (character == '\'')
            result += "'\\''";
        else
            result += character;
    }

    result += "'";
    return result;
}

std::filesystem::path DwgGeologicalSectionWriter::findDwgWriteExecutable(
    const std::filesystem::path& projectRoot) const
{
#ifdef GEOREPORT_DWGWRITE_PATH
    {
        const std::filesystem::path candidate(GEOREPORT_DWGWRITE_PATH);
        if (isRegularFile(candidate))
            return candidate;
    }
#endif

    if (const char* environment = std::getenv("GEOREPORT_DWGWRITE"))
    {
        const std::filesystem::path candidate(environment);
        if (isRegularFile(candidate))
            return candidate;
    }

    const std::filesystem::path librariesRoot =
        projectRoot.parent_path().parent_path() / "CppLibs";
    const std::filesystem::path libreDwgRoot = librariesRoot / "LibreDWG";

    const std::filesystem::path candidates[] =
    {
        libreDwgRoot / "programs" / "dwgwrite",
        libreDwgRoot / "src" / "dwgwrite",
        libreDwgRoot / "dwgwrite"
    };

    for (const auto& candidate : candidates)
    {
        if (isRegularFile(candidate))
            return candidate;
    }

    throw std::runtime_error(
        "Не найден LibreDWG dwgwrite. Пересоберите проект CMake "
        "или задайте переменную окружения GEOREPORT_DWGWRITE.");
}

void DwgGeologicalSectionWriter::write(
    const std::filesystem::path& filePath,
    const std::filesystem::path& projectRoot,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult,
    const SectionSettings& sectionSettings) const
{
    std::filesystem::create_directories(filePath.parent_path());

    const std::filesystem::path dwgWrite =
        findDwgWriteExecutable(projectRoot);

    const std::filesystem::path temporaryDxf =
        filePath.parent_path() / ".georeport_sections_source.dxf";
    const std::filesystem::path logFile =
        filePath.parent_path() / ".georeport_sections_dwgwrite.log";

    DxfDocument document;
    document.addLayer(DxfLayer("SECTION_BOREHOLES", 7));
    document.addLayer(DxfLayer("SECTION_LABELS", 7));
    document.addLayer(DxfLayer("SECTION_DEPTH_LABELS", 8));
    document.addLayer(DxfLayer("SECTION_LITHO_LABELS", 7));
    document.addLayer(DxfLayer("SECTION_SCALE", 7));
    document.addLayer(DxfLayer("SECTION_GROUNDWATER", 5, "DASHED"));

    const auto& sections = sectionSettings.getSections();

    if (sections.empty())
    {
        throw std::runtime_error(
            "Не задано ни одного разреза. Запустите программу повторно "
            "и укажите скважины для построения разрезов.");
    }

    double currentBaseY = 0.0;

    for (std::size_t sectionIndex = 0;
         sectionIndex < sections.size();
         ++sectionIndex)
    {
        std::vector<SectionBorehole> boreholes;

        for (const std::string& number : sections[sectionIndex])
        {
            const Borehole* borehole = findBorehole(deposit, number);
            const BoreholeAnalysisResult* result =
                findResult(analysisResult, number);

            if (borehole == nullptr || result == nullptr)
                continue;

            SectionBorehole item;
            item.borehole = borehole;
            item.result = result;
            item.intervals = collectIntervals(*borehole);
            boreholes.push_back(std::move(item));
        }

        if (boreholes.size() < 2)
            continue;

        boreholes.front().x = 0.0;
        for (std::size_t i = 1; i < boreholes.size(); ++i)
        {
            boreholes[i].x = boreholes[i - 1].x +
                planDistance(
                    *boreholes[i - 1].borehole,
                    *boreholes[i].borehole);
        }

        std::map<std::string, std::string> lithologyLayers;
        int lithologyIndex = 1;

        for (const SectionBorehole& borehole : boreholes)
        {
            for (const LithologyInterval& interval : borehole.intervals)
            {
                if (!lithologyLayers.count(interval.key))
                {
                    const std::string layerName =
                        "SEC" + std::to_string(sectionIndex + 1) +
                        "_LITHO_" + std::to_string(lithologyIndex++);

                    lithologyLayers[interval.key] = layerName;
                    document.addLayer(DxfLayer(layerName, 7));
                }
            }
        }

        double maxCollar = boreholes.front().borehole->getCollarElevation();
        double minBottom = maxCollar;

        for (const SectionBorehole& borehole : boreholes)
        {
            maxCollar = std::max(
                maxCollar,
                borehole.borehole->getCollarElevation());

            minBottom = std::min(
                minBottom,
                borehole.borehole->getCollarElevation() -
                    borehole.borehole->getTotalDepth());
        }

        const double scaleTop = std::ceil(maxCollar - EPS);
        const double scaleBottom = std::floor(minBottom + EPS);

        for (const SectionBorehole& borehole : boreholes)
        {
            addBorehole(
                document,
                borehole,
                maxCollar,
                currentBaseY,
                lithologyLayers);
        }

        const double scaleX = boreholes.front().x - SCALE_OFFSET;

        document.addText(
            "SECTION_SCALE",
            scaleX - 18.0,
            sectionTopY(maxCollar, scaleTop, currentBaseY) + 4.0,
            TEXT_HEIGHT,
            "Абс. отметка, м");

        document.addLine(
            "SECTION_SCALE",
            scaleX,
            currentBaseY + (scaleTop - maxCollar) * VERTICAL_SCALE,
            scaleX,
            currentBaseY + (scaleBottom - maxCollar) * VERTICAL_SCALE);

        for (double elevation = scaleTop;
             elevation >= scaleBottom - EPS;
             elevation -= 1.0)
        {
            const double y =
                currentBaseY + (elevation - maxCollar) * VERTICAL_SCALE;

            document.addLine(
                "SECTION_SCALE",
                scaleX,
                y,
                scaleX + 2.0,
                y);

            document.addText(
                "SECTION_SCALE",
                scaleX - 8.0,
                y - TEXT_HEIGHT / 2.0,
                TEXT_HEIGHT,
                formatElevation(elevation));
        }

        const double sectionCenter =
            (boreholes.front().x + boreholes.back().x) / 2.0;

        document.addText(
            "SECTION_LABELS",
            sectionCenter - 25.0,
            sectionTopY(maxCollar, scaleTop, currentBaseY) + 12.0,
            TITLE_HEIGHT,
            "Геолого-литологический разрез №" +
                std::to_string(sectionIndex + 1));

        std::string boreholeList;
        for (std::size_t i = 0; i < sections[sectionIndex].size(); ++i)
        {
            if (i != 0)
                boreholeList += " - ";
            boreholeList += sections[sectionIndex][i];
        }

        document.addText(
            "SECTION_LABELS",
            sectionCenter - 20.0,
            sectionTopY(maxCollar, scaleTop, currentBaseY) + 7.0,
            TEXT_HEIGHT,
            "Скважины: " + boreholeList);

        // Следующий разрез размещаем ниже предыдущего с запасом.
        currentBaseY -=
            ((scaleTop - scaleBottom) * VERTICAL_SCALE) + SECTION_GAP;
    }

    document.save(temporaryDxf.string());

    const std::string command =
        quoteShellArgument(dwgWrite.string()) +
        " --as r2000 -y -o " +
        quoteShellArgument(filePath.string()) +
        " " +
        quoteShellArgument(temporaryDxf.string()) +
        " > " +
        quoteShellArgument(logFile.string()) +
        " 2>&1";

    const int result = std::system(command.c_str());

    if (result != 0 || !std::filesystem::exists(filePath))
    {
        std::string details;
        std::ifstream log(logFile);

        if (log.is_open())
        {
            details.assign(
                (std::istreambuf_iterator<char>(log)),
                std::istreambuf_iterator<char>());
        }

        throw std::runtime_error(
            "LibreDWG не смог создать DWG с разрезами. Код возврата: " +
            std::to_string(result) +
            ". Подробности: " +
            (details.empty() ? logFile.string() : details));
    }

    std::error_code error;
    std::filesystem::remove(temporaryDxf, error);
    std::filesystem::remove(logFile, error);
}
