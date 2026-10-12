#include "../../headers/writers/MainIndicatorsResultWriter.h"

#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/writers/BlockDesignation.h"

#include <OpenXLSX.hpp>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <limits>
#include <map>
#include <set>
#include <vector>


namespace
{
    std::string formatBlockDesignation(const std::string& name)
    {
        const auto designation = BlockDesignation::parse(name);

        if (designation.second.empty())
            return designation.first;

        return designation.first + " " + designation.second;
    }
}

const BoreholeSectionAnalysis*
MainIndicatorsResultWriter::findAnalysis(
    const std::vector<BoreholeSectionAnalysis>& analyses,
    const std::string& boreholeNumber) const
{
    for (const BoreholeSectionAnalysis& analysis : analyses)
    {
        if (analysis.getBoreholeNumber() == boreholeNumber)
        {
            return &analysis;
        }
    }

    return nullptr;
}

std::string MainIndicatorsResultWriter::getBlockNames(
    const Deposit& deposit,
    const std::string& boreholeNumber) const
{
    std::ostringstream result;
    bool first = true;

    for (const Block& block : deposit.getBlocks())
    {
        for (const std::string& number : block.getBoreholeNumbers())
        {
            if (number == boreholeNumber)
            {
                if (!first)
                {
                    result << ", ";
                }

                result << formatBlockDesignation(block.getName());
                first = false;

                break;
            }
        }
    }

    return result.str();
}

