#include "../../headers/writers/AnaliticsWriter.h"
#include "../../headers/excel/ExcelWriteUtils.h"
#include "../../headers/normative/NormativeAnalysis.h"
#include "../../headers/writers/BlockDesignation.h"

#include <OpenXLSX.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include "../../headers/utils/ParameterValue.h"

namespace
{
    constexpr double EPS = 1e-9;

    double overlap(
        double firstStart,
        double firstEnd,
        double secondStart,
        double secondEnd)
    {
        const double left = std::max(firstStart, secondStart);
        const double right = std::min(firstEnd, secondEnd);

        return right > left + EPS ? right - left : 0.0;
    }

    const BoreholeAnalysisResult* findResult(
        const GeologicalAnalysisResult& result,
        const std::string& boreholeNumber)
    {
        for (const auto& boreholeResult : result.getBoreholes())
        {
            if (boreholeResult.getBoreholeNumber() == boreholeNumber)
                return &boreholeResult;
        }

        return nullptr;
    }
}

void AnaliticsWriter::write(
    OpenXLSX::XLWorksheet& sheet,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult) const
{
    unsigned int row = START_ROW;
    unsigned int serialNumber = 1;

    for (const auto& borehole : deposit.getBoreholes())
    {
        const auto* boreholeResult = findResult(
            analysisResult,
            borehole.getBoreholeNumber());

        if (!boreholeResult ||
            !boreholeResult->getSectionAnalysis().isResolved())
        {
            continue;
        }

        const auto& geology = boreholeResult->getGeology();

        for (const auto& block : deposit.getBlocks())
        {
            bool belongsToBlock = false;

            for (const auto& number : block.getBoreholeNumbers())
            {
                if (number == borehole.getBoreholeNumber())
                {
                    belongsToBlock = true;
                    break;
                }
            }

            if (!belongsToBlock)
                continue;

            for (const auto& sample : borehole.getSamples())
            {
                if (sample.isMonolith() ||
                    sample.isAgSample() ||
                    sample.getThickness() <= EPS)
                {
                    continue;
                }

                bool intersectsDryMineral = false;
                bool intersectsWaterBearingMineral = false;

                for (const auto& section :
                     geology.getUsefulMineral().getDrySections())
                {
                    if (overlap(
                            sample.getStartDepth(),
                            sample.getEndDepth(),
                            section.getRoofDepth(),
                            section.getBottomDepth()) > EPS)
                    {
                        intersectsDryMineral = true;
                        break;
                    }
                }

                for (const auto& section :
                     geology.getUsefulMineral().getWaterBearingSections())
                {
                    if (overlap(
                            sample.getStartDepth(),
                            sample.getEndDepth(),
                            section.getRoofDepth(),
                            section.getBottomDepth()) > EPS)
                    {
                        intersectsWaterBearingMineral = true;
                        break;
                    }
                }

                const auto writeRow = [&](int subblockNumber)
                {
                    const auto normative =
                        NormativeAnalysis::analyze(sample);

                    ExcelWriteUtils::writeDouble(
                        sheet, row, 1, serialNumber++);

                    ExcelWriteUtils::writeString(
                        sheet, row, 2, block.getName());

                    ExcelWriteUtils::writeString(
                        sheet,
                        row,
                        3,
                        BlockDesignation::subblock(
                            block.getName(),
                            subblockNumber));

                    ExcelWriteUtils::writeString(
                        sheet,
                        row,
                        4,
                        borehole.getBoreholeNumber());

                    ExcelWriteUtils::writeString(
                        sheet,
                        row,
                        5,
                        sample.getSampleNumber());

                    ExcelWriteUtils::writeDouble(
                        sheet, row, 6, sample.getStartDepth());

                    ExcelWriteUtils::writeDouble(
                        sheet, row, 7, sample.getEndDepth());

                    ExcelWriteUtils::writeString(
                        sheet, row, 8, normative.gostSandType);

                    ExcelWriteUtils::writeString(
                        sheet, row, 9, normative.spSandType);

                    // 10 — классификация по ТКП 17.04-37-2012.
                    double gravel5 = 0.0;
                    if (const auto* parameter = sample.findGOSTParameter("Грансостав гравия >5 мм"))
                    {
                        if (const auto value = getNumericValue(*parameter))
                            gravel5 = *value;
                    }
                    ExcelWriteUtils::writeString(
                        sheet, row, 10, NormativeAnalysis::tkp1704372012(gravel5));

                    // Нынешние столбцы смещены на один вправо.
                    ExcelWriteUtils::writeString(
                        sheet, row, 11, normative.gostClass);

                    ExcelWriteUtils::writeString(
                        sheet, row, 12, normative.gost31426Type);

                    ExcelWriteUtils::writeString(
                        sheet, row, 13, normative.gost23735Compliance);

                    ExcelWriteUtils::writeString(
                        sheet, row, 14, normative.tkp682FrostHeave);

                    ++row;
                };

                if (intersectsDryMineral)
                    writeRow(1);

                if (intersectsWaterBearingMineral)
                    writeRow(2);
            }
        }
    }
}
