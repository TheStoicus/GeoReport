#pragma once

#include "../../application/GeologicalAnalysisResult.h"
#include "../../domain/Deposit.h"
#include "../../settings/SectionSettings.h"

#include <filesystem>
#include <string>

/*
 * Третье графическое приложение — геолого-литологические разрезы в DWG.
 *
 * Пользователь заранее задаёт количество разрезов и порядок скважин
 * для каждого из них. Все выбранные разрезы записываются в один DWG.
 * Сначала строится временный DXF, затем LibreDWG dwgwrite преобразует
 * его в DWG R2000.
 */
class DwgGeologicalSectionWriter
{
public:
    void write(
        const std::filesystem::path& filePath,
        const std::filesystem::path& projectRoot,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        const SectionSettings& sectionSettings) const;

private:
    std::filesystem::path findDwgWriteExecutable(
        const std::filesystem::path& projectRoot) const;

    static std::string quoteShellArgument(const std::string& value);
};