double MainIndicatorsResultWriter::calculatePRSThickness(
    const Borehole& borehole) const
{
    constexpr double EPS = 1e-9;

    /*
     * ПРС — фактический верхний неопробованный интервал.
     *
     * Например:
     *   первая обычная проба с 0.20 м -> ПРС = 0.20 м;
     *   первая обычная проба с 0.30 м -> ПРС = 0.30 м;
     *   первая обычная проба с 0.00 м -> ПРС = 0.00 м.
     *
     * Здесь специально НЕ используется тип участка из геологической
     * классификации. Вскрыша 0.00–0.80 м и ПРС 0.00–0.20 м — это
     * разные показатели и должны определяться независимо.
     *
     * АГ и монолиты не считаются обычным опробованием верхней части.
     */
    double firstSampleStart =
        std::numeric_limits<double>::infinity();

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

double MainIndicatorsResultWriter::roundHalfUp(
    double value,
    int decimals)
{
    if (!std::isfinite(value))
    {
        return value;
    }

    const double factor =
        std::pow(10.0, decimals);

    const double epsilon =
        1e-9 * std::max(1.0, std::abs(value));

    if (value >= 0.0)
    {
        return std::floor(
                   (value + epsilon) * factor + 0.5)
               / factor;
    }

    return std::ceil(
               (value - epsilon) * factor - 0.5)
           / factor;
}

double MainIndicatorsResultWriter::roundToOneDecimal(
    double value)
{
    /*
     * Несмотря на старое имя функции,
     * теперь округляем до двух знаков.
     *
     * Примеры:
     * 12.347 -> 12.35
     * 12.345 -> 12.35
     * 12.344 -> 12.34
     * 12.3   -> 12.30
     */
    return roundHalfUp(value, 2);
}

void MainIndicatorsResultWriter::writeRow(
    OpenXLSX::XLWorksheet& sheet,
    uint32_t row,
    int serialNumber,
    const std::string& blockNames,
    const MainIndicators& indicators,
    double prsThickness) const
{
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_SERIAL,
        static_cast<double>(serialNumber));

    ExcelWriteUtils::writeString(
        sheet,
        row,
        COL_BLOCK,
        blockNames);

    ExcelWriteUtils::writeString(
        sheet,
        row,
        COL_BOREHOLE,
        indicators.getBoreholeNumber());

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_BOREHOLE_DEPTH,
        roundToOneDecimal(
            indicators.getBoreholeDepth()));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_COLLAR_ELEVATION,
        roundToOneDecimal(
            indicators.getCollarElevation()));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_OVERBURDEN,
        roundToOneDecimal(
            indicators.getOverburdenThickness()));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_PRS,
        roundToOneDecimal(
            prsThickness));

    // 8 — абсолютная отметка кровли ПИ.
    const double roofElevation =
        indicators.getUsefulMineralTopElevation();

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_ROOF_ELEVATION,
        roundToOneDecimal(
            roofElevation));

    // 9 — общая мощность ПИ.
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_MINERAL_THICKNESS,
        roundToOneDecimal(
            indicators.getUsefulMineralThickness()));

    // 10 — мощность сухого ПИ.
    const double dryThickness =
        indicators.getDryMineralThickness();

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_DRY_MINERAL_THICKNESS,
        roundToOneDecimal(
            dryThickness));

    // 11 — мощность обводнённого ПИ.
    const double waterBearingThickness =
        indicators.getWaterBearingMineralThickness();

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_WATER_BEARING_MINERAL_THICKNESS,
        roundToOneDecimal(
            waterBearingThickness));

    // 12 — абсолютная отметка подошвы ПИ.
    const double bottomElevation =
        indicators.getUsefulMineralBottomElevation();

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_BOTTOM_ELEVATION,
        roundToOneDecimal(
            bottomElevation));

    // 13 — глубина залегания УГВ.
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_GROUNDWATER_DEPTH,
        roundToOneDecimal(
            indicators.getGroundwaterDepth()));

    // 14 — абсолютная отметка УГВ.
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_GROUNDWATER_ELEVATION,
        roundToOneDecimal(
            indicators.getGroundwaterElevation()));

    // 15 — мощность подстилающих пород.
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_UNDERLYING_THICKNESS,
        roundToOneDecimal(
            indicators.getUnderlyingRockThickness()));

    // 18 — наименование подстилающих пород.
    // Столбцы 16 и 17 пока намеренно не заполняются.
    ExcelWriteUtils::writeString(
        sheet,
        row,
        COL_UNDERLYING_NAME,
        indicators.getUnderlyingRockName());

    /*
     * =========================================================
     * 19-22 — абсолютные отметки частей ПИ
     * =========================================================
     *
     * 19 — кровля сухого ПИ
     * 20 — подошва сухого ПИ
     * 21 — кровля обводнённого ПИ
     * 22 — подошва обводнённого ПИ
     */

    // 17–20 — реальные абсолютные отметки частей ПИ.
    ExcelWriteUtils::writeDouble(
        sheet, row, COL_DRY_ROOF_ELEVATION,
        roundToOneDecimal(indicators.getDryMineralTopElevation()));

    ExcelWriteUtils::writeDouble(
        sheet, row, COL_DRY_BOTTOM_ELEVATION,
        roundToOneDecimal(indicators.getDryMineralBottomElevation()));

    ExcelWriteUtils::writeDouble(
        sheet, row, COL_WATER_BEARING_ROOF_ELEVATION,
        roundToOneDecimal(indicators.getWaterBearingMineralTopElevation()));

    ExcelWriteUtils::writeDouble(
        sheet, row, COL_WATER_BEARING_BOTTOM_ELEVATION,
        roundToOneDecimal(indicators.getWaterBearingMineralBottomElevation()));
}

