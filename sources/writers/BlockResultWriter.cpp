#include "../../headers/writers/BlockResultWriter.h"

#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/calculations/WeightedAverageCalculator.h"
#include "../../headers/calculations/MineralCalculationPart.h"
#include "../../headers/normative/NormativeAnalysis.h"
#include "../../headers/writers/BlockDesignation.h"

#include <OpenXLSX.hpp>

#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include <cmath>

namespace
{
    const std::array<std::string, 16> GOST_PARAMETERS =
    {
        "Грансостав песка <5 мм",
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
        "Глинистые в породе, %"
    };

    const BoreholeSectionAnalysis* findAnalysis(
        const std::vector<BoreholeSectionAnalysis>& analyses,
        const std::string& boreholeNumber)
    {
        for (const BoreholeSectionAnalysis& analysis : analyses)
        {
            if (analysis.getBoreholeNumber() == boreholeNumber)
                return &analysis;
        }

        return nullptr;
    }

    const BoreholeGeology* findGeology(
        const std::vector<BoreholeAnalysisResult>& geologyResults,
        const std::string& boreholeNumber)
    {
        for (const BoreholeAnalysisResult& result : geologyResults)
        {
            if (result.getBoreholeNumber() == boreholeNumber)
                return &result.getGeology();
        }

        return nullptr;
    }

    const char* partName(MineralCalculationPart part)
    {
        switch (part)
        {
            case MineralCalculationPart::TotalUsefulMineral:
                return "";
            case MineralCalculationPart::DryUsefulMineral:
                return " — сухое полезное ископаемое";
            case MineralCalculationPart::WaterBearingUsefulMineral:
                return " — обводнённое полезное ископаемое";
        }

        return "";
    }

    std::string classifySandByFinenessModulus(double modulus)
    {
        if (!std::isfinite(modulus) || modulus < 0.0)
            return {};

        if (modulus <= 0.7) return "Очень тонкий";
        if (modulus <= 1.0) return "Тонкий";
        if (modulus <= 1.5) return "Очень мелкий";
        if (modulus <= 2.0) return "Мелкий";
        if (modulus <= 2.5) return "Средний";
        if (modulus <= 3.0) return "Крупный";
        if (modulus <= 3.5) return "Повышенной крупности";

        return {};
    }

    std::string check063(double modulus, double residue)
    {
        if (!std::isfinite(modulus) || !std::isfinite(residue))
            return {};

        constexpr double EPS = 1e-9;
        const std::string type = classifySandByFinenessModulus(modulus);

        if (type.empty())
        {
            if (modulus > 3.5 + EPS)
                return "Не соответствует (выше нормативного)";

            return "Не соответствует (ниже нормативного)";
        }

        double lower = 0.0;
        double upper = 0.0;
        bool normalized = true;

        if (type == "Повышенной крупности")
        {
            lower = 65.0; upper = 75.0;
        }
        else if (type == "Крупный")
        {
            lower = 45.0; upper = 65.0;
        }
        else if (type == "Средний")
        {
            lower = 30.0; upper = 45.0;
        }
        else if (type == "Мелкий")
        {
            lower = 10.0; upper = 30.0;
        }
        else if (type == "Очень мелкий")
        {
            upper = 10.0;
        }
        else
        {
            normalized = false;
        }

        if (!normalized)
            return {};

        if (type == "Очень мелкий")
            return residue <= upper + EPS
                       ? "Соответствует"
                       : "Не соответствует (выше нормативного)";

        if (residue <= lower + EPS)
            return "Не соответствует (ниже нормативного)";

        if (residue <= upper + EPS)
            return "Соответствует";

        return "Не соответствует (выше нормативного)";
    }

    std::string classifyTkp1704372012(double gravel5)
    {
        if (gravel5 >= 7.0 && gravel5 <= 15.0) return "Гравелистый песок";
        if (gravel5 > 15.0 && gravel5 <= 30.0) return "Гравийно-песчаная смесь";
        if (gravel5 > 30.0) return "Песчано-гравийная смесь";
        return "Песок";
    }

}

