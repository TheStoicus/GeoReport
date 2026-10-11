#include "../../headers/normative/NormativeAnalysis.h"
#include "../../headers/utils/ParameterValue.h"

#include <cmath>
#include <string>

namespace
{
    constexpr double EPS = 1e-9;

    bool inRange(double value, double minimum, double maximum)
    {
        return value >= minimum - EPS && value <= maximum + EPS;
    }

    bool contains(
        const std::string& value,
        const char* text)
    {
        return value.find(text) != std::string::npos;
    }
}

bool NormativeAnalysis::gost(
    const Sample& sample,
    const std::string& name,
    double& value)
{
    const auto* parameter = sample.findGOSTParameter(name);

    if (!parameter)
        return false;

    const auto numericValue = getNumericValue(*parameter);

    if (!numericValue || !std::isfinite(*numericValue))
        return false;

    value = *numericValue;
    return true;
}

bool NormativeAnalysis::tkp(
    const Sample& sample,
    const std::string& name,
    double& value)
{
    const auto* parameter = sample.findTKPParameter(name);

    if (!parameter)
        return false;

    const auto numericValue = getNumericValue(*parameter);

    if (!numericValue || !std::isfinite(*numericValue))
        return false;

    value = *numericValue;
    return true;
}

std::string NormativeAnalysis::spSand(const Sample& sample)
{
    double greaterThan10 = 0.0;
    double from10To5 = 0.0;
    double from5To2 = 0.0;
    double from2To1 = 0.0;
    double from1To05 = 0.0;
    double fraction = 0.0;

    if (!tkp(sample, "ТКП >10 мм", greaterThan10) ||
        !tkp(sample, "ТКП 10-5 мм", from10To5) ||
        !tkp(sample, "ТКП 5-2 мм", from5To2) ||
        !tkp(sample, "ТКП 2-1 мм", from2To1) ||
        !tkp(sample, "ТКП 1-0.5 мм", from1To05))
    {
        return "Нет данных";
    }

    const double greaterThan2 =
        greaterThan10 + from10To5 + from5To2;

    const double greaterThan05 =
        greaterThan2 + from2To1 + from1To05;

    if (!tkp(sample, "ТКП 0.5-0.25 мм", fraction))
        return "Нет данных";

    const double greaterThan025 = greaterThan05 + fraction;

    if (!tkp(sample, "ТКП 0.25-0.1 мм", fraction))
        return "Нет данных";

    const double greaterThan01 = greaterThan025 + fraction;

    if (greaterThan2 > 25.0 + EPS)
        return "Песок гравелистый";

    if (greaterThan05 > 50.0 + EPS)
        return "Песок крупный";

    if (greaterThan025 > 50.0 + EPS)
        return "Песок средний";

    if (greaterThan01 >= 75.0 - EPS)
        return "Песок мелкий";

    return "Песок пылеватый";
}

bool NormativeAnalysis::gost8736FullResidue063(double module, double residue063)
{
    if (!std::isfinite(module) || !std::isfinite(residue063) || residue063 < -EPS)
        return false;

    // Для тонких и очень тонких песков полный остаток на сите 0,63 мм
    // ГОСТ 8736 не нормирует, поэтому отсутствие ограничения считается
    // соответствием данному показателю.
    if (module > 0.7 && module <= 1.0)
        return true;
    if (module >= 0.0 && module <= 0.7)
        return true;

    if (module > 3.0 && module <= 3.5)
        return residue063 > 65.0 - EPS && residue063 <= 75.0 + EPS;
    if (module > 2.5 && module <= 3.0)
        return residue063 > 45.0 - EPS && residue063 <= 65.0 + EPS;
    if (module > 2.0 && module <= 2.5)
        return residue063 > 30.0 - EPS && residue063 <= 45.0 + EPS;
    if (module > 1.5 && module <= 2.0)
        return residue063 > 10.0 - EPS && residue063 <= 30.0 + EPS;
    if (module > 1.0 && module <= 1.5)
        return residue063 <= 10.0 + EPS;

    return false;
}

bool NormativeAnalysis::gost8736ClassIILess016(double module, double less016)
{
    if (!std::isfinite(module) || !std::isfinite(less016) || less016 < -EPS)
        return false;

    // Для тонких и очень тонких песков содержание фракции <0,16 мм
    // по ГОСТ 8736 не нормируется.
    if (module > 0.7 && module <= 1.0)
        return true;
    if (module >= 0.0 && module <= 0.7)
        return true;

    double limit = 0.0;

    if (module > 3.0 && module <= 3.5)
        limit = 10.0;
    else if (module > 2.5 && module <= 3.0)
        limit = 15.0;
    else if (module > 2.0 && module <= 2.5)
        limit = 15.0;
    else if (module > 1.5 && module <= 2.0)
        limit = 20.0;
    else if (module > 1.0 && module <= 1.5)
        limit = 20.0;
    else
        return false;

    return less016 <= limit + EPS;
}

