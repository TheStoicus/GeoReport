#pragma once

#include <string>
#include <vector>

/*
 * Пользовательский выбор линий геолого-литологических разрезов.
 * Каждый элемент внешнего вектора — отдельный разрез.
 * Внутри — номера скважин в порядке построения разреза.
 */
class SectionSettings
{
public:
    void setSections(const std::vector<std::vector<std::string>>& sections);

    const std::vector<std::vector<std::string>>& getSections() const;

private:
    std::vector<std::vector<std::string>> sections;
};