void MainIndicatorsResultWriter::writeTotalsRow(
    OpenXLSX::XLWorksheet& sheet,
    uint32_t row,
    const std::vector<MainIndicators>& indicators,
    const std::vector<double>& prsThicknesses) const
{
    if (indicators.empty())
    {
        return;
    }

    ExcelWriteUtils::writeString(
        sheet,
        row,
        COL_SERIAL,
        "ИТОГО");

    double totalBoreholeDepth = 0.0;
    double totalCollarElevation = 0.0;
    double totalOverburden = 0.0;
    double totalPRSThickness = 0.0;
    double totalRoofElevation = 0.0;
    double totalMineralThickness = 0.0;
    double totalDryMineralThickness = 0.0;
    double totalWaterBearingMineralThickness = 0.0;
    double totalBottomElevation = 0.0;
    double totalGroundwaterDepth = 0.0;
    double totalGroundwaterElevation = 0.0;
    double totalUnderlyingThickness = 0.0;

    double totalDryRoofElevation = 0.0;
    double totalDryBottomElevation = 0.0;
    double totalWaterBearingRoofElevation = 0.0;
    double totalWaterBearingBottomElevation = 0.0;

    for (std::size_t i = 0;
         i < indicators.size();
         ++i)
    {
        const MainIndicators& value =
            indicators[i];

        totalBoreholeDepth +=
            value.getBoreholeDepth();

        totalCollarElevation +=
            value.getCollarElevation();

        totalOverburden +=
            value.getOverburdenThickness();

        if (i < prsThicknesses.size())
        {
            totalPRSThickness +=
                prsThicknesses[i];
        }

        const double roofElevation = value.getUsefulMineralTopElevation();
        const double bottomElevation = value.getUsefulMineralBottomElevation();
        const double dryRoofElevation = value.getDryMineralTopElevation();
        const double dryBottomElevation = value.getDryMineralBottomElevation();
        const double waterBearingRoofElevation = value.getWaterBearingMineralTopElevation();
        const double waterBearingBottomElevation = value.getWaterBearingMineralBottomElevation();

        totalRoofElevation +=
            roofElevation;

        totalMineralThickness +=
            value.getUsefulMineralThickness();

        totalDryMineralThickness +=
            value.getDryMineralThickness();

        totalWaterBearingMineralThickness +=
            value.getWaterBearingMineralThickness();

        totalBottomElevation +=
            bottomElevation;

        totalDryRoofElevation +=
            dryRoofElevation;

        totalDryBottomElevation +=
            dryBottomElevation;

        totalWaterBearingRoofElevation +=
            waterBearingRoofElevation;

        totalWaterBearingBottomElevation +=
            waterBearingBottomElevation;

        totalGroundwaterDepth +=
            value.getGroundwaterDepth();

        totalGroundwaterElevation +=
            value.getGroundwaterElevation();

        totalUnderlyingThickness +=
            value.getUnderlyingRockThickness();
    }

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_BOREHOLE_DEPTH,
        roundToOneDecimal(
            totalBoreholeDepth));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_COLLAR_ELEVATION,
        roundToOneDecimal(
            totalCollarElevation));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_OVERBURDEN,
        roundToOneDecimal(
            totalOverburden));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_PRS,
        roundToOneDecimal(
            totalPRSThickness));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_ROOF_ELEVATION,
        roundToOneDecimal(
            totalRoofElevation));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_MINERAL_THICKNESS,
        roundToOneDecimal(
            totalMineralThickness));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_DRY_MINERAL_THICKNESS,
        roundToOneDecimal(
            totalDryMineralThickness));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_WATER_BEARING_MINERAL_THICKNESS,
        roundToOneDecimal(
            totalWaterBearingMineralThickness));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_BOTTOM_ELEVATION,
        roundToOneDecimal(
            totalBottomElevation));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_GROUNDWATER_DEPTH,
        roundToOneDecimal(
            totalGroundwaterDepth));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_GROUNDWATER_ELEVATION,
        roundToOneDecimal(
            totalGroundwaterElevation));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_UNDERLYING_THICKNESS,
        roundToOneDecimal(
            totalUnderlyingThickness));

    // 17 — кровля сухого ПИ.
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_DRY_ROOF_ELEVATION,
        roundToOneDecimal(
            totalDryRoofElevation));

    // 18 — подошва сухого ПИ.
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_DRY_BOTTOM_ELEVATION,
        roundToOneDecimal(
            totalDryBottomElevation));

    // 19 — кровля обводнённого ПИ.
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_WATER_BEARING_ROOF_ELEVATION,
        roundToOneDecimal(
            totalWaterBearingRoofElevation));

    // 20 — подошва обводнённого ПИ.
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_WATER_BEARING_BOTTOM_ELEVATION,
        roundToOneDecimal(
            totalWaterBearingBottomElevation));
}

