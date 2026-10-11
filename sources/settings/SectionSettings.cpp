#include "../../headers/settings/SectionSettings.h"

void SectionSettings::setSections(
    const std::vector<std::vector<std::string>>& value)
{
    sections = value;
}

const std::vector<std::vector<std::string>>& SectionSettings::getSections() const
{
    return sections;
}