bool NormativeAnalysis::gost8736ClassIIClayey(double module, double clayey)
{
    if (!std::isfinite(module) || !std::isfinite(clayey) || clayey < -EPS)
        return false;

    double limit = 0.0;

    if (module > 3.0 && module <= 3.5)
        limit = 3.0;
    else if (module > 2.5 && module <= 3.0)
        limit = 3.0;
    else if (module > 2.0 && module <= 2.5)
        limit = 3.0;
    else if (module > 1.5 && module <= 2.0)
        limit = 5.0;
    else if (module > 1.0 && module <= 1.5)
        limit = 5.0;
    else if (module > 0.7 && module <= 1.0)
        limit = 10.0;
    else if (module >= 0.0 && module <= 0.7)
        limit = 10.0;
    else
        return false;

    return clayey <= limit + EPS;
}

std::string NormativeAnalysis::gostSand(const Sample& sample)
{
    double module = 0.0;

    if (!gost(sample, "Модуль крупности песка", module))
        return "Нет данных";

    if (module > 3.0 && module <= 3.5)
        return "Песок повышенной крупности";

    if (module > 2.5 && module <= 3.0)
        return "Песок крупный";

    if (module > 2.0 && module <= 2.5)
        return "Песок средний";

    if (module > 1.5 && module <= 2.0)
        return "Песок мелкий";

    if (module > 1.0 && module <= 1.5)
        return "Песок очень мелкий";

    if (module > 0.7 && module <= 1.0)
        return "Песок тонкий";

    if (module >= 0.0 && module <= 0.7)
        return "Песок очень тонкий";

    return "Нет данных";
}

std::string NormativeAnalysis::gostClass(const Sample& sample)
{
    double module = 0.0;
    double residue063 = 0.0;
    double remainder5 = 0.0;
    double less016 = 0.0;
    double clayey = 0.0;
    double greaterThan70 = 0.0;
    double from70To40 = 0.0;
    double from40To20 = 0.0;
    double from20To10 = 0.0;

    if (!gost(sample, "Модуль крупности песка", module) ||
        !gost(sample, "Грансостав песка 0.63 мм", residue063) ||
        !gost(sample, "Грансостав гравия >5 мм", remainder5) ||
        !gost(sample, "Грансостав гравия >70 мм", greaterThan70) ||
        !gost(sample, "Грансостав гравия 70-40 мм", from70To40) ||
        !gost(sample, "Грансостав гравия 40-20 мм", from40To20) ||
        !gost(sample, "Грансостав гравия 20-10 мм", from20To10) ||
        !gost(sample, "Грансостав песка <0.16 мм", less016) ||
        !gost(sample, "Глинистые в породе, %", clayey))
    {
        return "Нет данных";
    }

    const double remainder10 =
        greaterThan70 + from70To40 + from40To20 + from20To10;

    return gost8736ClassFromValues(
        module, residue063, remainder10, remainder5, less016, clayey);
}

