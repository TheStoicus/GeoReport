#include "../../headers/settings/ReserveSummarySettings.h"

void ReserveSummarySettings::setAreaMode(BlockAreaMode value)
{
    areaMode = value;
}

BlockAreaMode ReserveSummarySettings::getAreaMode() const
{
    return areaMode;
}

void ReserveSummarySettings::addBlock(
    const ReserveSummaryBlockSettings& value)
{
    for (ReserveSummaryBlockSettings& existing : blocks)
    {
        if (existing.blockName == value.blockName)
        {
            existing = value;
            return;
        }
    }

    blocks.push_back(value);
}

const std::vector<ReserveSummaryBlockSettings>&
ReserveSummarySettings::getBlocks() const
{
    return blocks;
}

const ReserveSummaryBlockSettings*
ReserveSummarySettings::findBlock(
    const std::string& blockName) const
{
    for (const ReserveSummaryBlockSettings& value : blocks)
    {
        if (value.blockName == blockName)
            return &value;
    }

    return nullptr;
}
