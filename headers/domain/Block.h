#pragma once

#include <string>
#include <vector>

class Block
{
private:
    std::string blockName;
    std::vector<std::string> boreholeNumbers;

public:
    explicit Block(const std::string& name);

    const std::string& getName() const;

    void addBorehole(const std::string& boreholeNumber);

    const std::vector<std::string>& getBoreholeNumbers() const;
};
