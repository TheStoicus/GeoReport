#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace terranex::domain
{

// Параметры хранятся отдельно по нормативному источнику.
// Ключ параметра должен быть стабильным машинным именем, а не подписью в таблице.
struct LaboratoryResults
{
    std::map<std::string, double> gostParameters;
    std::map<std::string, double> spParameters;
};

struct Block
{
    std::string id;
    std::string name;
    std::string reserveCategory;
};

struct Borehole
{
    std::string id;
    std::string number;

    // Координаты сохраняются без округления. Система координат задаётся проектом.
    double x = 0.0;
    double y = 0.0;
    std::optional<double> elevation;
    std::optional<double> groundwaterDepth;

    // Связь многие-ко-многим: одна скважина может пересекать несколько блоков.
    std::vector<std::string> blockIds;
};

struct Sample
{
    std::string id;
    std::string number;
    std::string boreholeId;

    // Глубины в метрах от устья скважины.
    double depthFrom = 0.0;
    double depthTo = 0.0;

    LaboratoryResults laboratory;
};

struct Deposit
{
    std::string id;
    std::string name;
    std::string coordinateReferenceSystem = "SK-1963";

    std::vector<Block> blocks;
    std::vector<Borehole> boreholes;
    std::vector<Sample> samples;
};

struct ValidationIssue
{
    std::string code;
    std::string message;
};

// Проверяет внутреннюю согласованность модели, не изменяя исходные данные.
[[nodiscard]] std::vector<ValidationIssue> validateDeposit(const Deposit& deposit);

} // namespace terranex::domain
