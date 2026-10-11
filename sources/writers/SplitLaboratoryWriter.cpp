#include "../../headers/writers/SplitLaboratoryWriter.h"
#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/utils/ParameterValue.h"
#include "../../headers/normative/NormativeAnalysis.h"

#include <OpenXLSX.hpp>
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    constexpr double EPS = 1e-9;

    void writeParameter(
        OpenXLSX::XLWorksheet& sheet,
        unsigned row,
        int column,
        const ParameterValue& value)
    {
        if (const auto numeric = getNumericValue(value))
            ExcelWriteUtils::writeDouble(sheet, row, column, *numeric);
        else if (isText(value))
            ExcelWriteUtils::writeString(sheet, row, column, getTextValue(value));
    }

    const BoreholeAnalysisResult* findResult(
        const GeologicalAnalysisResult& result,
        const std::string& number)
    {
        for (const auto& item : result.getBoreholes())
            if (item.getBoreholeNumber() == number)
                return &item;
        return nullptr;
    }

    double overlap(double a1, double a2, double b1, double b2)
    {
        const double left = std::max(a1, b1);
        const double right = std::min(a2, b2);
        return right > left + EPS ? right - left : 0.0;
    }

    std::string samplePosition(
        const Sample& sample,
        const BoreholeAnalysisResult* result)
    {
        if (!result)
            return {};

        bool dry = false;
        bool water = false;
        bool overburden = false;
        bool underlying = false;

        const auto& geology = result->getGeology();

        for (const auto& section : geology.getUsefulMineral().getDrySections())
            dry |= overlap(sample.getStartDepth(), sample.getEndDepth(),
                           section.getRoofDepth(), section.getBottomDepth()) > EPS;

        for (const auto& section : geology.getUsefulMineral().getWaterBearingSections())
            water |= overlap(sample.getStartDepth(), sample.getEndDepth(),
                             section.getRoofDepth(), section.getBottomDepth()) > EPS;

        for (const auto& section : geology.getOverburden())
            overburden |= overlap(sample.getStartDepth(), sample.getEndDepth(),
                                  section.getRoofDepth(), section.getBottomDepth()) > EPS;

        for (const auto& section : geology.getUnderlyingRock())
            underlying |= overlap(sample.getStartDepth(), sample.getEndDepth(),
                                  section.getRoofDepth(), section.getBottomDepth()) > EPS;

        if (dry && water) return "Сухое ПИ / Обводненное ПИ";
        if (dry) return "Сухое ПИ";
        if (water) return "Обводненное ПИ";
        if (overburden) return "Вскрыша";
        if (underlying) return "Подстилающие";
        return {};
    }

    std::string tkp682Type(
        const Sample& sample,
        const GeologicalAnalysisResult& result)
    {
        const auto analysis = NormativeAnalysis::analyze(sample);
        if (analysis.tkp682FrostHeave == "Нет данных")
            return {};
        return analysis.tkp682FrostHeave;
    }

    std::string labSampleNumber(const Sample& sample)
    {
        if (sample.isMonolith())
            return "М" + sample.getSampleNumber();
        return sample.getSampleNumber();
    }
}

void SplitLaboratoryWriter::write(
    OpenXLSX::XLDocument& document,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult) const
{
    auto gostSheet = document.workbook().worksheet(11);
    auto tkpSheet = document.workbook().worksheet(12);
    writeGost(gostSheet, deposit, analysisResult);
    writeTkp(tkpSheet, deposit, analysisResult);
}

