#include "GeologicalModel.h"

#include <cmath>
#include <set>
#include <string>
#include <unordered_set>

namespace terranex::domain
{
namespace
{

bool isBlank(const std::string& value)
{
    return value.find_first_not_of(" \t\n\r") == std::string::npos;
}

bool isFinite(double value)
{
    return std::isfinite(value);
}

void addIssue(std::vector<ValidationIssue>& issues,
              const std::string& code,
              const std::string& message)
{
    issues.push_back({code, message});
}

bool validLaboratoryParameters(const std::map<std::string, double>& parameters)
{
    for (const auto& [name, value] : parameters)
    {
        if (isBlank(name) || !isFinite(value))
        {
            return false;
        }
    }
    return true;
}

} // namespace

std::vector<ValidationIssue> validateDeposit(const Deposit& deposit)
{
    std::vector<ValidationIssue> issues;

    if (isBlank(deposit.id))
        addIssue(issues, "deposit.id.empty", "Не задан идентификатор месторождения.");

    if (isBlank(deposit.name))
        addIssue(issues, "deposit.name.empty", "Не задано название месторождения.");

    if (isBlank(deposit.coordinateReferenceSystem))
        addIssue(issues, "deposit.crs.empty", "Не задана система координат проекта.");

    std::unordered_set<std::string> blockIds;
    for (const Block& block : deposit.blocks)
    {
        if (isBlank(block.id))
            addIssue(issues, "block.id.empty", "У блока отсутствует идентификатор.");
        else if (!blockIds.insert(block.id).second)
            addIssue(issues, "block.id.duplicate", "Повторяется идентификатор блока: " + block.id);

        if (isBlank(block.name))
            addIssue(issues, "block.name.empty", "У блока " + block.id + " отсутствует название.");
    }

    std::unordered_set<std::string> boreholeIds;
    for (const Borehole& borehole : deposit.boreholes)
    {
        if (isBlank(borehole.id))
            addIssue(issues, "borehole.id.empty", "У скважины отсутствует идентификатор.");
        else if (!boreholeIds.insert(borehole.id).second)
            addIssue(issues, "borehole.id.duplicate", "Повторяется идентификатор скважины: " + borehole.id);

        if (isBlank(borehole.number))
            addIssue(issues, "borehole.number.empty", "У скважины " + borehole.id + " отсутствует номер.");

        if (!isFinite(borehole.x) || !isFinite(borehole.y))
            addIssue(issues, "borehole.coordinates.invalid",
                     "Координаты скважины " + borehole.number + " должны быть конечными числами.");

        if (borehole.elevation && !isFinite(*borehole.elevation))
            addIssue(issues, "borehole.elevation.invalid",
                     "Отметка устья скважины " + borehole.number + " некорректна.");

        if (borehole.groundwaterDepth &&
            (!isFinite(*borehole.groundwaterDepth) || *borehole.groundwaterDepth < 0.0))
            addIssue(issues, "borehole.groundwater.invalid",
                     "Глубина грунтовых вод скважины " + borehole.number + " некорректна.");

        std::unordered_set<std::string> linkedBlocks;
        for (const std::string& blockId : borehole.blockIds)
        {
            if (!blockIds.contains(blockId))
                addIssue(issues, "borehole.block.missing",
                         "Скважина " + borehole.number + " ссылается на неизвестный блок: " + blockId);
            if (!linkedBlocks.insert(blockId).second)
                addIssue(issues, "borehole.block.duplicate",
                         "В связях скважины " + borehole.number + " повторяется блок: " + blockId);
        }
    }

    std::unordered_set<std::string> sampleIds;
    for (const Sample& sample : deposit.samples)
    {
        if (isBlank(sample.id))
            addIssue(issues, "sample.id.empty", "У пробы отсутствует идентификатор.");
        else if (!sampleIds.insert(sample.id).second)
            addIssue(issues, "sample.id.duplicate", "Повторяется идентификатор пробы: " + sample.id);

        if (isBlank(sample.number))
            addIssue(issues, "sample.number.empty", "У пробы " + sample.id + " отсутствует номер.");

        if (!boreholeIds.contains(sample.boreholeId))
            addIssue(issues, "sample.borehole.missing",
                     "Проба " + sample.number + " ссылается на неизвестную скважину: " + sample.boreholeId);

        if (!isFinite(sample.depthFrom) || !isFinite(sample.depthTo) ||
            sample.depthFrom < 0.0 || sample.depthTo <= sample.depthFrom)
            addIssue(issues, "sample.interval.invalid",
                     "Интервал пробы " + sample.number + " должен иметь глубины 0 <= начало < конец.");

        if (!validLaboratoryParameters(sample.laboratory.gostParameters))
            addIssue(issues, "sample.lab.gost.invalid",
                     "Параметры ГОСТ пробы " + sample.number + " содержат пустое имя или некорректное число.");

        if (!validLaboratoryParameters(sample.laboratory.spParameters))
            addIssue(issues, "sample.lab.sp.invalid",
                     "Параметры СП пробы " + sample.number + " содержат пустое имя или некорректное число.");
    }

    return issues;
}

} // namespace terranex::domain
