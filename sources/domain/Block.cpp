#include "../../headers/domain/Block.h"

Block::Block(const std::string& name) : blockName(name) {}

const std::string& Block::getName() const
{
    return blockName;
}

void Block::addBorehole(const std::string& boreholeNumber)
{
    if (boreholeNumber.empty())
        return;

    for (const std::string& existing : boreholeNumbers)
    {
        if (existing == boreholeNumber)
            return;
    }

    boreholeNumbers.push_back(boreholeNumber);
}

const std::vector<std::string>& Block::getBoreholeNumbers() const
{
    return boreholeNumbers;
}
