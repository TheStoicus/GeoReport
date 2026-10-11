#include "../../../headers/graphics/dwg/DwgPlanWriter.h"

#include "../../../headers/graphics/cad/BoreholePlanDrawingBuilder.h"
#include "../../../headers/graphics/dxf/DxfDocument.h"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>

namespace
{
    bool isRegularFile(const std::filesystem::path& path)
    {
        std::error_code error;
        return std::filesystem::exists(path, error) &&
               std::filesystem::is_regular_file(path, error);
    }
}

std::string DwgPlanWriter::quoteShellArgument(const std::string& value)
{
    std::string result = "'";

    for (const char character : value)
    {
        if (character == '\'')
            result += "'\\''";
        else
            result += character;
    }

    result += "'";
    return result;
}

std::filesystem::path DwgPlanWriter::findDwgWriteExecutable(
    const std::filesystem::path& projectRoot) const
{
#ifdef GEOREPORT_DWGWRITE_PATH
    {
        const std::filesystem::path candidate(GEOREPORT_DWGWRITE_PATH);

        if (isRegularFile(candidate))
            return candidate;
    }
#endif

    if (const char* environment = std::getenv("GEOREPORT_DWGWRITE"))
    {
        const std::filesystem::path candidate(environment);

        if (isRegularFile(candidate))
            return candidate;
    }

    const std::filesystem::path librariesRoot =
        projectRoot.parent_path().parent_path() / "CppLibs";

    const std::filesystem::path libreDwgRoot =
        librariesRoot / "LibreDWG";

    const std::filesystem::path candidates[] =
    {
        libreDwgRoot / "programs" / "dwgwrite",
        libreDwgRoot / "src" / "dwgwrite",
        libreDwgRoot / "dwgwrite"
    };

    for (const auto& candidate : candidates)
    {
        if (isRegularFile(candidate))
            return candidate;
    }

    throw std::runtime_error(
        "Не найден LibreDWG dwgwrite. Пересоберите проект CMake "
        "или задайте переменную окружения GEOREPORT_DWGWRITE.");
}

void DwgPlanWriter::write(
    const std::filesystem::path& filePath,
    const std::filesystem::path& projectRoot,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult,
    const ReserveSummarySettings& reserveSummarySettings) const
{
    std::filesystem::create_directories(filePath.parent_path());

    const std::filesystem::path dwgWrite =
        findDwgWriteExecutable(projectRoot);

    const std::filesystem::path temporaryDxf =
        filePath.parent_path() / ".georeport_block_plan_source.dxf";

    const std::filesystem::path logFile =
        filePath.parent_path() / ".georeport_dwgwrite.log";

    DxfDocument document;
    BoreholePlanDrawingBuilder builder;

    builder.build(
        document,
        deposit,
        analysisResult,
        reserveSummarySettings,
        true);

    document.save(temporaryDxf.string());

    const std::string command =
        quoteShellArgument(dwgWrite.string()) +
        " --as r2000 -y -o " +
        quoteShellArgument(filePath.string()) +
        " " +
        quoteShellArgument(temporaryDxf.string()) +
        " > " +
        quoteShellArgument(logFile.string()) +
        " 2>&1";

    const int result = std::system(command.c_str());

    if (result != 0 || !std::filesystem::exists(filePath))
    {
        std::string details;

        std::ifstream log(logFile);
        if (log.is_open())
        {
            details.assign(
                (std::istreambuf_iterator<char>(log)),
                std::istreambuf_iterator<char>());
        }

        throw std::runtime_error(
            "LibreDWG не смог создать DWG. Код возврата: " +
            std::to_string(result) +
            ". Подробности: " +
            (details.empty() ? logFile.string() : details));
    }

    std::error_code error;
    std::filesystem::remove(temporaryDxf, error);
    std::filesystem::remove(logFile, error);
}
