#include "../../headers/writers/BlockDesignation.h"

#include <cctype>
#include <string>
#include <utility>

std::pair<std::string, std::string> BlockDesignation::parse(
    const std::string& name)
{
    if (name.size() < 2)
        return {name, {}};

    std::size_t position = name.size() - 1;

    while (position > 0 &&
           std::isdigit(static_cast<unsigned char>(name[position - 1])))
    {
        --position;
    }

    if (position > 0)
    {
        const char category = name[position - 1];

        if ((category >= 'A' && category <= 'Z') ||
            (category >= 'a' && category <= 'z'))
        {
            --position;
        }
    }

    if (position == 0)
        return {name, {}};

    return {
        name.substr(0, position),
        name.substr(position)
    };
}

std::string BlockDesignation::subblock(
    const std::string& name,
    int number)
{
    const auto designation = parse(name);

    if (designation.second.empty())
        return name + "-" + std::to_string(number);

    return designation.first + "-" +
           std::to_string(number) + designation.second;
}
