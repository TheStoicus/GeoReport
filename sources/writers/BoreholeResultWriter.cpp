#include "../../headers/writers/BoreholeResultWriter.h"

#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/calculations/WeightedAverageCalculator.h"

#include <string>
#include <algorithm>

using namespace OpenXLSX;

namespace
{
    bool isMonolithSample(const Sample& sample)
    {
        const std::string& number = sample.getSampleNumber();

        // Номер монолита начинается с кириллической буквы "м" / "М".
        // В UTF-8 это два байта. Проверяем именно начало строки.
        return number.rfind("м", 0) == 0 ||
               number.rfind("М", 0) == 0;
    }
}

void BoreholeResultWriter::write(
    XLWorksheet& sheet,
    const Borehole& borehole,
    const BoreholeSectionAnalysis& analysis,
    const BoreholeGeology& geology,
    uint32_t& row,
    int& serialNumber) const
{
    // ОБРАЗЦЫ
    //
    // В результирующую ведомость не записываем:
    //   1. монолиты;
    //   2. специальные АГ-пробы.
    //
    // Эти образцы остаются в исходных данных,
    // но не относятся к результирующей таблице
    // и не участвуют в расчетах.

    for (const Sample& sample : borehole.getSamples())
    {
        // Монолиты полностью исключаются.
        if (isMonolith(sample))
            continue;

        // АГ-пробы полностью исключаются.
        if (sample.isAgSample())
            continue;

        /*
         * На листах со средневзвешенными показателями должны
         * присутствовать только образцы, которые действительно
         * относятся к полезному ископаемому.
         *
         * Используем именно фактически учитываемое ПИ из BoreholeGeology:
         * сухое ПИ + разрешённое обводнённое ПИ.
         * Поэтому часть, превращённая в подстилающую из-за условий
         * разработки, сюда не попадает.
         */
        const double usefulThickness =
            countedUsefulOverlapThickness(sample, geology);

        /*
         * В таблицу по пробам записываем только те фрагменты,
         * которые целиком относятся к полезному ископаемому.
         *
         * Это особенно важно после разделения исходной пробы:
         * если исходная проба 4.0-6.0 м была разделена на
         *
         *     4.0-5.0 м -> вскрыша
         *     5.0-6.0 м -> ПИ
         *
         * то в таблицу ПИ попадёт только второй фрагмент.
         *
         * При этом сам WeightedAverageCalculator по-прежнему
         * умеет учитывать только полезную часть смешанного интервала,
         * что сохраняет правильность расчёта до/после разделения.
         */
        if (usefulThickness <= 1e-9 ||
            std::abs(usefulThickness - sample.getThickness()) > 1e-9)
        {
            continue;
        }

        writeSample(
            sheet,
            borehole,
            sample,
            row,
            serialNumber);

        ++row;
        ++serialNumber;
    }

    // ИТОГО
    WeightedAverageCalculator calculator;

    const BoreholeResult result =
        calculator.calculateBoreholeResult(
            borehole,
            analysis,
            geology);

    ExcelWriteUtils::writeString(
        sheet,
        row,
        1,
        "ИТОГО:");

    // Лист 4:
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        6,
        result.getTotalThickness());

    ++row;

    // СРЕДНЕВЗВЕШЕННОЕ
    ExcelWriteUtils::writeString(
        sheet,
        row,
        1,
        "СРЕДНЕВЗВЕШЕННОЕ");

    writeWeightedAverage(
        sheet,
        row,
        result.getWeightedAverage());

    ++row;
}

