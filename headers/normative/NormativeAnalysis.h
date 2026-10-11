#pragma once

#include "../domain/Sample.h"

#include <string>

struct NormativeAnalysisResult
{
    std::string gostSandType;
    std::string spSandType;
    std::string gostClass;
    std::string gost31426Type;
    std::string gost23735Compliance;
    std::string tkp682FrostHeave;
};

class NormativeAnalysis
{
public:
    static NormativeAnalysisResult analyze(const Sample& sample);

    // Проверки, используемые таблицами 14, 15, 17 и листом 5.
    static bool gost8736ClassIILess016(double module, double less016);
    static bool gost8736ClassIIClayey(double module, double clayey);
    static bool gost8736FullResidue063(double module, double residue063);
    static bool gost23735Values(double module, double gravel5, double less016, double clayey);
    static std::string gost8736ClassFromValues(double module, double residue063, double remainder10, double remainder5, double less016, double clayey);
    static std::string tkp1704372012(double gravel5);

private:
    static bool gost(
        const Sample& sample,
        const std::string& name,
        double& value);

    static bool tkp(
        const Sample& sample,
        const std::string& name,
        double& value);

    static std::string gostSand(const Sample& sample);
    static std::string spSand(const Sample& sample);
    static std::string gostClass(const Sample& sample);
    static std::string gost31426(const Sample& sample);
    static std::string gost23735(const Sample& sample);
    static std::string tkp682(const Sample& sample);
};
