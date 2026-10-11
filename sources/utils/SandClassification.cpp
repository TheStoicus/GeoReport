#include "../../headers/utils/SandClassification.h"


namespace
{
    bool contains(
        const std::string& value,
        const std::string& part)
    {
        return value.find(part) != std::string::npos;
    }
}


// ============================================================
// ГОСТ
// ============================================================

GostSandClass SandClassification::classifyGost(
    const std::string& soilType)
{
    if (soilType.empty())
    {
        return GostSandClass::Unknown;
    }


    /*
     * В исходной таблице ГОСТ слово "песок"
     * может отсутствовать.
     *
     * Например:
     *
     *     "очень тонкий"
     *     "тонкий"
     *     "очень мелкий"
     *     "мелкий"
     *     "средний"
     *
     * Поэтому классифицируем непосредственно
     * по текстовому описанию.
     */


    // --------------------------------------------------------
    // Очень тонкий
    // --------------------------------------------------------

    if (contains(soilType, "очень тонкий") ||
        contains(soilType, "Очень тонкий"))
    {
        return GostSandClass::VeryFine;
    }


    // --------------------------------------------------------
    // Тонкий
    // --------------------------------------------------------

    if (contains(soilType, "тонкий") ||
        contains(soilType, "Тонкий"))
    {
        return GostSandClass::Fine;
    }


    // --------------------------------------------------------
    // Очень мелкий
    //
    // Проверяем раньше "мелкий".
    // --------------------------------------------------------

    if (contains(soilType, "очень мелкий") ||
        contains(soilType, "Очень мелкий"))
    {
        return GostSandClass::VerySmall;
    }


    // --------------------------------------------------------
    // Мелкий
    // --------------------------------------------------------

    if (contains(soilType, "мелкий") ||
        contains(soilType, "Мелкий"))
    {
        return GostSandClass::Small;
    }


    // --------------------------------------------------------
    // Средний
    // --------------------------------------------------------

    if (contains(soilType, "средний") ||
        contains(soilType, "Средний"))
    {
        return GostSandClass::Medium;
    }


    // --------------------------------------------------------
    // Повышенной крупности
    // --------------------------------------------------------

    if (contains(soilType, "повышенной крупности") ||
        contains(soilType, "Повышенной крупности"))
    {
        return GostSandClass::IncreasedCoarseness;
    }


    // --------------------------------------------------------
    // Крупный
    // --------------------------------------------------------

    if (contains(soilType, "крупный") ||
        contains(soilType, "Крупный"))
    {
        return GostSandClass::Coarse;
    }


    return GostSandClass::Unknown;
}


// ============================================================
// ТКП
// ============================================================

TkpSandClass SandClassification::classifyTkp(
    const std::string& soilType)
{
    if (soilType.empty())
    {
        return TkpSandClass::Unknown;
    }


    // --------------------------------------------------------
    // Пылеватый
    // --------------------------------------------------------

    if (contains(soilType, "пылеватый") ||
        contains(soilType, "Пылеватый"))
    {
        return TkpSandClass::Dusty;
    }


    // --------------------------------------------------------
    // Мелкий
    // --------------------------------------------------------

    if (contains(soilType, "мелкий") ||
        contains(soilType, "Мелкий"))
    {
        return TkpSandClass::Fine;
    }


    // --------------------------------------------------------
    // Средний
    // --------------------------------------------------------

    if (contains(soilType, "средний") ||
        contains(soilType, "Средний"))
    {
        return TkpSandClass::Medium;
    }


    // --------------------------------------------------------
    // Крупный
    // --------------------------------------------------------

    if (contains(soilType, "крупный") ||
        contains(soilType, "Крупный"))
    {
        return TkpSandClass::Coarse;
    }


    // --------------------------------------------------------
    // Гравелистый
    // --------------------------------------------------------

    if (contains(soilType, "гравелистый") ||
        contains(soilType, "Гравелистый"))
    {
        return TkpSandClass::Gravelly;
    }


    return TkpSandClass::Unknown;
}


// ============================================================
// Название класса ГОСТ
// ============================================================

std::string SandClassification::toString(
    GostSandClass sandClass)
{
    switch (sandClass)
    {
        case GostSandClass::VeryFine:
            return "Очень тонкий";

        case GostSandClass::Fine:
            return "Тонкий";

        case GostSandClass::VerySmall:
            return "Очень мелкий";

        case GostSandClass::Small:
            return "Мелкий";

        case GostSandClass::Medium:
            return "Средний";

        case GostSandClass::Coarse:
            return "Крупный";

        case GostSandClass::IncreasedCoarseness:
            return "Повышенной крупности";

        case GostSandClass::Unknown:
        default:
            return {};
    }
}


// ============================================================
// Название класса ТКП
// ============================================================

std::string SandClassification::toString(
    TkpSandClass sandClass)
{
    switch (sandClass)
    {
        case TkpSandClass::Dusty:
            return "Пылеватый";

        case TkpSandClass::Fine:
            return "Мелкий";

        case TkpSandClass::Medium:
            return "Средний";

        case TkpSandClass::Coarse:
            return "Крупный";

        case TkpSandClass::Gravelly:
            return "Гравелистый";

        case TkpSandClass::Unknown:
        default:
            return {};
    }
}