void MainIndicatorsResultWriter::writeAverageRow(
    OpenXLSX::XLWorksheet& sheet,
    uint32_t row,
    const std::vector<MainIndicators>& indicators,
    const std::vector<double>& prsThicknesses) const
{
    if (indicators.empty())
    {
        return;
    }

    ExcelWriteUtils::writeString(
        sheet,
        row,
        COL_SERIAL,
        "СРЕДНЕЕ");

    double sumBoreholeDepth = 0.0;
    double sumCollarElevation = 0.0;
    double sumOverburden = 0.0;
    double sumPRSThickness = 0.0;
    double sumRoofElevation = 0.0;
    double sumMineralThickness = 0.0;
    double sumDryMineralThickness = 0.0;
    double sumWaterBearingMineralThickness = 0.0;
    double sumBottomElevation = 0.0;
    double sumGroundwaterDepth = 0.0;
    double sumGroundwaterElevation = 0.0;
    double sumUnderlyingThickness = 0.0;

    double sumDryRoofElevation = 0.0;
    double sumDryBottomElevation = 0.0;
    double sumWaterBearingRoofElevation = 0.0;
    double sumWaterBearingBottomElevation = 0.0;

    std::size_t groundwaterCount = 0;

    for (std::size_t i = 0;
         i < indicators.size();
         ++i)
    {
        const MainIndicators& value =
            indicators[i];

        sumBoreholeDepth +=
            value.getBoreholeDepth();

        sumCollarElevation +=
            value.getCollarElevation();

        sumOverburden +=
            value.getOverburdenThickness();

        if (i < prsThicknesses.size())
        {
            sumPRSThickness +=
                prsThicknesses[i];
        }

        const double roofElevation = value.getUsefulMineralTopElevation();
        const double bottomElevation = value.getUsefulMineralBottomElevation();
        const double dryRoofElevation = value.getDryMineralTopElevation();
        const double dryBottomElevation = value.getDryMineralBottomElevation();
        const double waterBearingRoofElevation = value.getWaterBearingMineralTopElevation();
        const double waterBearingBottomElevation = value.getWaterBearingMineralBottomElevation();

        sumRoofElevation +=
            roofElevation;

        sumMineralThickness +=
            value.getUsefulMineralThickness();

        sumDryMineralThickness +=
            value.getDryMineralThickness();

        sumWaterBearingMineralThickness +=
            value.getWaterBearingMineralThickness();

        sumBottomElevation +=
            bottomElevation;

        sumDryRoofElevation +=
            dryRoofElevation;

        sumDryBottomElevation +=
            dryBottomElevation;

        sumWaterBearingRoofElevation +=
            waterBearingRoofElevation;

        sumWaterBearingBottomElevation +=
            waterBearingBottomElevation;

        if (value.getGroundwaterDepth() > 0.0)
        {
            sumGroundwaterDepth +=
                value.getGroundwaterDepth();

            sumGroundwaterElevation +=
                value.getGroundwaterElevation();

            ++groundwaterCount;
        }

        sumUnderlyingThickness +=
            value.getUnderlyingRockThickness();
    }

    const double count =
        static_cast<double>(indicators.size());

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_BOREHOLE_DEPTH,
        roundToOneDecimal(
            sumBoreholeDepth / count));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_COLLAR_ELEVATION,
        roundToOneDecimal(
            sumCollarElevation / count));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_OVERBURDEN,
        roundToOneDecimal(
            sumOverburden / count));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_PRS,
        roundToOneDecimal(
            sumPRSThickness / count));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_ROOF_ELEVATION,
        roundToOneDecimal(
            sumRoofElevation / count));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_MINERAL_THICKNESS,
        roundToOneDecimal(
            sumMineralThickness / count));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_DRY_MINERAL_THICKNESS,
        roundToOneDecimal(
            sumDryMineralThickness / count));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_WATER_BEARING_MINERAL_THICKNESS,
        roundToOneDecimal(
            sumWaterBearingMineralThickness / count));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_BOTTOM_ELEVATION,
        roundToOneDecimal(
            sumBottomElevation / count));

    /*
     * 17 — средняя отметка кровли сухого ПИ.
     */
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_DRY_ROOF_ELEVATION,
        roundToOneDecimal(
            sumDryRoofElevation / count));

    /*
     * 18 — средняя отметка подошвы сухого ПИ.
     */
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_DRY_BOTTOM_ELEVATION,
        roundToOneDecimal(
            sumDryBottomElevation / count));

    /*
     * 19 — средняя отметка кровли обводнённого ПИ.
     */
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_WATER_BEARING_ROOF_ELEVATION,
        roundToOneDecimal(
            sumWaterBearingRoofElevation / count));

    /*
     * 20 — средняя отметка подошвы обводнённого ПИ.
     */
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_WATER_BEARING_BOTTOM_ELEVATION,
        roundToOneDecimal(
            sumWaterBearingBottomElevation / count));

    if (groundwaterCount > 0)
    {
        const double groundwaterCountDouble =
            static_cast<double>(groundwaterCount);

        ExcelWriteUtils::writeDouble(
            sheet,
            row,
            COL_GROUNDWATER_DEPTH,
            roundToOneDecimal(
                sumGroundwaterDepth /
                groundwaterCountDouble));

        ExcelWriteUtils::writeDouble(
            sheet,
            row,
            COL_GROUNDWATER_ELEVATION,
            roundToOneDecimal(
                sumGroundwaterElevation /
                groundwaterCountDouble));
    }
    else
    {
        ExcelWriteUtils::writeDouble(
            sheet,
            row,
            COL_GROUNDWATER_DEPTH,
            0.0);

        ExcelWriteUtils::writeDouble(
            sheet,
            row,
            COL_GROUNDWATER_ELEVATION,
            0.0);
    }

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_UNDERLYING_THICKNESS,
        roundToOneDecimal(
            sumUnderlyingThickness / count));
}


