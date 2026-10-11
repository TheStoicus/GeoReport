#pragma once

#include "../domain/Sample.h"

#include <set>
#include <string>
#include <vector>

/*
 * Способ определения внешней вскрыши.
 */
enum class OverburdenMode
{
    FixedThickness,
    ByRockType
};

/*
 * Источник классификации песка.
 * Для строительных материалов рекомендуется ГОСТ.
 */
enum class SandClassificationSource
{
    Gost,
    Tkp,
    GostAndTkp
};

/*
 * Как поступать со специальной поверхностной пробой 0.00–0.50 м.
 */
enum class SurfaceSampleMode
{
    Exclude,
    UsefulMineral,
    ExternalOverburden
};

class MainIndicatorsSettings
{
public:
    // -------------------- Внешняя вскрыша --------------------
    void setOverburdenMode(OverburdenMode mode);
    OverburdenMode getOverburdenMode() const;

    void setFixedOverburdenThickness(double thickness);
    double getFixedOverburdenThickness() const;
    bool isFixedOverburdenEnabled() const;

    void setMinimumOverburdenThickness(double thickness);
    double getMinimumOverburdenThickness() const;

    void setOverburdenRockTypes(const std::vector<std::string>& rockTypes);
    const std::vector<std::string>& getOverburdenRockTypes() const;
    bool isOverburdenRockType(const std::string& rockType) const;
    void setOverburdenSandClassificationSource(SandClassificationSource source);
    SandClassificationSource getOverburdenSandClassificationSource() const;
    void setOverburdenGostSandClassUseful(GostSandClass sandClass, bool value);
    void setOverburdenTkpSandClassUseful(TkpSandClass sandClass, bool value);
    bool isOverburdenSample(const Sample& sample) const;

    // -------------------- Полезное ископаемое --------------------
    void setUsefulRockTypes(const std::vector<std::string>& rockTypes);
    const std::vector<std::string>& getUsefulRockTypes() const;
    bool isUsefulRockType(const std::string& rockType) const;

    void setSandUseful(bool value);
    bool isSandUseful() const;

    void setSupesUseful(bool value);
    bool isSupesUseful() const;

    void setSuglinokUseful(bool value);
    bool isSuglinokUseful() const;

    void setClayUseful(bool value);
    bool isClayUseful() const;

    void setSandClassificationSource(SandClassificationSource source);
    SandClassificationSource getSandClassificationSource() const;

    void setGostSandClassUseful(GostSandClass sandClass, bool value);
    bool isGostSandClassUseful(GostSandClass sandClass) const;

    void setTkpSandClassUseful(TkpSandClass sandClass, bool value);
    bool isTkpSandClassUseful(TkpSandClass sandClass) const;

    // -------------------- Явное назначение подстилающих пород --------------------
    /*
     * Обычно подстилающая порода определяется автоматически по положению
     * ниже последнего полезного интервала. Этот список позволяет пользователю
     * принудительно исключить отдельный тип из ПИ и считать его подстилающим.
     */
    void setUnderlyingRockTypes(const std::vector<std::string>& rockTypes);
    const std::vector<std::string>& getUnderlyingRockTypes() const;
    bool isUnderlyingRockType(const std::string& rockType) const;
    void setUnderlyingSandClassificationSource(SandClassificationSource source);
    SandClassificationSource getUnderlyingSandClassificationSource() const;
    void setUnderlyingGostSandClassUseful(GostSandClass sandClass, bool value);
    void setUnderlyingTkpSandClassUseful(TkpSandClass sandClass, bool value);
    bool isUnderlyingSample(const Sample& sample) const;

    // -------------------- Поверхностная проба --------------------
    void setSurfaceSampleMode(SurfaceSampleMode mode);
    SurfaceSampleMode getSurfaceSampleMode() const;

    // -------------------- Общая классификация образца --------------------
    bool isUseful(const Sample& sample) const;

private:
    static std::string trim(const std::string& value);
    static bool sameRockType(const std::string& left, const std::string& right);

    static void replaceTypes(
        std::vector<std::string>& destination,
        const std::vector<std::string>& source);

    static bool containsType(
        const std::vector<std::string>& types,
        const std::string& value);

private:
    OverburdenMode overburdenMode = OverburdenMode::FixedThickness;
    double fixedOverburdenThickness = 0.0;
    double minimumOverburdenThickness = 0.8;
    std::vector<std::string> overburdenRockTypes;
    SandClassificationSource overburdenSandClassificationSource = SandClassificationSource::Gost;
    std::set<GostSandClass> overburdenGostSandClasses;
    std::set<TkpSandClass> overburdenTkpSandClasses;

    std::vector<std::string> usefulRockTypes;

    bool sandUseful = true;
    bool supesUseful = false;
    bool suglinokUseful = false;
    bool clayUseful = false;

    SandClassificationSource sandClassificationSource = SandClassificationSource::Gost;
    std::set<GostSandClass> usefulGostSandClasses;
    std::set<TkpSandClass> usefulTkpSandClasses;

    std::vector<std::string> underlyingRockTypes;
    SandClassificationSource underlyingSandClassificationSource = SandClassificationSource::Gost;
    std::set<GostSandClass> underlyingGostSandClasses;
    std::set<TkpSandClass> underlyingTkpSandClasses;

    SurfaceSampleMode surfaceSampleMode = SurfaceSampleMode::Exclude;
};
