#include "../../headers/writers/BlockDesignation.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace
{
    std::string trim(const std::string& value)
    {
        const auto first = std::find_if_not(
            value.begin(), value.end(),
            [](unsigned char c) { return std::isspace(c); });

        if (first == value.end())
            return {};

        const auto last = std::find_if_not(
            value.rbegin(), value.rend(),
            [](unsigned char c) { return std::isspace(c); }).base();

        return std::string(first, last);
    }

    std::string upperAscii(std::string value)
    {
        for (char& c : value)
        {
            if (c >= 'a' && c <= 'z')
                c = static_cast<char>(c - 'a' + 'A');
        }

        return value;
    }
}

std::pair<std::string, std::string> BlockDesignation::parse(
    const std::string& rawName)
{
    const std::string name = trim(rawName);
    if (name.empty())
        return {{}, {}};

    /*
     * Допустимые суффиксы категорий запасов: A, B, C1, C2, P1 и P2.
     * Всё перед суффиксом является обозначением блока/подблока:
     * IIIB -> III / B; IVC1 -> IV / C1; III-1B -> III-1 / B.
     */
    const std::string upper = upperAscii(name);
    const char last = upper.back();

    if ((last == '1' || last == '2') && upper.size() > 2)
    {
        const char preceding = upper[upper.size() - 2];
        if (preceding == 'C' || preceding == 'P')
            return {name.substr(0, name.size() - 2),
                    upper.substr(upper.size() - 2)};
    }

    if ((last == 'A' || last == 'B') && upper.size() > 1)
        return {name.substr(0, name.size() - 1),
                upper.substr(upper.size() - 1)};

    return {name, {}};
}

std::string BlockDesignation::subblock(
    const std::string& name,
    int number)
{
    const auto designation = parse(name);
    const std::string subblockName =
        designation.first + "-" + std::to_string(number);

    if (designation.second.empty())
        return subblockName;

    return subblockName + designation.second;
}

std::string BlockDesignation::format(const std::string& name)
{
    const auto designation = parse(name);
    if (designation.second.empty())
        return designation.first;
    return designation.first + " " + designation.second;
}
