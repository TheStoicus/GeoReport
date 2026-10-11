#pragma once
#include <filesystem>
class CadStandardsWriter
{
public:
    void write(const std::filesystem::path& outputDirectory) const;
};