namespace
{
    using ValueGetter = double (*)(const MainIndicators&);

    double v4(const MainIndicators& v){return v.getBoreholeDepth();}
    double v5(const MainIndicators& v){return v.getCollarElevation();}
    double v6(const MainIndicators& v){return v.getOverburdenThickness();}
    double v8(const MainIndicators& v){return v.getUsefulMineralTopElevation();}
    double v9(const MainIndicators& v){return v.getUsefulMineralThickness();}
    double v10(const MainIndicators& v){return v.getDryMineralThickness();}
    double v11(const MainIndicators& v){return v.getWaterBearingMineralThickness();}
    double v12(const MainIndicators& v){return v.getUsefulMineralBottomElevation();}
    double v13(const MainIndicators& v){return v.getGroundwaterDepth();}
    double v14(const MainIndicators& v){return v.getGroundwaterElevation();}
    double v15(const MainIndicators& v){return v.getUnderlyingRockThickness();}
    double v19(const MainIndicators& v){return v.getDryMineralTopElevation();}
    double v20(const MainIndicators& v){return v.getDryMineralBottomElevation();}
    double v21(const MainIndicators& v){return v.getWaterBearingMineralTopElevation();}
    double v22(const MainIndicators& v){return v.getWaterBearingMineralBottomElevation();}

    const std::pair<int,ValueGetter> SUMMARY_COLUMNS[] = {
        {4,v4},{5,v5},{6,v6},{8,v8},{9,v9},{10,v10},{11,v11},
        {12,v12},{13,v13},{14,v14},{15,v15},{19,v19},{20,v20},{21,v21},{22,v22}
    };

