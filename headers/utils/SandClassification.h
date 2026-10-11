#pragma once

#include <string>


// Классификация песка по ГОСТ.
// Важно: эти значения не следует смешивать с классификацией песка по ТКП.
enum class GostSandClass
{
    Unknown,
    VeryFine,           // очень тонкий
    Fine,               // тонкий
    VerySmall,          // очень мелкий
    Small,              // мелкий
    Medium,             // средний
    Coarse,             // крупный
    IncreasedCoarseness // повышенной крупности
};

// Классификация песка по ТКП.
enum class TkpSandClass
{
    Unknown,
    Dusty,       // пылеватый
    Fine,        // мелкий
    Medium,      // средний
    Coarse,      // крупный
    Gravelly     // гравелистый
};

class SandClassification
{
public:
    static GostSandClass classifyGost(const std::string& soilType); // ГОСТ
    static TkpSandClass classifyTkp(const std::string& soilType); // ТКП

    // Человекочитаемые названия
    static std::string toString(GostSandClass sandClass);
    static std::string toString(TkpSandClass sandClass);
};
