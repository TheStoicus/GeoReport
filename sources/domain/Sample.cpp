#include "../../headers/domain/Sample.h"

#include <cmath>
#include <string>

namespace
{
    // Удаляет пробелы в начале и конце строки.
    std::string trim(const std::string& value)
    {
        const auto first = value.find_first_not_of(" \t\r\n");

        if (first == std::string::npos)
            return {};

        const auto last = value.find_last_not_of(" \t\r\n");

        return value.substr(first, last - first + 1);
    }

    /*
     * Проверяет наличие фрагмента в строке.
     * Русские символы специально не преобразуем в нижний регистр.
     */
    bool contains(const std::string& value, const std::string& part)
    {
        return value.find(part) != std::string::npos;
    }


    // Допустимая погрешность сравнения глубин.
    constexpr double DEPTH_EPSILON = 1e-9;
}

// Конструктор
Sample::Sample(const std::string& sampleNumber, double startDepth, double endDepth)
    : sampleNumber(sampleNumber), startDepth(startDepth), endDepth(endDepth)
{
    monolith = sampleNumber.rfind("М", 0) == 0 || sampleNumber.rfind("м", 0) == 0;
    agSample = sampleNumber.find("аг") != std::string::npos ||
               sampleNumber.find("Аг") != std::string::npos ||
               sampleNumber.find("аГ") != std::string::npos ||
               sampleNumber.find("АГ") != std::string::npos;
    calculateThickness();
}

// Номер образца
void Sample::setSampleNumber(const std::string& sampleNumber)
{
    // При первом присвоении номера фиксируем исходный тип образца.
    // При последующей сквозной перенумерации эти признаки не меняются.
    if (this->sampleNumber.empty())
    {
        monolith = sampleNumber.rfind("М", 0) == 0 || sampleNumber.rfind("м", 0) == 0;
        agSample = sampleNumber.find("аг") != std::string::npos ||
                   sampleNumber.find("Аг") != std::string::npos ||
                   sampleNumber.find("аГ") != std::string::npos ||
                   sampleNumber.find("АГ") != std::string::npos;
    }
    this->sampleNumber = sampleNumber;
}

const std::string& Sample::getSampleNumber() const
{
    return sampleNumber;
}

// Тип образца
bool Sample::isMonolith() const
{
    return monolith;
}

bool Sample::isSurfaceSample() const
{
    /*
     * Специальный поверхностный образец:
     *     0.0–0.5 м
     * Используем сравнение с небольшой погрешностью, поскольку глубины хранятся как double.
     */
    return
        std::abs(startDepth - 0.0) < DEPTH_EPSILON &&
        std::abs(endDepth - 0.5) < DEPTH_EPSILON;
}

// Глубины
double Sample::getStartDepth() const
{
    return startDepth;
}

double Sample::getEndDepth() const
{
    return endDepth;
}

double Sample::getThickness() const
{
    return thickness;
}

void Sample::setStartDepth(double depth)
{
    startDepth = depth;
    calculateThickness();
}

void Sample::setEndDepth(double depth)
{
    endDepth = depth;
    calculateThickness();
}

void Sample::calculateThickness()
{
    thickness = endDepth - startDepth;

    // Отрицательная мощность образца физически невозможна.
    if (thickness < 0.0)
        thickness = 0.0;
}

// Название грунта по ГОСТ
void Sample::setGostSoilType(const std::string& soilType)
{
    gostSoilType = trim(soilType);
}

const std::string& Sample::getGostSoilType() const
{
    return gostSoilType;
}

// Название грунта по ТКП
void Sample::setTkpSoilType(const std::string& soilType)
{
    tkpSoilType = trim(soilType);
}

const std::string& Sample::getTkpSoilType() const
{
    return tkpSoilType;
}

// Унифицированное название грунта
std::string Sample::getSoilType() const
{
    // Если название есть в ГОСТ — используем его. Иначе используем ТКП.
    if (!gostSoilType.empty())
        return gostSoilType;

    return tkpSoilType;
}

// Базовый тип породы
std::string Sample::getRockType() const
{
    const std::string soilType = getSoilType();

    if (soilType.empty())
        return {};

    /*
     * Песок по ГОСТ в исходном Excel может быть записан без слова
     * "песок": например "Очень мелкий", "Мелкий", "Средний".
     * Поэтому сначала проверяем оба лабораторных классификатора.
     *
     * Это принципиально важно: в дальнейшем MainIndicatorsSettings
     * должен работать с единым базовым типом "Песок", а конкретный
     * класс песка определяется отдельно по ГОСТ/ТКП.
     */
    if (SandClassification::classifyGost(gostSoilType) != GostSandClass::Unknown ||
        SandClassification::classifyTkp(tkpSoilType) != TkpSandClass::Unknown ||
        contains(soilType, "Песок") ||
        contains(soilType, "песок"))
    {
        return "Песок";
    }

    // Супесь
    if (contains(soilType, "Супесь") || contains(soilType, "супесь"))
        return "Супесь";

    // Суглинок
    if (contains(soilType, "Суглинок") || contains(soilType, "суглинок"))
        return "Суглинок";

    // Глина
    if (contains(soilType, "Глина") || contains(soilType, "глина"))
        return "Глина";

    // Неизвестный тип пока не выбрасываем. Возвращаем исходное название.
    return soilType;
}

// Класс песка по ГОСТ
GostSandClass Sample::getGostSandClass() const
{
    /*
     * Классификация выполняется именно по названию, полученному из ГОСТ.
     * Мы намеренно НЕ используем getSoilType(), потому что нельзя подменять классификацию ГОСТ значением из ТКП.
     */
    if (gostSoilType.empty())
        return GostSandClass::Unknown;

    /*
     * В исходной таблице ГОСТ слово "Песок" может отсутствовать:
     *     "мелкий", "средний", "очень тонкий"
     * Поэтому классификатор SandClassification непосредственно определяет класс по описанию.
     */
    return SandClassification::classifyGost(gostSoilType);
}

// Класс песка по ТКП
TkpSandClass Sample::getTkpSandClass() const
{
    if (tkpSoilType.empty())
        return TkpSandClass::Unknown;

    return SandClassification::classifyTkp(tkpSoilType);
}

// Параметры ГОСТ
void Sample::setGOSTParameter(const std::string& name, const ParameterValue& value)
{
    gostParameters[name] = value;
}

const std::map<std::string, ParameterValue>& Sample::getGOSTParameters() const
{
    return gostParameters;
}

const ParameterValue* Sample::findGOSTParameter(const std::string& name) const
{
    const auto iterator = gostParameters.find(name);

    if (iterator == gostParameters.end())
        return nullptr;

    return &iterator->second;
}

// Параметры ТКП
void Sample::setTKPParameter(const std::string& name, const ParameterValue& value)
{
    tkpParameters[name] = value;
}

const std::map<std::string, ParameterValue>& Sample::getTKPParameters() const
{
    return tkpParameters;
}

const ParameterValue* Sample::findTKPParameter(const std::string& name) const
{
    const auto iterator = tkpParameters.find(name);

    if (iterator == tkpParameters.end())
        return nullptr;

    return &iterator->second;
}

bool Sample::isAgSample() const
{
    return agSample;
}
