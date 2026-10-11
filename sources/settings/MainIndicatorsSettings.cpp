#include "../../headers/settings/MainIndicatorsSettings.h"

#include <algorithm>

namespace
{
    bool contains(const std::string& value, const std::string& part)
    {
        return value.find(part) != std::string::npos;
    }
}

std::string MainIndicatorsSettings::trim(const std::string& value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool MainIndicatorsSettings::sameRockType(
    const std::string& left,
    const std::string& right)
{
    return trim(left) == trim(right);
}

void MainIndicatorsSettings::replaceTypes(
    std::vector<std::string>& destination,
    const std::vector<std::string>& source)
{
    destination.clear();

    for (const std::string& value : source)
    {
        const std::string cleaned = trim(value);
        if (cleaned.empty())
            continue;

        if (!containsType(destination, cleaned))
            destination.push_back(cleaned);
    }
}

bool MainIndicatorsSettings::containsType(
    const std::vector<std::string>& types,
    const std::string& value)
{
    return std::any_of(
        types.begin(), types.end(),
        [&value](const std::string& type)
        {
            return sameRockType(type, value);
        });
}

void MainIndicatorsSettings::setOverburdenMode(OverburdenMode mode)
{
    overburdenMode = mode;
}

OverburdenMode MainIndicatorsSettings::getOverburdenMode() const
{
    return overburdenMode;
}

void MainIndicatorsSettings::setFixedOverburdenThickness(double thickness)
{
    fixedOverburdenThickness = std::max(0.0, thickness);
}

double MainIndicatorsSettings::getFixedOverburdenThickness() const
{
    return fixedOverburdenThickness;
}

bool MainIndicatorsSettings::isFixedOverburdenEnabled() const
{
    return overburdenMode == OverburdenMode::FixedThickness &&
           fixedOverburdenThickness > 0.0;
}

void MainIndicatorsSettings::setMinimumOverburdenThickness(double thickness)
{
    minimumOverburdenThickness = std::max(0.0, thickness);
}

double MainIndicatorsSettings::getMinimumOverburdenThickness() const
{
    return minimumOverburdenThickness;
}

void MainIndicatorsSettings::setOverburdenRockTypes(
    const std::vector<std::string>& rockTypes)
{
    replaceTypes(overburdenRockTypes, rockTypes);
}

const std::vector<std::string>& MainIndicatorsSettings::getOverburdenRockTypes() const
{
    return overburdenRockTypes;
}

bool MainIndicatorsSettings::isOverburdenRockType(const std::string& rockType) const
{
    return containsType(overburdenRockTypes, rockType);
}

void MainIndicatorsSettings::setOverburdenSandClassificationSource(
    SandClassificationSource source)
{
    overburdenSandClassificationSource = source;
}

SandClassificationSource MainIndicatorsSettings::getOverburdenSandClassificationSource() const
{
    return overburdenSandClassificationSource;
}

void MainIndicatorsSettings::setOverburdenGostSandClassUseful(
    GostSandClass sandClass, bool value)
{
    if (sandClass == GostSandClass::Unknown) return;
    if (value) overburdenGostSandClasses.insert(sandClass);
    else overburdenGostSandClasses.erase(sandClass);
}

void MainIndicatorsSettings::setOverburdenTkpSandClassUseful(
    TkpSandClass sandClass, bool value)
{
    if (sandClass == TkpSandClass::Unknown) return;
    if (value) overburdenTkpSandClasses.insert(sandClass);
    else overburdenTkpSandClasses.erase(sandClass);
}

bool MainIndicatorsSettings::isOverburdenSample(const Sample& sample) const
{
    const std::string rockType = trim(sample.getRockType());

    if (!isOverburdenRockType(rockType))
        return false;

    if (rockType != "Песок")
        return true;

    const GostSandClass gost = sample.getGostSandClass();
    const TkpSandClass tkp = sample.getTkpSandClass();

    switch (overburdenSandClassificationSource)
    {
        case SandClassificationSource::Gost:
            return gost != GostSandClass::Unknown &&
                   overburdenGostSandClasses.contains(gost);

        case SandClassificationSource::Tkp:
            return tkp != TkpSandClass::Unknown &&
                   overburdenTkpSandClasses.contains(tkp);

        case SandClassificationSource::GostAndTkp:
            return (gost != GostSandClass::Unknown && overburdenGostSandClasses.contains(gost)) ||
                   (tkp != TkpSandClass::Unknown && overburdenTkpSandClasses.contains(tkp));
    }

    return false;
}

void MainIndicatorsSettings::setUsefulRockTypes(
    const std::vector<std::string>& rockTypes)
{
    replaceTypes(usefulRockTypes, rockTypes);
}

const std::vector<std::string>& MainIndicatorsSettings::getUsefulRockTypes() const
{
    return usefulRockTypes;
}

bool MainIndicatorsSettings::isUsefulRockType(const std::string& rockType) const
{
    return containsType(usefulRockTypes, rockType);
}

void MainIndicatorsSettings::setSandUseful(bool value)
{
    sandUseful = value;
}

bool MainIndicatorsSettings::isSandUseful() const
{
    return sandUseful;
}

void MainIndicatorsSettings::setSupesUseful(bool value)
{
    supesUseful = value;
}

bool MainIndicatorsSettings::isSupesUseful() const
{
    return supesUseful;
}

void MainIndicatorsSettings::setSuglinokUseful(bool value)
{
    suglinokUseful = value;
}

bool MainIndicatorsSettings::isSuglinokUseful() const
{
    return suglinokUseful;
}

void MainIndicatorsSettings::setClayUseful(bool value)
{
    clayUseful = value;
}

bool MainIndicatorsSettings::isClayUseful() const
{
    return clayUseful;
}

void MainIndicatorsSettings::setSandClassificationSource(
    SandClassificationSource source)
{
    sandClassificationSource = source;
}

SandClassificationSource MainIndicatorsSettings::getSandClassificationSource() const
{
    return sandClassificationSource;
}

void MainIndicatorsSettings::setGostSandClassUseful(
    GostSandClass sandClass,
    bool value)
{
    if (sandClass == GostSandClass::Unknown)
        return;

    if (value)
        usefulGostSandClasses.insert(sandClass);
    else
        usefulGostSandClasses.erase(sandClass);
}

bool MainIndicatorsSettings::isGostSandClassUseful(GostSandClass sandClass) const
{
    return usefulGostSandClasses.contains(sandClass);
}

void MainIndicatorsSettings::setTkpSandClassUseful(
    TkpSandClass sandClass,
    bool value)
{
    if (sandClass == TkpSandClass::Unknown)
        return;

    if (value)
        usefulTkpSandClasses.insert(sandClass);
    else
        usefulTkpSandClasses.erase(sandClass);
}

bool MainIndicatorsSettings::isTkpSandClassUseful(TkpSandClass sandClass) const
{
    return usefulTkpSandClasses.contains(sandClass);
}

void MainIndicatorsSettings::setUnderlyingRockTypes(
    const std::vector<std::string>& rockTypes)
{
    replaceTypes(underlyingRockTypes, rockTypes);
}

const std::vector<std::string>& MainIndicatorsSettings::getUnderlyingRockTypes() const
{
    return underlyingRockTypes;
}

bool MainIndicatorsSettings::isUnderlyingRockType(const std::string& rockType) const
{
    return containsType(underlyingRockTypes, rockType);
}

void MainIndicatorsSettings::setUnderlyingSandClassificationSource(
    SandClassificationSource source)
{
    underlyingSandClassificationSource = source;
}

SandClassificationSource MainIndicatorsSettings::getUnderlyingSandClassificationSource() const
{
    return underlyingSandClassificationSource;
}

void MainIndicatorsSettings::setUnderlyingGostSandClassUseful(
    GostSandClass sandClass, bool value)
{
    if (sandClass == GostSandClass::Unknown) return;
    if (value) underlyingGostSandClasses.insert(sandClass);
    else underlyingGostSandClasses.erase(sandClass);
}

void MainIndicatorsSettings::setUnderlyingTkpSandClassUseful(
    TkpSandClass sandClass, bool value)
{
    if (sandClass == TkpSandClass::Unknown) return;
    if (value) underlyingTkpSandClasses.insert(sandClass);
    else underlyingTkpSandClasses.erase(sandClass);
}

bool MainIndicatorsSettings::isUnderlyingSample(const Sample& sample) const
{
    const std::string rockType = trim(sample.getRockType());

    if (!isUnderlyingRockType(rockType))
        return false;

    if (rockType != "Песок")
        return true;

    const GostSandClass gost = sample.getGostSandClass();
    const TkpSandClass tkp = sample.getTkpSandClass();

    switch (underlyingSandClassificationSource)
    {
        case SandClassificationSource::Gost:
            return gost != GostSandClass::Unknown &&
                   underlyingGostSandClasses.contains(gost);

        case SandClassificationSource::Tkp:
            return tkp != TkpSandClass::Unknown &&
                   underlyingTkpSandClasses.contains(tkp);

        case SandClassificationSource::GostAndTkp:
            return (gost != GostSandClass::Unknown && underlyingGostSandClasses.contains(gost)) ||
                   (tkp != TkpSandClass::Unknown && underlyingTkpSandClasses.contains(tkp));
    }

    return false;
}

void MainIndicatorsSettings::setSurfaceSampleMode(SurfaceSampleMode mode)
{
    surfaceSampleMode = mode;
}

SurfaceSampleMode MainIndicatorsSettings::getSurfaceSampleMode() const
{
    return surfaceSampleMode;
}

bool MainIndicatorsSettings::isUseful(const Sample& sample) const
{
    if (sample.isAgSample() || sample.isMonolith())
        return false;

    if (sample.isSurfaceSample() &&
        surfaceSampleMode != SurfaceSampleMode::UsefulMineral)
    {
        return false;
    }

    const std::string rockType = trim(sample.getRockType());

    if (rockType.empty() || isUnderlyingSample(sample))
        return false;

    if (!isUsefulRockType(rockType))
        return false;

    if (rockType == "Песок")
    {
        const GostSandClass gost = sample.getGostSandClass();
        const TkpSandClass tkp = sample.getTkpSandClass();

        switch (sandClassificationSource)
        {
            case SandClassificationSource::Gost:
                return gost != GostSandClass::Unknown &&
                       isGostSandClassUseful(gost);

            case SandClassificationSource::Tkp:
                return tkp != TkpSandClass::Unknown &&
                       isTkpSandClassUseful(tkp);

            case SandClassificationSource::GostAndTkp:
                return (gost != GostSandClass::Unknown &&
                        isGostSandClassUseful(gost)) ||
                       (tkp != TkpSandClass::Unknown &&
                        isTkpSandClassUseful(tkp));
        }
    }

    if (rockType == "Супесь")
        return supesUseful;

    if (rockType == "Суглинок")
        return suglinokUseful;

    if (rockType == "Глина")
        return clayUseful;

    return true;
}