void BlockResultWriter::write(
    OpenXLSX::XLWorksheet& sheet,
    const Deposit& deposit,
    const std::vector<BoreholeSectionAnalysis>& analyses,
    const std::vector<BoreholeAnalysisResult>& geologyResults) const
{
    WeightedAverageCalculator calculator;

    uint32_t row = START_ROW;
    uint32_t serialNumber = 1;

    for (const Block& block : deposit.getBlocks())
    {
        /*
         * Сначала записываем основной блок — всё принятое к расчёту ПИ.
         */
        row = writeBlockResult(
            sheet,
            row,
            serialNumber,
            block,
            deposit,
            analyses,
            geologyResults,
            calculator,
            MineralCalculationPart::TotalUsefulMineral,
            "Блок " + BlockDesignation::format(block.getName()));

        /*
         * Затем, если в блоке присутствуют обе части ПИ,
         * создаём два расчётных подблока:
         *
         *   1. сухое ПИ;
         *   2. обводнённое ПИ.
         *
         * Подблоки являются производными исходного блока и не меняют
         * состав исходного Block в Deposit.
         */
        const bool hasDry = hasPartInBlock(
            block,
            deposit,
            analyses,
            geologyResults,
            calculator,
            MineralCalculationPart::DryUsefulMineral);

        const bool hasWater = hasPartInBlock(
            block,
            deposit,
            analyses,
            geologyResults,
            calculator,
            MineralCalculationPart::WaterBearingUsefulMineral);

        if (hasDry && hasWater)
        {
            row = writeBlockResult(
                sheet,
                row,
                serialNumber,
                block,
                deposit,
                analyses,
                geologyResults,
                calculator,
                MineralCalculationPart::DryUsefulMineral,
                "Подблок " + BlockDesignation::format(block.getName()) + " — сухое ПИ");

            row = writeBlockResult(
                sheet,
                row,
                serialNumber,
                block,
                deposit,
                analyses,
                geologyResults,
                calculator,
                MineralCalculationPart::WaterBearingUsefulMineral,
                "Подблок " + BlockDesignation::format(block.getName()) + " — обводнённое ПИ");
        }
    }
}

bool BlockResultWriter::hasPartInBlock(
    const Block& block,
    const Deposit& deposit,
    const std::vector<BoreholeSectionAnalysis>& analyses,
    const std::vector<BoreholeAnalysisResult>& geologyResults,
    const WeightedAverageCalculator& calculator,
    MineralCalculationPart part) const
{
    for (const std::string& boreholeNumber : block.getBoreholeNumbers())
    {
        const Borehole* borehole = deposit.findBorehole(boreholeNumber);
        if (borehole == nullptr)
            continue;

        const BoreholeSectionAnalysis* analysis =
            findAnalysis(analyses, boreholeNumber);
        if (analysis == nullptr || !analysis->isResolved())
            continue;

        const BoreholeGeology* geology =
            findGeology(geologyResults, boreholeNumber);
        if (geology == nullptr)
            continue;

        const BoreholeResult result = calculator.calculateBoreholeResult(
            *borehole,
            *analysis,
            *geology,
            part);

        if (result.getTotalThickness() > 1e-9)
            return true;
    }

    return false;
}