void BoreholeResultWriter::writeSample(
    XLWorksheet& sheet,
    const Borehole& borehole,
    const Sample& sample,
    uint32_t row,
    int serialNumber) const
{
    // Лист 4:
    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        1,
        static_cast<double>(serialNumber)); // № п/п

    ExcelWriteUtils::writeString(
        sheet,
        row,
        2,
        borehole.getBoreholeNumber()); // № скважины

    ExcelWriteUtils::writeString(
        sheet,
        row,
        3,
        sample.getSampleNumber()); // № образца

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        4,
        sample.getStartDepth()); // начало

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        5,
        sample.getEndDepth()); // конец

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        6,
        sample.getThickness()); // мощность

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        7,
        getGOSTValue(
            sample,
            "Грансостав песка <5 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        8,
        getGOSTValue(
            sample,
            "Грансостав гравия >5 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        9,
        getGOSTValue(
            sample,
            "Грансостав гравия >70 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        10,
        getGOSTValue(
            sample,
            "Грансостав гравия 70-40 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        11,
        getGOSTValue(
            sample,
            "Грансостав гравия 40-20 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        12,
        getGOSTValue(
            sample,
            "Грансостав гравия 20-10 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        13,
        getGOSTValue(
            sample,
            "Грансостав гравия 10-5 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        14,
        getGOSTValue(
            sample,
            "Грансостав песка 2.5 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        15,
        getGOSTValue(
            sample,
            "Грансостав песка 1.25 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        16,
        getGOSTValue(
            sample,
            "Грансостав песка 0.63 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        17,
        getGOSTValue(
            sample,
            "Грансостав песка 0.315 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        18,
        getGOSTValue(
            sample,
            "Грансостав песка 0.16 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        19,
        getGOSTValue(
            sample,
            "Грансостав песка <0.16 мм"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        20,
        getGOSTValue(
            sample,
            "Глинистые в песке, %"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        21,
        getGOSTValue(
            sample,
            "Глинистые в гравии, %"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        22,
        getGOSTValue(
            sample,
            "Глинистые в породе, %"));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        23,
        getGOSTValue(
            sample,
            "Модуль крупности песка"));
}

void BoreholeResultWriter::writeWeightedAverage(
    XLWorksheet& sheet,
    uint32_t row,
    const WeightedAverageResult& result) const
{
    const auto& values =
        result.getValues();

    const auto getValue =
        [&values](const std::string& name)
        {
            const auto it =
                values.find(name);

            if (it == values.end())
                return 0.0;

            return it->second;
        };

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        7,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав песка <5 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        8,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав гравия >5 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        9,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав гравия >70 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        10,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав гравия 70-40 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        11,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав гравия 40-20 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        12,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав гравия 20-10 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        13,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав гравия 10-5 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        14,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав песка 2.5 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        15,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав песка 1.25 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        16,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав песка 0.63 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        17,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав песка 0.315 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        18,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав песка 0.16 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        19,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Грансостав песка <0.16 мм")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        20,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Глинистые в песке, %")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        21,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Глинистые в гравии, %")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        22,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Глинистые в породе, %")));

    ExcelWriteUtils::writeDouble(
        sheet,
        row,
        23,
        ExcelWriteUtils::roundToTwoDecimals(
            getValue("Модуль крупности песка")));
}

double BoreholeResultWriter::getGOSTValue(
    const Sample& sample,
    const std::string& parameterName)
{
    const ParameterValue* value =
        sample.findGOSTParameter(parameterName);

    if (value == nullptr)
        return 0.0;

    const auto numeric =
        getNumericValue(*value);

    if (!numeric.has_value())
        return 0.0;

    return *numeric;
}

double BoreholeResultWriter::countedUsefulOverlapThickness(
    const Sample& sample,
    const BoreholeGeology& geology)
{
    if (sample.getThickness() <= 1e-9 ||
        sample.isMonolith() ||
        sample.isAgSample())
    {
        return 0.0;
    }

    double result = 0.0;

    for (const DryMineralSection& section :
         geology.getUsefulMineral().getDrySections())
    {
        const double start =
            std::max(sample.getStartDepth(), section.getRoofDepth());
        const double end =
            std::min(sample.getEndDepth(), section.getBottomDepth());

        if (end > start + 1e-9)
            result += end - start;
    }

    for (const WaterBearingMineralSection& section :
         geology.getUsefulMineral().getWaterBearingSections())
    {
        const double start =
            std::max(sample.getStartDepth(), section.getRoofDepth());
        const double end =
            std::min(sample.getEndDepth(), section.getBottomDepth());

        if (end > start + 1e-9)
            result += end - start;
    }

    return result;
}

bool BoreholeResultWriter::isMonolith(
    const Sample& sample)
{
    return isMonolithSample(sample);
}