void SplitLaboratoryWriter::writeGost(
    OpenXLSX::XLWorksheet& sheet,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult) const
{
    static const char* names[] =
    {
        "Грансостав гравия >5 мм", "Грансостав песка <5 мм",
        "Грансостав гравия >70 мм", "Грансостав гравия 70-40 мм",
        "Грансостав гравия 40-20 мм", "Грансостав гравия 20-10 мм",
        "Грансостав гравия 10-5 мм", "Грансостав песка 2.5 мм",
        "Грансостав песка 1.25 мм", "Грансостав песка 0.63 мм",
        "Грансостав песка 0.315 мм", "Грансостав песка 0.16 мм",
        "Грансостав песка <0.16 мм", "Глинистые в песке, %",
        "Глинистые в гравии, %", "Глинистые в породе, %",
        "Колориметрическая проба в песке", "Колориметрическая проба в гравии",
        "Модуль крупности песка"
    };

    unsigned row = 6;

    for (const auto& borehole : deposit.getBoreholes())
    {
        const auto* result = findResult(analysisResult, borehole.getBoreholeNumber());

        for (const auto& sample : borehole.getSamples())
        {
            // Монолиты в ГОСТ-лабораторную ведомость не записываем.
            if (sample.isMonolith())
                continue;

            ExcelWriteUtils::writeString(sheet, row, 1, borehole.getBoreholeNumber());
            ExcelWriteUtils::writeString(sheet, row, 2, labSampleNumber(sample));
            ExcelWriteUtils::writeDouble(sheet, row, 3, sample.getStartDepth());
            ExcelWriteUtils::writeDouble(sheet, row, 4, sample.getEndDepth());

            for (int i = 0; i < 19; ++i)
            {
                if (const auto* value = sample.findGOSTParameter(names[i]))
                    writeParameter(sheet, row, 5 + i, *value);
            }

            ExcelWriteUtils::writeString(sheet, row, 24, sample.getGostSoilType());
            ExcelWriteUtils::writeString(sheet, row, 25, samplePosition(sample, result));
            ++row;
        }
    }
}

void SplitLaboratoryWriter::writeTkp(
    OpenXLSX::XLWorksheet& sheet,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult) const
{
    static const char* granulometry[] =
    {
        "ТКП >10 мм", "ТКП 10-5 мм", "ТКП 5-2 мм", "ТКП 2-1 мм",
        "ТКП 1-0.5 мм", "ТКП 0.5-0.25 мм", "ТКП 0.25-0.1 мм",
        "ТКП <0.1 мм", "ТКП <0.05 мм", "ТКП 0.05-0.01 мм",
        "ТКП 0.01-0.005 мм", "ТКП <0.005 мм", "ТКП >2 мм",
        "ТКП 2-0.25 мм", "ТКП 0.25-0.05 мм", "ТКП 0.05 мм"
    };

    static const char* physical[] =
    {
        "Естеств.влажность %", "Плотность грунта", "Плотность сухого грунта",
        "Плотность частиц", "Коэффициент пористости грунта", "Пористость,%",
        "Степень влажности", "Угол естественного откоса, сухой",
        "Угол естественного откоса, под водой", "Коэфф. фильтрации, м/сут",
        "Граница текучести", "Граница раскатки", "Число пластичности",
        "Показ. текучести", "Потери при прокаливании", "Оптимальная влажность",
        "Максимальная плотность", "Показатель неоднородности"
    };

    const int physicalColumns[] =
    {
        21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38
    };

    unsigned row = 8;

    for (const auto& borehole : deposit.getBoreholes())
    {
        const auto* result = findResult(analysisResult, borehole.getBoreholeNumber());

        for (const auto& sample : borehole.getSamples())
        {
            // Лист 12 содержит только образцы, для которых ТКП 682 действительно применим.
            if (tkp682Type(sample, analysisResult).empty())
                continue;

            ExcelWriteUtils::writeString(sheet, row, 1, borehole.getBoreholeNumber());
            ExcelWriteUtils::writeString(sheet, row, 2, labSampleNumber(sample));
            ExcelWriteUtils::writeDouble(sheet, row, 3, sample.getStartDepth());
            ExcelWriteUtils::writeDouble(sheet, row, 4, sample.getEndDepth());

            for (int i = 0; i < 16; ++i)
            {
                if (const auto* value = sample.findTKPParameter(granulometry[i]))
                    writeParameter(sheet, row, 5 + i, *value);
            }

            for (int i = 0; i < 18; ++i)
            {
                if (const auto* value = sample.findTKPParameter(physical[i]))
                    writeParameter(sheet, row, physicalColumns[i], *value);
            }

            ExcelWriteUtils::writeString(sheet, row, 39, sample.getTkpSoilType());
            ExcelWriteUtils::writeString(sheet, row, 40, samplePosition(sample, result));
            ++row;
        }
    }
}
