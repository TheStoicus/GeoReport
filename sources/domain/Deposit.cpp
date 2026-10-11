#include "../../headers/domain/Deposit.h"

void Deposit::addBorehole(const Borehole& borehole)
{
    boreholes.push_back(borehole);
}

const std::vector<Borehole>& Deposit::getBoreholes() const
{
    return boreholes;
}

std::vector<Borehole>& Deposit::getBoreholesModifiable()
{
    return boreholes;
}

Borehole* Deposit::findBorehole(const std::string& number)
{
    for (Borehole& borehole : boreholes)
    {
        if (borehole.getBoreholeNumber() == number)
            return &borehole;
    }

    return nullptr;
}

const Borehole* Deposit::findBorehole(const std::string& number) const
{
    for (const Borehole& borehole : boreholes)
    {
        if (borehole.getBoreholeNumber() == number)
            return &borehole;
    }

    return nullptr;
}

void Deposit::addBlock(const Block& block)
{
    blocks.push_back(block);
}

const std::vector<Block>& Deposit::getBlocks() const
{
    return blocks;
}

Block* Deposit::findBlock(const std::string& name)
{
    for (Block& block : blocks)
    {
        if (block.getName() == name)
            return &block;
    }

    return nullptr;
}

const Block* Deposit::findBlock(const std::string& name) const
{
    for (const Block& block : blocks)
    {
        if (block.getName() == name)
            return &block;
    }

    return nullptr;
}