std::string NormativeAnalysis::gost8736ClassFromValues(
    double module, double residue063, double remainder10, double remainder5,
    double less016, double clayey)
{
    if (!gost8736FullResidue063(module, residue063))
        return "Не соответствует";

    double class1Sieve10 = 0.0, class1Sieve5 = 0.0, class1Less016 = 0.0, class1Clayey = 0.0;
    double class2Sieve10 = 0.0, class2Sieve5 = 0.0, class2Less016 = 0.0, class2Clayey = 0.0;
    bool class1Sieve10Normalized = false;
    bool class1Sieve5Normalized = false;
    bool class1Less016Normalized = false;
    bool class2Sieve10Normalized = false;
    bool class2Sieve5Normalized = false;
    bool class2Less016Normalized = false;

    if (module > 3.0 && module <= 3.5)
    {
        class1Sieve10=0.5; class1Sieve5=5.0; class1Less016=5.0; class1Clayey=2.0;
        class2Sieve10=5.0; class2Sieve5=20.0; class2Less016=10.0; class2Clayey=3.0;
        class1Sieve10Normalized=class1Sieve5Normalized=class1Less016Normalized=true;
        class2Sieve10Normalized=class2Sieve5Normalized=class2Less016Normalized=true;
    }
    else if (module > 2.5 && module <= 3.0)
    {
        class1Sieve10=0.5; class1Sieve5=5.0; class1Less016=5.0; class1Clayey=2.0;
        class2Sieve10=5.0; class2Sieve5=15.0; class2Less016=15.0; class2Clayey=3.0;
        class1Sieve10Normalized=class1Sieve5Normalized=class1Less016Normalized=true;
        class2Sieve10Normalized=class2Sieve5Normalized=class2Less016Normalized=true;
    }
    else if (module > 2.0 && module <= 2.5)
    {
        class1Sieve10=0.5; class1Sieve5=5.0; class1Less016=5.0; class1Clayey=2.0;
        class2Sieve10=5.0; class2Sieve5=15.0; class2Less016=15.0; class2Clayey=3.0;
        class1Sieve10Normalized=class1Sieve5Normalized=class1Less016Normalized=true;
        class2Sieve10Normalized=class2Sieve5Normalized=class2Less016Normalized=true;
    }
    else if (module > 1.5 && module <= 2.0)
    {
        class1Sieve10=0.5; class1Sieve5=10.0; class1Less016=10.0; class1Clayey=3.0;
        class2Sieve10=0.5; class2Sieve5=10.0; class2Less016=20.0; class2Clayey=5.0;
        class1Sieve10Normalized=class1Sieve5Normalized=class1Less016Normalized=true;
        class2Sieve10Normalized=class2Sieve5Normalized=class2Less016Normalized=true;
    }
    else if (module > 1.0 && module <= 1.5)
    {
        class2Sieve10=0.5; class2Sieve5=10.0; class2Less016=20.0; class2Clayey=5.0;
        class2Sieve10Normalized=class2Sieve5Normalized=class2Less016Normalized=true;
    }
    else if (module > 0.7 && module <= 1.0)
    {
        class2Clayey=10.0;
    }
    else if (module >= 0.0 && module <= 0.7)
    {
        class2Clayey=10.0;
    }
    else
        return "Не соответствует";

    const auto met = [&](double s10, bool s10Norm,
                         double s5, bool s5Norm,
                         double l16, bool l16Norm,
                         double clayLimit)
    {
        return (!s10Norm || remainder10 <= s10 + EPS) &&
               (!s5Norm || remainder5 <= s5 + EPS) &&
               (!l16Norm || less016 <= l16 + EPS) &&
               clayey <= clayLimit + EPS;
    };

    if (class1Sieve10Normalized &&
        met(class1Sieve10, class1Sieve10Normalized,
            class1Sieve5, class1Sieve5Normalized,
            class1Less016, class1Less016Normalized, class1Clayey))
        return "I класс";

    if (met(class2Sieve10, class2Sieve10Normalized,
            class2Sieve5, class2Sieve5Normalized,
            class2Less016, class2Less016Normalized, class2Clayey))
        return "II класс";

    return "Не соответствует";
}

bool NormativeAnalysis::gost23735Values(double module, double gravel5, double less016, double clayey)
{
    const bool sandTypeOk =
        (module > 2.5 && module <= 3.0) ||
        (module > 2.0 && module <= 2.5) ||
        (module > 1.5 && module <= 2.0) ||
        (module > 1.0 && module <= 1.5);

    return gravel5 >= 10.0 - EPS && gravel5 <= 90.0 + EPS &&
           less016 <= 20.0 + EPS &&
           clayey <= 5.0 + EPS &&
           sandTypeOk;
}

std::string NormativeAnalysis::tkp1704372012(double gravel5)
{
    if (!std::isfinite(gravel5)) return {};
    if (gravel5 >= 7.0 - EPS && gravel5 <= 15.0 + EPS)
        return "Гравелистый песок";
    if (gravel5 > 15.0 && gravel5 <= 30.0 + EPS)
        return "Гравийно-песчаная смесь";
    if (gravel5 > 30.0)
        return "Песчано-гравийная смесь";
    return "Песок";
}

std::string NormativeAnalysis::gost31426(const Sample& sample)
{
    double sand = 0.0;
    double gravel5 = 0.0;
    double gravel20 = 0.0;
    double gravel40 = 0.0;
    double gravel70 = 0.0;
    double boulders = 0.0;

    if (!gost(sample, "Грансостав песка <5 мм", sand) ||
        !gost(sample, "Грансостав гравия 10-5 мм", gravel5) ||
        !gost(sample, "Грансостав гравия 20-10 мм", gravel20) ||
        !gost(sample, "Грансостав гравия 40-20 мм", gravel40) ||
        !gost(sample, "Грансостав гравия 70-40 мм", gravel70) ||
        !gost(sample, "Грансостав гравия >70 мм", boulders))
    {
        return "Нет данных";
    }

    const double gravel = gravel5 + gravel20 + gravel40 + gravel70;

    if (sand > 90.0 && sand <= 100.0 + EPS &&
        gravel <= 10.0 + EPS && boulders <= EPS)
    {
        return "Песчаная горная порода";
    }

    if (sand > 10.0 && sand <= 90.0 + EPS &&
        gravel > 10.0 && gravel <= 85.0 + EPS &&
        boulders <= 5.0 + EPS)
    {
        return "Гравийно-песчаная горная порода";
    }

    if (sand > 10.0 && sand <= 85.0 + EPS &&
        gravel > 10.0 && gravel <= 85.0 + EPS &&
        boulders > 5.0 && boulders <= 80.0 + EPS)
    {
        return "Валунно-гравийно-песчаная горная порода";
    }

    return "Не соответствует";
}

