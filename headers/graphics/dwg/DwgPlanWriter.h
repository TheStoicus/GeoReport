#pragma once

#include "../../application/GeologicalAnalysisResult.h"
#include "../../domain/Deposit.h"
#include "../../settings/ReserveSummarySettings.h"

#include <filesystem>
#include <string>

/*
 * Создаёт второе графическое приложение в формате DWG.
 *
 * Используется свободная библиотека GNU LibreDWG. Сначала строится
 * временный DXF с 2D-сущностями, затем штатная утилита LibreDWG
 * dwgwrite преобразует его в DWG R2000.
 */
class DwgPlanWriter
{
public:
    void write(
        const std::filesystem::path& filePath,
        const std::filesystem::path& projectRoot,
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        const ReserveSummarySettings& reserveSummarySettings) const;

private:
    std::filesystem::path findDwgWriteExecutable(
        const std::filesystem::path& projectRoot) const;

    static std::string quoteShellArgument(
        const std::string& value);
};
