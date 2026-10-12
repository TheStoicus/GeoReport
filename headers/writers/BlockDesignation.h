#pragma once

#include <string>
#include <utility>

class BlockDesignation
{
public:
    static std::pair<std::string, std::string> parse(
        const std::string& name);

    static std::string subblock(
        const std::string& name,
        int number);

    // Нормализованное отображение: «III B», «IV C1» и т. п.
    static std::string format(const std::string& name);
};
