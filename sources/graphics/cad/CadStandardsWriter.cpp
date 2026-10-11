#include "../../../headers/graphics/cad/CadStandardsWriter.h"
#include <fstream>
#include <stdexcept>
#include <string>

void CadStandardsWriter::write(const std::filesystem::path& outputDirectory) const
{
    const auto dir = outputDirectory / "symbols";
    std::filesystem::create_directories(dir);
    const auto write = [](const std::filesystem::path& path, const std::string& text)
    {
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file.is_open())
            throw std::runtime_error("Не удалось создать файл условных обозначений: " + path.string());
        file << text;
    };
    write(dir / "GeoReport_Linetypes.lin",
          "; GeoReport — типы линий для AutoCAD\n"
          "; Загрузка: LINETYPE -> Load -> File.\n\n"
          "*GEO_B,GeoReport B - штриховая\nA,6,-3\n\n"
          "*GEO_C1,GeoReport C1 - штрихпунктирная\nA,12,-6,0,-3\n\n"
          "*GEO_C2,GeoReport C2 - штрихпунктирная с двумя точками\nA,12,-6,0,-3,0,-3\n\n"
          "*GEO_GROUNDWATER,GeoReport - уровень грунтовых вод\nA,6,-3\n");
    write(dir / "GeoReport_BlockContours.lin",
          "; Типы линий контуров блоков GeoReport\n\n"
          "*GEO_B,Категория B - штриховая\nA,6,-3\n\n"
          "*GEO_C1,Категория C1 - штрихпунктирная\nA,12,-6,0,-3\n\n"
          "*GEO_C2,Категория C2 - штрихпунктирная с двумя точками\nA,12,-6,0,-3,0,-3\n");
    write(dir / "README.txt",
          "Условные обозначения GeoReport\n\n"
          "GEO_B  — категория B, штриховая.\n"
          "GEO_C1 — категория C1, штрихпунктирная.\n"
          "GEO_C2 — категория C2, штрихпунктирная с двумя точками.\n"
          "GEO_GROUNDWATER — уровень грунтовых вод.\n\n"
          "Файлы .lin загружаются в AutoCAD командой LINETYPE.\n"
          "В дальнейшем сюда будут добавлены .pat-заливки пород.\n");
}