std::string NormativeAnalysis::gost23735(const Sample& sample)
{
    double gravel5 = 0.0;
    double less016 = 0.0;
    double clayey = 0.0;

    if (!gost(sample, "Грансостав гравия >5 мм", gravel5) ||
        !gost(sample, "Грансостав песка <0.16 мм", less016) ||
        !gost(sample, "Глинистые в породе, %", clayey))
    {
        return "Нет данных";
    }

    // По действующей для нашей таблицы проверке ГОСТ 23735:
    // - содержание гравия берём непосредственно из графы
    //   "гравий крупнее 5 мм";
    // - максимальную крупность гравия не учитываем;
    // - пылевидные и глинистые частицы берём из графы
    //   "Глинистые в породе, %";
    // - глину в комках не учитываем.
    // Дополнительно песчаная составляющая должна относиться
    // к нормируемым группам песка, а фракция <0.16 мм не должна
    // превышать 20 %.
    double module = 0.0;
    if (!gost(sample, "Модуль крупности песка", module))
        return "Нет данных";

    const auto sandType = gostSand(sample);
    const bool sandTypeOk =
        sandType == "Песок крупный" ||
        sandType == "Песок средний" ||
        sandType == "Песок мелкий" ||
        sandType == "Песок очень мелкий";

    const bool result =
        gost23735Values(module, gravel5, less016, clayey) && sandTypeOk;

    return result ? "Соответствует" : "Не соответствует";
}

std::string NormativeAnalysis::tkp682(const Sample& sample)
{
    double less005 = 0.0;

    if (!tkp(sample, "ТКП <0.05 мм", less005))
        return "Нет данных";

    const auto soilType = sample.getTkpSoilType();

    if (soilType.empty())
        return "Нет данных";

    const bool gravelly = contains(soilType, "гравелистый") ||
                          contains(soilType, "Гравелистый");
    const bool coarse = contains(soilType, "крупный") ||
                        contains(soilType, "Крупный");
    const bool medium = contains(soilType, "средний") ||
                        contains(soilType, "Средний");
    const bool fine = contains(soilType, "мелкий") ||
                      contains(soilType, "Мелкий");
    const bool dusty = contains(soilType, "пылеватый") ||
                       contains(soilType, "Пылеватый");

    const bool lightSandyLoam =
        contains(soilType, "супесь легкая песчанистая") ||
        contains(soilType, "Супесь легкая песчанистая");
    const bool dustySandyLoam =
        contains(soilType, "супесь пылеватая") ||
        contains(soilType, "Супесь пылеватая");
    const bool heavyDustyLoam =
        contains(soilType, "суглинок тяжелый пылеватый") ||
        contains(soilType, "Суглинок тяжелый пылеватый");
    const bool lightDustyLoam =
        contains(soilType, "суглинок легкий пылеватый") ||
        contains(soilType, "Суглинок легкий пылеватый");
    const bool lightSandyLoamSoil =
        contains(soilType, "суглинок легкий песчанистый") ||
        contains(soilType, "Суглинок легкий песчанистый");
    const bool heavySandyLoam =
        contains(soilType, "суглинок тяжелый песчанистый") ||
        contains(soilType, "Суглинок тяжелый песчанистый");
    const bool clay = contains(soilType, "глина") ||
                      contains(soilType, "Глина");

    if ((gravelly || coarse || medium) && less005 <= 2.0 + EPS)
        return "Непучинистый";

    if ((gravelly || coarse || medium) &&
        inRange(less005, 2.0, 15.0))
    {
        return "Слабопучинистый";
    }

    if (fine && less005 <= 5.0 + EPS)
        return "Слабопучинистый";

    if (lightSandyLoam)
        return "Слабопучинистый";

    if (fine && less005 <= 8.0 + EPS)
        return "Среднепучинистый";

    if (lightSandyLoamSoil || heavySandyLoam || clay)
        return "Среднепучинистый";

    if (fine && less005 <= 15.0 + EPS)
        return "Сильнопучинистый";

    if (dustySandyLoam || heavyDustyLoam || lightDustyLoam)
        return "Сильнопучинистый";

    if (dusty)
        return "Чрезмернопучинистый";

    return "Нет данных";
}

NormativeAnalysisResult NormativeAnalysis::analyze(const Sample& sample)
{
    return {
        gostSand(sample),
        spSand(sample),
        gostClass(sample),
        gost31426(sample),
        gost23735(sample),
        tkp682(sample)
    };
}
