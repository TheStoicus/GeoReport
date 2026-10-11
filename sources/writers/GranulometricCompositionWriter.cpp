#include "../../headers/writers/GranulometricCompositionWriter.h"

#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/calculations/MineralCalculationPart.h"

#include <OpenXLSX.hpp>

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace OpenXLSX;

namespace
{
    const std::vector<std::string> PARAMETERS =
    {
        "Грансостав гравия >5 мм",
        "Грансостав гравия >70 мм",
        "Грансостав гравия 70-40 мм",
        "Грансостав гравия 40-20 мм",
        "Грансостав гравия 20-10 мм",
        "Грансостав гравия 10-5 мм",
        "Грансостав песка 2.5 мм",
        "Грансостав песка 1.25 мм",
        "Грансостав песка 0.63 мм",
        "Грансостав песка 0.315 мм",
        "Грансостав песка 0.16 мм",
        "Грансостав песка <0.16 мм",
        "Глинистые в песке, %",
        "Глинистые в гравии, %",
        "Глинистые в породе, %",
        "Модуль крупности песка"
    };
}

void GranulometricCompositionWriter::write(
    XLWorksheet& sheet,
    const Deposit& deposit,
    const std::vector<BoreholeSectionAnalysis>& analyses,
    const std::vector<BoreholeAnalysisResult>& geologyResults) const
{
    GranulometricCompositionCalculator calculator;

    uint32_t row = START_ROW;

    for (const Block& block : deposit.getBlocks())
    {
        const GranulometricCompositionBlockResult result =
            calculator.calculate(
                block,
                deposit,
                analyses,
                geologyResults,
                MineralCalculationPart::TotalUsefulMineral);

        row = writeBlock(
            sheet,
            row,
            result);

        const GranulometricCompositionBlockResult dryResult =
            calculator.calculate(
                block,
                deposit,
                analyses,
                geologyResults,
                MineralCalculationPart::DryUsefulMineral);

        const GranulometricCompositionBlockResult waterResult =
            calculator.calculate(
                block,
                deposit,
                analyses,
                geologyResults,
                MineralCalculationPart::WaterBearingUsefulMineral);

        /* Подблоки выводим только тогда, когда у блока действительно
         * присутствуют обе части ПИ. */
        if ((dryResult.sampleCount > 0 || dryResult.boreholeCount > 0) &&
            (waterResult.sampleCount > 0 || waterResult.boreholeCount > 0))
        {
            GranulometricCompositionBlockResult dryTitled = dryResult;
            dryTitled.blockName = block.getName() + " — сухое ПИ";
            row = writeBlock(sheet, row, dryTitled);

            GranulometricCompositionBlockResult waterTitled = waterResult;
            waterTitled.blockName = block.getName() + " — обводнённое ПИ";
            row = writeBlock(sheet, row, waterTitled);
        }
    }
}

uint32_t GranulometricCompositionWriter::writeBlock(
    XLWorksheet& sheet,
    uint32_t row,
    const GranulometricCompositionBlockResult& result) const
{
    /*
     * ---------------------------------------------------------
     * Название блока
     * ---------------------------------------------------------
     *
     * Объединяем A:P.
     */
    writeMergedRow(
        sheet,
        row,
        "Блок " + result.blockName);

    ++row;


    /*
     * ---------------------------------------------------------
     * По пробам
     * ---------------------------------------------------------
     *
     * Объединяем A:P.
     */
    writeMergedRow(
        sheet,
        row,
        "по пробам (" +
        std::to_string(result.sampleCount) +
        ")");

    ++row;


    /*
     * ---------------------------------------------------------
     * Диапазоны значений по пробам
     * ---------------------------------------------------------
     *
     * 16 значений записываются в A:P.
     */
    writeRangeRow(
        sheet,
        row,
        result.sampleRanges);

    ++row;


    /*
     * ---------------------------------------------------------
     * По пересечениям
     * ---------------------------------------------------------
     *
     * Объединяем A:P.
     */
    writeMergedRow(
        sheet,
        row,
        "по пересечениям (" +
        std::to_string(result.boreholeCount) +
        ")");

    ++row;


    /*
     * ---------------------------------------------------------
     * Диапазоны значений по пересечениям
     * ---------------------------------------------------------
     *
     * 16 значений записываются в A:P.
     */
    writeRangeRow(
        sheet,
        row,
        result.boreholeRanges);

    ++row;


    /*
     * ---------------------------------------------------------
     * По блоку
     * ---------------------------------------------------------
     *
     * Объединяем A:P.
     */
    writeMergedRow(
        sheet,
        row,
        "по блоку");

    ++row;


    /*
     * ---------------------------------------------------------
     * Средневзвешенные значения по блоку
     * ---------------------------------------------------------
     *
     * 16 значений записываются в A:P.
     */
    writeBlockAverageRow(
        sheet,
        row,
        result.blockWeightedAverage);

    ++row;

    return row;
}

void GranulometricCompositionWriter::writeRangeRow(
    XLWorksheet& sheet,
    uint32_t row,
    const std::vector<GranulometricParameterRange>& ranges) const
{
    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
        const std::string value =
            formatRange(ranges[i]);

        if (value.empty())
            continue;

        /*
         * Теперь первый параметр начинается
         * с первого столбца.
         *
         * i = 0 -> A
         * i = 1 -> B
         * ...
         * i = 15 -> P
         */
        const uint16_t column =
            static_cast<uint16_t>(
                COL_FIRST_PARAMETER + i);

        ExcelWriteUtils::writeString(
            sheet,
            row,
            column,
            value);
    }
}

void GranulometricCompositionWriter::writeBlockAverageRow(
    XLWorksheet& sheet,
    uint32_t row,
    const WeightedAverageResult& result) const
{
    const auto& values =
        result.getValues();

    for (std::size_t i = 0;
         i < PARAMETERS.size();
         ++i)
    {
        const auto it =
            values.find(PARAMETERS[i]);

        if (it == values.end())
            continue;

        /*
         * Первый параметр -> A,
         * последний -> P.
         */
        const uint16_t column =
            static_cast<uint16_t>(
                COL_FIRST_PARAMETER + i);

        ExcelWriteUtils::writeDouble(
            sheet,
            row,
            column,
            ExcelWriteUtils::roundToTwoDecimals(
                it->second));
    }
}

void GranulometricCompositionWriter::writeMergedRow(
    XLWorksheet& sheet,
    uint32_t row,
    const std::string& text)
{
    /*
     * Объединяем всю ширину третьей таблицы:
     *
     * A:P
     */
    const std::string range =
        "A" +
        std::to_string(row) +
        ":P" +
        std::to_string(row);

    sheet.mergeCells(range);

    /*
     * Текст записываем в левую верхнюю
     * ячейку объединенного диапазона — A.
     */
    ExcelWriteUtils::writeString(
        sheet,
        row,
        COL_LABEL,
        text);
}

std::string GranulometricCompositionWriter::formatRange(
    const GranulometricParameterRange& range)
{
    if (!range.hasValue)
        return {};

    std::ostringstream stream;

    stream << std::fixed
           << std::setprecision(2)
           << range.minimum
           << "-"
           << range.maximum;

    return stream.str();
}