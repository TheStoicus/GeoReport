#pragma once

#include <string>
#include <vector>

#include "Block.h"
#include "Borehole.h"

class Deposit
{
private:
    std::vector<Borehole> boreholes;
    std::vector<Block> blocks;

public:
    void addBorehole(const Borehole& borehole);

    const std::vector<Borehole>& getBoreholes() const;
    std::vector<Borehole>& getBoreholesModifiable();
    const std::vector<Block>& getBlocks() const;

    Borehole* findBorehole(const std::string& number);
    const Borehole* findBorehole(const std::string& number) const;

    void addBlock(const Block& block);

    Block* findBlock(const std::string& name);
    const Block* findBlock(const std::string& name) const;
};