    void writeMinMaxRow(
        OpenXLSX::XLWorksheet& sheet,
        uint32_t row,
        const std::vector<MainIndicators>& values,
        const std::vector<double>& prs,
        bool maximum)
    {
        ExcelWriteUtils::writeString(sheet, row, 1, maximum ? "МАКСИМАЛЬНОЕ" : "МИНИМАЛЬНОЕ");

        bool foundPrs = false;
        double selectedPrs = maximum ? -std::numeric_limits<double>::infinity()
                                     : std::numeric_limits<double>::infinity();
        for (double v : prs)
        {
            if (!std::isfinite(v) || std::abs(v) <= 1e-9) continue;
            if ((!maximum && v < selectedPrs) || (maximum && v > selectedPrs))
            { selectedPrs = v; foundPrs = true; }
        }
        if (foundPrs) ExcelWriteUtils::writeDouble(sheet,row,7,ExcelWriteUtils::roundToTwoDecimals(selectedPrs));

        for (const auto& [column,getter] : SUMMARY_COLUMNS)
        {
            double selected = maximum ? -std::numeric_limits<double>::infinity()
                                      : std::numeric_limits<double>::infinity();
            bool found = false;
            for (const auto& value : values)
            {
                const double v = getter(value);
                if (!std::isfinite(v) || std::abs(v) <= 1e-9) continue;
                if ((!maximum && v < selected) || (maximum && v > selected))
                {
                    selected = v;
                    found = true;
                }
            }
            if (found)
                ExcelWriteUtils::writeDouble(sheet,row,column,ExcelWriteUtils::roundToTwoDecimals(selected));
        }
    }

    void writeMissingRow(
        OpenXLSX::XLWorksheet& sheet,
        uint32_t row,
        const std::vector<MainIndicators>& values,
        const std::vector<double>& prs)
    {
        ExcelWriteUtils::writeString(sheet,row,1,"Значение отсутствует");
        std::ostringstream prsNumbers;
        bool firstPrs = true;
        for (std::size_t i = 0; i < values.size() && i < prs.size(); ++i)
        {
            if (std::abs(prs[i]) <= 1e-9)
            {
                if (!firstPrs) prsNumbers << ", ";
                prsNumbers << values[i].getBoreholeNumber();
                firstPrs = false;
            }
        }
        if (!firstPrs) ExcelWriteUtils::writeString(sheet,row,7,prsNumbers.str());

        for (const auto& [column,getter] : SUMMARY_COLUMNS)
        {
            std::ostringstream numbers;
            bool first=true;
            for (const auto& value : values)
            {
                if (std::abs(getter(value)) <= 1e-9)
                {
                    if (!first) numbers << ", ";
                    numbers << value.getBoreholeNumber();
                    first=false;
                }
            }
            if (!first)
                ExcelWriteUtils::writeString(sheet,row,column,numbers.str());
        }
    }
}