uint32_t BlockResultWriter::writeBlockResult(
    OpenXLSX::XLWorksheet& sheet,
    uint32_t row,
    uint32_t& serialNumber,
    const Block& block,
    const Deposit& deposit,
    const std::vector<BoreholeSectionAnalysis>& analyses,
    const std::vector<BoreholeAnalysisResult>& geologyResults,
    const WeightedAverageCalculator& calculator,
    MineralCalculationPart part,
    const std::string& title) const
{
    /*
     * Заголовок блока/подблока.
     */
    ExcelWriteUtils::writeString(
        sheet,
        row,
        COL_BOREHOLE_OR_BLOCK,
        title);

    ++row;

    std::vector<BoreholeResult> boreholeResults;
    double blockTotalThickness = 0.0;

    for (const std::string& boreholeNumber : block.getBoreholeNumbers())
    {
        const Borehole* borehole = deposit.findBorehole(boreholeNumber);
        if (borehole == nullptr)
            continue;

        const BoreholeSectionAnalysis* analysis =
            findAnalysis(analyses, boreholeNumber);
        if (analysis == nullptr || !analysis->isResolved())
            continue;

        const BoreholeGeology* geology =
            findGeology(geologyResults, boreholeNumber);
        if (geology == nullptr)
            continue;

        const BoreholeResult boreholeResult =
            calculator.calculateBoreholeResult(
                *borehole,
                *analysis,
                *geology,
                part);

        if (boreholeResult.getTotalThickness() <= 1e-9)
            continue;

        boreholeResults.push_back(boreholeResult);
        blockTotalThickness += boreholeResult.getTotalThickness();

        ExcelWriteUtils::writeDouble(
            sheet,
            row,
            COL_SERIAL,
            static_cast<double>(serialNumber));

        ExcelWriteUtils::writeString(
            sheet,
            row,
            COL_BOREHOLE_OR_BLOCK,
            borehole->getBoreholeNumber());

        /*
         * Диапазон глубин для строки скважины определяется только
         * по образцам, имеющим пересечение именно с выбранной частью ПИ.
         *
         * Для подблока сухого ПИ сюда попадут только сухие интервалы,
         * для подблока обводнённого ПИ — только обводнённые интервалы.
         */
        double startDepth = 0.0;
        double endDepth = 0.0;
        bool firstSample = true;

        for (const Sample& sample : borehole->getSamples())
        {
            if (sample.isMonolith() || sample.isAgSample())
                continue;

            double countedThickness = 0.0;

            if (part == MineralCalculationPart::DryUsefulMineral)
            {
                for (const DryMineralSection& section :
                     geology->getUsefulMineral().getDrySections())
                {
                    countedThickness += overlap(
                        sample.getStartDepth(),
                        sample.getEndDepth(),
                        section.getRoofDepth(),
                        section.getBottomDepth());
                }
            }
            else if (part == MineralCalculationPart::WaterBearingUsefulMineral)
            {
                for (const WaterBearingMineralSection& section :
                     geology->getUsefulMineral().getWaterBearingSections())
                {
                    countedThickness += overlap(
                        sample.getStartDepth(),
                        sample.getEndDepth(),
                        section.getRoofDepth(),
                        section.getBottomDepth());
                }
            }
            else
            {
                for (const DryMineralSection& section :
                     geology->getUsefulMineral().getDrySections())
                {
                    countedThickness += overlap(
                        sample.getStartDepth(),
                        sample.getEndDepth(),
                        section.getRoofDepth(),
                        section.getBottomDepth());
                }

                for (const WaterBearingMineralSection& section :
                     geology->getUsefulMineral().getWaterBearingSections())
                {
                    countedThickness += overlap(
                        sample.getStartDepth(),
                        sample.getEndDepth(),
                        section.getRoofDepth(),
                        section.getBottomDepth());
                }
            }

            if (countedThickness <= 1e-9)
                continue;

            if (firstSample)
            {
                startDepth = sample.getStartDepth();
                endDepth = sample.getEndDepth();
                firstSample = false;
            }
            else
            {
                startDepth = std::min(startDepth, sample.getStartDepth());
                endDepth = std::max(endDepth, sample.getEndDepth());
            }
        }

        ExcelWriteUtils::writeDouble(
            sheet,
            row,
            COL_START_DEPTH,
            ExcelWriteUtils::roundToTwoDecimals(startDepth));

        ExcelWriteUtils::writeDouble(
            sheet,
            row,
            COL_END_DEPTH,
            ExcelWriteUtils::roundToTwoDecimals(endDepth));

        ExcelWriteUtils::writeDouble(
            sheet,
            row,
            COL_THICKNESS,
            ExcelWriteUtils::roundToTwoDecimals(
                boreholeResult.getTotalThickness()));

        const auto& values =
            boreholeResult.getWeightedAverage().getValues();

        for (std::size_t i = 0; i < GOST_PARAMETERS.size(); ++i)
        {
            const auto it = values.find(GOST_PARAMETERS[i]);
            if (it == values.end())
                continue;

            const uint16_t column =
                static_cast<uint16_t>(COL_FIRST_PARAMETER + i);

            ExcelWriteUtils::writeDouble(
                sheet,
                row,
                column,
                ExcelWriteUtils::roundToTwoDecimals(it->second));
        }

        const auto modulusIt =
            values.find("Модуль крупности песка");
        const auto residueIt =
            values.find("Грансостав песка 0.63 мм");

        // 22 — модуль крупности песка (числовое значение).
        if (modulusIt != values.end())
        {
            ExcelWriteUtils::writeDouble(
                sheet, row, 22,
                ExcelWriteUtils::roundToTwoDecimals(
                    modulusIt->second));
        }

        if (modulusIt != values.end())
        {
            const std::string type =
                classifySandByFinenessModulus(modulusIt->second);

            if (!type.empty())
            {
                ExcelWriteUtils::writeString(
                    sheet, row, 23, type);
            }

            if (residueIt != values.end())
            {
                const std::string conformity =
                    check063(modulusIt->second, residueIt->second);

                if (!conformity.empty())
                {
                    ExcelWriteUtils::writeString(
                        sheet, row, 24, conformity);
                }
            }

            const auto gravel5It = values.find("Грансостав гравия >5 мм");
            const auto less016It = values.find("Грансостав песка <0.16 мм");
            const auto clayIt = values.find("Глинистые в породе, %");
            if (gravel5It != values.end())
                ExcelWriteUtils::writeString(sheet, row, 25, classifyTkp1704372012(gravel5It->second));
            if (modulusIt != values.end() && residueIt != values.end() &&
                less016It != values.end() && clayIt != values.end())
            {
                const auto remainder10It = values.find("Грансостав гравия >70 мм");
                const auto from70It = values.find("Грансостав гравия 70-40 мм");
                const auto from40It = values.find("Грансостав гравия 40-20 мм");
                const auto from20It = values.find("Грансостав гравия 20-10 мм");
                const auto remainder5It = values.find("Грансостав гравия >5 мм");
                if (remainder10It != values.end() && from70It != values.end() &&
                    from40It != values.end() && from20It != values.end() && remainder5It != values.end())
                {
                    const double remainder10 = remainder10It->second + from70It->second + from40It->second + from20It->second;
                    ExcelWriteUtils::writeString(sheet, row, 26,
                        NormativeAnalysis::gost8736ClassFromValues(
                            modulusIt->second, residueIt->second, remainder10,
                            remainder5It->second, less016It->second, clayIt->second));
                }
            }
        }

        ++row;
        ++serialNumber;
    }

    if (boreholeResults.empty())
        return row;

    /* Итого по блоку/подблоку. */
    ExcelWriteUtils::writeString(
        sheet,
        row,
        COL_BOREHOLE_OR_BLOCK,
        "ИТОГО");

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        COL_THICKNESS,
        ExcelWriteUtils::roundToTwoDecimals(blockTotalThickness));

    ++row;

    /* Средневзвешенное по блоку/подблоку. */
    const WeightedAverageResult blockResult =
        calculator.calculateBlock(boreholeResults);

    ExcelWriteUtils::writeString(
        sheet,
        row,
        COL_BOREHOLE_OR_BLOCK,
        "СРЕДНЕВЗВЕШЕННОЕ");

    const auto& blockValues = blockResult.getValues();

    for (std::size_t i = 0; i < GOST_PARAMETERS.size(); ++i)
    {
        const auto it = blockValues.find(GOST_PARAMETERS[i]);
        if (it == blockValues.end())
            continue;

        const uint16_t column =
            static_cast<uint16_t>(COL_FIRST_PARAMETER + i);

        ExcelWriteUtils::writeDouble(
            sheet,
            row,
            column,
            ExcelWriteUtils::roundToTwoDecimals(it->second));
    }

    const auto modulusIt =
        blockValues.find("Модуль крупности песка");
    const auto residueIt =
        blockValues.find("Грансостав песка 0.63 мм");

    // 22 — модуль крупности песка (числовое значение).
    if (modulusIt != blockValues.end())
    {
        ExcelWriteUtils::writeDouble(
            sheet, row, 22,
            ExcelWriteUtils::roundToTwoDecimals(
                modulusIt->second));
    }

    if (modulusIt != blockValues.end())
    {
        const std::string type =
            classifySandByFinenessModulus(modulusIt->second);

        if (!type.empty())
        {
            ExcelWriteUtils::writeString(
                sheet, row, 23, type);
        }

        if (residueIt != blockValues.end())
        {
            const std::string conformity =
                check063(modulusIt->second, residueIt->second);

            if (!conformity.empty())
            {
                ExcelWriteUtils::writeString(
                    sheet, row, 24, conformity);
            }
        }

        const auto gravel5It = blockValues.find("Грансостав гравия >5 мм");
        const auto less016It = blockValues.find("Грансостав песка <0.16 мм");
        const auto clayIt = blockValues.find("Глинистые в породе, %");
        if (gravel5It != blockValues.end())
            ExcelWriteUtils::writeString(sheet, row, 25, classifyTkp1704372012(gravel5It->second));
        if (modulusIt != blockValues.end() && residueIt != blockValues.end() &&
            less016It != blockValues.end() && clayIt != blockValues.end())
        {
            const auto remainder10It = blockValues.find("Грансостав гравия >70 мм");
            const auto from70It = blockValues.find("Грансостав гравия 70-40 мм");
            const auto from40It = blockValues.find("Грансостав гравия 40-20 мм");
            const auto from20It = blockValues.find("Грансостав гравия 20-10 мм");
            const auto remainder5It = blockValues.find("Грансостав гравия >5 мм");
            if (remainder10It != blockValues.end() && from70It != blockValues.end() &&
                from40It != blockValues.end() && from20It != blockValues.end() && remainder5It != blockValues.end())
            {
                const double remainder10 = remainder10It->second + from70It->second + from40It->second + from20It->second;
                ExcelWriteUtils::writeString(sheet, row, 26,
                    NormativeAnalysis::gost8736ClassFromValues(
                        modulusIt->second, residueIt->second, remainder10,
                        remainder5It->second, less016It->second, clayIt->second));
            }
        }
    }

    ++row;

    return row;
}

double BlockResultWriter::overlap(
    double sampleStart,
    double sampleEnd,
    double sectionStart,
    double sectionEnd)
{
    const double start = std::max(sampleStart, sectionStart);
    const double end = std::min(sampleEnd, sectionEnd);

    return end > start + 1e-9 ? end - start : 0.0;
}