void MainIndicatorsResultWriter::write(
    OpenXLSX::XLWorksheet& sheet,
    const Deposit& deposit,
    const std::vector<BoreholeSectionAnalysis>& analyses,
    const std::vector<MainIndicators>& indicators) const
{
    uint32_t row = START_ROW;
    int serialNumber = 1;

    std::vector<double> prsThicknesses;
    prsThicknesses.reserve(indicators.size());

    for (const MainIndicators& result : indicators)
    {
        const BoreholeSectionAnalysis* analysis =
            findAnalysis(
                analyses,
                result.getBoreholeNumber());

        if (analysis == nullptr)
        {
            continue;
        }

        const std::string blockNames =
            getBlockNames(
                deposit,
                result.getBoreholeNumber());

        const Borehole* borehole =
            deposit.findBorehole(result.getBoreholeNumber());

        if (borehole == nullptr)
            continue;

        const double prsThickness =
            calculatePRSThickness(*borehole);

        writeRow(
            sheet,
            row,
            serialNumber,
            blockNames,
            result,
            prsThickness);

        prsThicknesses.push_back(
            prsThickness);

        ++row;
        ++serialNumber;
    }

    if (prsThicknesses.empty())
    {
        return;
    }

    writeTotalsRow(
        sheet,
        row,
        indicators,
        prsThicknesses);

    ++row;

    writeAverageRow(
        sheet,
        row,
        indicators,
        prsThicknesses);

    ++row;
    writeMinMaxRow(sheet, row, indicators, prsThicknesses, false);
    ++row;
    writeMinMaxRow(sheet, row, indicators, prsThicknesses, true);
    ++row;
    writeMissingRow(sheet, row, indicators, prsThicknesses);

    // Одна пустая строка перед сводкой разведки 2026 года.
    ++row;
    ++row;

    std::vector<MainIndicators> year2026;
    std::vector<double> year2026Prs;
    for (std::size_t i = 0; i < indicators.size(); ++i)
    {
        const Borehole* borehole = deposit.findBorehole(indicators[i].getBoreholeNumber());
        if (borehole && borehole->getDrillingYear() == 2026)
        {
            year2026.push_back(indicators[i]);
            if (i < prsThicknesses.size()) year2026Prs.push_back(prsThicknesses[i]);
        }
    }

    if (!year2026.empty())
    {
        ExcelWriteUtils::writeString(sheet,row,1,"РАЗВЕДКА 2026 ГОДА");
        ++row;
        writeMinMaxRow(sheet,row,year2026,year2026Prs,false);
        ++row;
        writeMinMaxRow(sheet,row,year2026,year2026Prs,true);
        ++row;
        writeAverageRow(sheet,row,year2026,year2026Prs);
        ++row;
        writeTotalsRow(sheet,row,year2026,year2026Prs);

        ExcelWriteUtils::writeString(sheet,row,1,"СУММА");
    }

    // Отдельные итоги по группам запасов. Скважина входит в группу,
    // если она включена хотя бы в один блок с соответствующей категорией.
    struct ReserveGroup
    {
        std::string title;
        std::vector<std::string> categories;
    };

    const ReserveGroup reserveGroups[] =
    {
        {"СВОДКА ПРОМЫШЛЕННЫХ ЗАПАСОВ (A + B + C1)", {"A", "B", "C1"}},
        {"СВОДКА ЗАБАЛАНСОВЫХ ЗАПАСОВ (C2)", {"C2"}},
        {"СВОДКА ПРОГНОЗНЫХ РЕСУРСОВ (P1 + P2)", {"P1", "P2"}}
    };

    for (const ReserveGroup& group : reserveGroups)
    {
        std::set<std::string> groupBoreholes;
        for (const Block& block : deposit.getBlocks())
        {
            const auto designation = BlockDesignation::parse(block.getName());
            if (std::find(group.categories.begin(), group.categories.end(),
                          designation.second) == group.categories.end())
                continue;

            groupBoreholes.insert(
                block.getBoreholeNumbers().begin(),
                block.getBoreholeNumbers().end());
        }

        std::vector<MainIndicators> groupIndicators;
        std::vector<double> groupPrs;
        for (const MainIndicators& indicator : indicators)
        {
            if (!groupBoreholes.contains(indicator.getBoreholeNumber()))
                continue;

            const Borehole* borehole =
                deposit.findBorehole(indicator.getBoreholeNumber());
            if (borehole == nullptr)
                continue;

            groupIndicators.push_back(indicator);
            groupPrs.push_back(calculatePRSThickness(*borehole));
        }

        ++row;
        ExcelWriteUtils::writeString(sheet, row, 1, group.title);
        ++row;

        if (groupIndicators.empty())
        {
            ExcelWriteUtils::writeString(
                sheet, row, 1, "Нет скважин в этой группе категорий");
            continue;
        }

        writeMinMaxRow(sheet, row, groupIndicators, groupPrs, false);
        ++row;
        writeMinMaxRow(sheet, row, groupIndicators, groupPrs, true);
        ++row;
        writeAverageRow(sheet, row, groupIndicators, groupPrs);
        ++row;
        writeTotalsRow(sheet, row, groupIndicators, groupPrs);
        ExcelWriteUtils::writeString(sheet, row, 1, "СУММА");
    }
}