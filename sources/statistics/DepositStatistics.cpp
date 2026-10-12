#include "../../headers/statistics/DepositStatistics.h"

#include "../../headers/calculations/MineralCalculationPart.h"
#include "../../headers/domain/Block.h"
#include "../../headers/domain/Borehole.h"
#include "../../headers/domain/DryMineralSection.h"
#include "../../headers/domain/WaterBearingMineralSection.h"
#include "../../headers/utils/SandClassification.h"
#include "../../headers/utils/ParameterValue.h"
#include "../../headers/writers/BlockDesignation.h"
#include "../../headers/settings/MainIndicatorsSettings.h"
#include "../../headers/settings/GroundwaterSettings.h"
#include "../../headers/settings/GroundwaterCalculationSettings.h"
#include "../../headers/settings/DevelopmentSettings.h"
#include "../../headers/settings/ReserveSummarySettings.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    constexpr double EPS = 1e-9;
    const std::string KF_PARAMETER = "Коэфф. фильтрации, м/сут";

    struct SampleRef
    {
        const Borehole* borehole = nullptr;
        const Sample* sample = nullptr;
    };

    struct Extreme
    {
        bool hasValue = false;
        double value = 0.0;
        std::string borehole;
        std::string sample;
    };

    struct TypeStats
    {
        std::size_t count = 0;
        Extreme minimum;
        Extreme maximum;
    };

    enum class Scope
    {
        All,
        Useful,
        DryUseful,
        WaterUseful
    };

    std::string formatNumber(double value, int precision = 2)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(precision) << value;
        return stream.str();
    }

    std::string percent(std::size_t value, std::size_t total)
    {
        if (total == 0)
            return "0,0 %";

        return formatNumber(100.0 * static_cast<double>(value) /
                            static_cast<double>(total), 1) + " %";
    }

    std::string countWithPercent(std::size_t value, std::size_t total)
    {
        return std::to_string(value) + " (" + percent(value, total) + ")";
    }

    std::string normalizeDecimal(std::string value)
    {
        std::replace(value.begin(), value.end(), '.', ',');
        return value;
    }

    std::string displayNumber(double value, int precision = 2)
    {
        return normalizeDecimal(formatNumber(value, precision));
    }

    std::string samplePosition(
        const Sample& sample,
        const BoreholeGeology& geology)
    {
        bool dry = false;
        bool water = false;

        for (const DryMineralSection& section :
             geology.getUsefulMineral().getDrySections())
        {
            if (std::min(sample.getEndDepth(), section.getBottomDepth()) >
                std::max(sample.getStartDepth(), section.getRoofDepth()) + EPS)
            {
                dry = true;
                break;
            }
        }

        for (const WaterBearingMineralSection& section :
             geology.getUsefulMineral().getWaterBearingSections())
        {
            if (std::min(sample.getEndDepth(), section.getBottomDepth()) >
                std::max(sample.getStartDepth(), section.getRoofDepth()) + EPS)
            {
                water = true;
                break;
            }
        }

        if (dry && water)
            return "both";
        if (dry)
            return "dry";
        if (water)
            return "water";
        return {};
    }

    bool overlapsUseful(
        const Sample& sample,
        const BoreholeGeology& geology,
        MineralCalculationPart part = MineralCalculationPart::TotalUsefulMineral)
    {
        if (sample.getThickness() <= EPS)
            return false;

        const bool checkDry =
            part != MineralCalculationPart::WaterBearingUsefulMineral;
        const bool checkWater =
            part != MineralCalculationPart::DryUsefulMineral;

        if (checkDry)
        {
            for (const DryMineralSection& section :
                 geology.getUsefulMineral().getDrySections())
            {
                const double start =
                    std::max(sample.getStartDepth(), section.getRoofDepth());
                const double end =
                    std::min(sample.getEndDepth(), section.getBottomDepth());

                if (end > start + EPS)
                    return true;
            }
        }

        if (checkWater)
        {
            for (const WaterBearingMineralSection& section :
                 geology.getUsefulMineral().getWaterBearingSections())
            {
                const double start =
                    std::max(sample.getStartDepth(), section.getRoofDepth());
                const double end =
                    std::min(sample.getEndDepth(), section.getBottomDepth());

                if (end > start + EPS)
                    return true;
            }
        }

        return false;
    }

    const BoreholeAnalysisResult* findResult(
        const GeologicalAnalysisResult& result,
        const std::string& boreholeNumber)
    {
        for (const BoreholeAnalysisResult& boreholeResult : result.getBoreholes())
        {
            if (boreholeResult.getBoreholeNumber() == boreholeNumber)
                return &boreholeResult;
        }

        return nullptr;
    }

    bool inScope(
        const SampleRef& ref,
        const GeologicalAnalysisResult& analysisResult,
        Scope scope)
    {
        if (scope == Scope::All)
            return true;

        const BoreholeAnalysisResult* result =
            findResult(analysisResult, ref.borehole->getBoreholeNumber());

        if (result == nullptr)
            return false;

        switch (scope)
        {
            case Scope::Useful:
                return overlapsUseful(*ref.sample, result->getGeology());

            case Scope::DryUseful:
                return overlapsUseful(
                    *ref.sample,
                    result->getGeology(),
                    MineralCalculationPart::DryUsefulMineral);

            case Scope::WaterUseful:
                return overlapsUseful(
                    *ref.sample,
                    result->getGeology(),
                    MineralCalculationPart::WaterBearingUsefulMineral);

            case Scope::All:
            default:
                return true;
        }
    }

    std::vector<SampleRef> collectSamples(
        const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult,
        Scope scope,
        const Block* block = nullptr)
    {
        std::vector<SampleRef> samples;

        auto addBorehole = [&](const Borehole* borehole)
        {
            if (borehole == nullptr)
                return;

            for (const Sample& sample : borehole->getSamples())
            {
                SampleRef ref{borehole, &sample};

                if (inScope(ref, analysisResult, scope))
                    samples.push_back(ref);
            }
        };

        if (block != nullptr)
        {
            for (const std::string& number : block->getBoreholeNumbers())
                addBorehole(deposit.findBorehole(number));
        }
        else
        {
            for (const Borehole& borehole : deposit.getBoreholes())
                addBorehole(&borehole);
        }

        return samples;
    }

    std::string gostName(GostSandClass sandClass)
    {
        return SandClassification::toString(sandClass);
    }

    std::string spName(TkpSandClass sandClass)
    {
        return SandClassification::toString(sandClass);
    }

    std::vector<std::pair<std::string, GostSandClass>> gostClasses()
    {
        return
        {
            {"Повышенной крупности", GostSandClass::IncreasedCoarseness},
            {"Крупный", GostSandClass::Coarse},
            {"Средний", GostSandClass::Medium},
            {"Мелкий", GostSandClass::Small},
            {"Очень мелкий", GostSandClass::VerySmall},
            {"Тонкий", GostSandClass::Fine},
            {"Очень тонкий", GostSandClass::VeryFine}
        };
    }

    std::vector<std::pair<std::string, TkpSandClass>> spClasses()
    {
        return
        {
            {"Гравелистый", TkpSandClass::Gravelly},
            {"Крупный", TkpSandClass::Coarse},
            {"Средний", TkpSandClass::Medium},
            {"Мелкий", TkpSandClass::Fine},
            {"Пылеватый", TkpSandClass::Dusty}
        };
    }

    std::size_t countGost(
        const std::vector<SampleRef>& samples,
        GostSandClass wanted)
    {
        std::size_t count = 0;

        for (const SampleRef& ref : samples)
        {
            if (ref.sample->getGostSandClass() == wanted)
                ++count;
        }

        return count;
    }

    std::size_t countSp(
        const std::vector<SampleRef>& samples,
        TkpSandClass wanted)
    {
        std::size_t count = 0;

        for (const SampleRef& ref : samples)
        {
            if (ref.sample->getTkpSandClass() == wanted)
                ++count;
        }

        return count;
    }

    std::size_t classifiedGostCount(const std::vector<SampleRef>& samples)
    {
        std::size_t count = 0;

        for (const SampleRef& ref : samples)
        {
            if (ref.sample->getGostSandClass() != GostSandClass::Unknown)
                ++count;
        }

        return count;
    }

    std::size_t classifiedSpCount(const std::vector<SampleRef>& samples)
    {
        std::size_t count = 0;

        for (const SampleRef& ref : samples)
        {
            if (ref.sample->getTkpSandClass() != TkpSandClass::Unknown)
                ++count;
        }

        return count;
    }

    std::string filterTypeForKf(const Sample& sample)
    {
        const TkpSandClass spClass = sample.getTkpSandClass();

        if (spClass != TkpSandClass::Unknown)
            return spName(spClass);

        const std::string rockType = sample.getRockType();

        if (!rockType.empty())
            return rockType;

        return "Не классифицировано";
    }

    void updateExtreme(
        Extreme& extreme,
        double value,
        const SampleRef& ref,
        bool minimum)
    {
        if (!std::isfinite(value))
            return;

        if (!extreme.hasValue ||
            (minimum && value < extreme.value) ||
            (!minimum && value > extreme.value))
        {
            extreme.hasValue = true;
            extreme.value = value;
            extreme.borehole = ref.borehole->getBoreholeNumber();
            extreme.sample = ref.sample->getSampleNumber();
        }
    }

    std::map<std::string, TypeStats> calculateKf(
        const std::vector<SampleRef>& samples)
    {
        std::map<std::string, TypeStats> result;

        for (const SampleRef& ref : samples)
        {
            const ParameterValue* parameter =
                ref.sample->findTKPParameter(KF_PARAMETER);

            if (parameter == nullptr)
                continue;

            const std::optional<double> value = getNumericValue(*parameter);

            if (!value.has_value() || !std::isfinite(*value))
                continue;

            const std::string type = filterTypeForKf(*ref.sample);
            TypeStats& stats = result[type];

            ++stats.count;
            updateExtreme(stats.minimum, *value, ref, true);
            updateExtreme(stats.maximum, *value, ref, false);
        }

        return result;
    }

    std::string extremeText(const Extreme& extreme)
    {
        if (!extreme.hasValue)
            return "нет данных";

        std::string text = displayNumber(extreme.value) +
                           " м/сут (скв. " + extreme.borehole;

        if (!extreme.sample.empty())
            text += ", обр. " + extreme.sample;

        text += ")";
        return text;
    }

    void writeDistribution(
        std::ostream& out,
        const std::vector<SampleRef>& allSamples,
        const std::vector<SampleRef>& usefulSamples,
        const std::string& title)
    {
        out << title << "\n";
        out << "  " << std::left << std::setw(25) << "Тип"
            << std::right << std::setw(18) << "Все образцы"
            << std::setw(18) << "Образцы ПИ" << "\n";
        out << "  " << std::string(61, '-') << "\n";

        if (title.find("ГОСТ") != std::string::npos)
        {
            for (const auto& [name, cls] : gostClasses())
            {
                const std::size_t allCount = countGost(allSamples, cls);
                const std::size_t usefulCount = countGost(usefulSamples, cls);

                out << "  " << std::left << std::setw(25) << name
                    << std::right << std::setw(18)
                    << countWithPercent(allCount, classifiedGostCount(allSamples))
                    << std::setw(18)
                    << countWithPercent(usefulCount, classifiedGostCount(usefulSamples))
                    << "\n";
            }

            out << "  Всего классифицировано: "
                << classifiedGostCount(allSamples) << " / "
                << classifiedGostCount(usefulSamples) << "\n\n";
        }
        else
        {
            for (const auto& [name, cls] : spClasses())
            {
                const std::size_t allCount = countSp(allSamples, cls);
                const std::size_t usefulCount = countSp(usefulSamples, cls);

                out << "  " << std::left << std::setw(25) << name
                    << std::right << std::setw(18)
                    << countWithPercent(allCount, classifiedSpCount(allSamples))
                    << std::setw(18)
                    << countWithPercent(usefulCount, classifiedSpCount(usefulSamples))
                    << "\n";
            }

            out << "  Всего классифицировано: "
                << classifiedSpCount(allSamples) << " / "
                << classifiedSpCount(usefulSamples) << "\n\n";
        }
    }

    void writeKf(
        std::ostream& out,
        const std::vector<SampleRef>& allSamples,
        const std::vector<SampleRef>& usefulSamples)
    {
        out << "Коэффициент фильтрации по типам пород\n";
        out << "  Значения приведены отдельно для всех образцов и образцов ПИ.\n";
        out << "  Минимум и максимум сопровождаются номером скважины и образца.\n\n";

        const auto all = calculateKf(allSamples);
        const auto useful = calculateKf(usefulSamples);

        std::set<std::string> types;
        for (const auto& [type, stats] : all)
            types.insert(type);
        for (const auto& [type, stats] : useful)
            types.insert(type);

        if (types.empty())
        {
            out << "  Нет числовых данных по коэффициенту фильтрации.\n\n";
            return;
        }

        for (const std::string& type : types)
        {
            out << "  " << type << ":\n";

            const auto allIt = all.find(type);
            const auto usefulIt = useful.find(type);

            if (allIt != all.end())
            {
                out << "    Все образцы: " << allIt->second.count << "\n"
                    << "      минимум — " << extremeText(allIt->second.minimum) << "\n"
                    << "      максимум — " << extremeText(allIt->second.maximum) << "\n";
            }
            else
            {
                out << "    Все образцы: нет данных\n";
            }

            if (usefulIt != useful.end())
            {
                out << "    Образцы ПИ: " << usefulIt->second.count << "\n"
                    << "      минимум — " << extremeText(usefulIt->second.minimum) << "\n"
                    << "      максимум — " << extremeText(usefulIt->second.maximum) << "\n";
            }
            else
            {
                out << "    Образцы ПИ: нет данных\n";
            }

            out << "\n";
        }
    }

    double totalThickness(
        const std::vector<SampleRef>& samples)
    {
        double value = 0.0;
        for (const SampleRef& ref : samples)
            value += ref.sample->getThickness();
        return value;
    }

    double usefulThickness(
        const BoreholeGeology& geology,
        MineralCalculationPart part)
    {
        if (part == MineralCalculationPart::DryUsefulMineral)
            return geology.getUsefulMineral().getDryThickness();

        if (part == MineralCalculationPart::WaterBearingUsefulMineral)
            return geology.getUsefulMineral().getWaterBearingThickness();

        return geology.getUsefulMineral().getTotalThickness();
    }

    std::size_t intersectionCount(
        const Deposit& deposit,
        const GeologicalAnalysisResult& result,
        const Block* block,
        MineralCalculationPart part)
    {
        std::size_t count = 0;

        auto check = [&](const Borehole* borehole)
        {
            if (borehole == nullptr)
                return;

            const BoreholeAnalysisResult* boreholeResult =
                findResult(result, borehole->getBoreholeNumber());

            if (boreholeResult == nullptr)
                return;

            if (usefulThickness(boreholeResult->getGeology(), part) > EPS)
                ++count;
        };

        if (block != nullptr)
        {
            std::set<std::string> unique;
            for (const std::string& number : block->getBoreholeNumbers())
            {
                if (unique.insert(number).second)
                    check(deposit.findBorehole(number));
            }
        }
        else
        {
            for (const Borehole& borehole : deposit.getBoreholes())
                check(&borehole);
        }

        return count;
    }

    struct ThicknessRange
    {
        bool hasValue = false;
        double minimum = 0.0;
        double maximum = 0.0;
        std::string minimumBorehole;
        std::string maximumBorehole;
    };

    ThicknessRange thicknessRange(
        const Deposit& deposit,
        const GeologicalAnalysisResult& result,
        const Block* block,
        MineralCalculationPart part)
    {
        ThicknessRange range;

        auto check = [&](const Borehole* borehole)
        {
            if (borehole == nullptr)
                return;

            const BoreholeAnalysisResult* boreholeResult =
                findResult(result, borehole->getBoreholeNumber());
            if (boreholeResult == nullptr)
                return;

            const double value = usefulThickness(boreholeResult->getGeology(), part);
            if (value <= EPS)
                return;

            if (!range.hasValue)
            {
                range.hasValue = true;
                range.minimum = value;
                range.maximum = value;
                range.minimumBorehole = borehole->getBoreholeNumber();
                range.maximumBorehole = borehole->getBoreholeNumber();
                return;
            }

            if (value < range.minimum)
            {
                range.minimum = value;
                range.minimumBorehole = borehole->getBoreholeNumber();
            }

            if (value > range.maximum)
            {
                range.maximum = value;
                range.maximumBorehole = borehole->getBoreholeNumber();
            }
        };

        if (block != nullptr)
        {
            std::set<std::string> unique;
            for (const std::string& number : block->getBoreholeNumbers())
            {
                if (unique.insert(number).second)
                    check(deposit.findBorehole(number));
            }
        }
        else
        {
            for (const Borehole& borehole : deposit.getBoreholes())
                check(&borehole);
        }

        return range;
    }

    double totalUsefulThickness(
        const Deposit& deposit,
        const GeologicalAnalysisResult& result,
        const Block* block,
        MineralCalculationPart part)
    {
        double total = 0.0;

        auto add = [&](const Borehole* borehole)
        {
            if (borehole == nullptr)
                return;

            const BoreholeAnalysisResult* r =
                findResult(result, borehole->getBoreholeNumber());
            if (r != nullptr)
                total += usefulThickness(r->getGeology(), part);
        };

        if (block != nullptr)
        {
            std::set<std::string> unique;
            for (const std::string& number : block->getBoreholeNumbers())
            {
                if (unique.insert(number).second)
                    add(deposit.findBorehole(number));
            }
        }
        else
        {
            for (const Borehole& borehole : deposit.getBoreholes())
                add(&borehole);
        }

        return total;
    }

    std::string thicknessSummary(
        const Deposit& deposit,
        const GeologicalAnalysisResult& result,
        const Block* block,
        MineralCalculationPart part)
    {
        const std::size_t intersections =
            intersectionCount(deposit, result, block, part);
        const double total =
            totalUsefulThickness(deposit, result, block, part);
        const ThicknessRange range =
            thicknessRange(deposit, result, block, part);

        std::ostringstream out;
        out << "  Пересечений: " << intersections << "\n";
        out << "  Суммарная мощность: " << displayNumber(total) << " м\n";

        if (range.hasValue)
        {
            out << "  Средняя мощность по пересечениям: "
                << displayNumber(total / static_cast<double>(intersections)) << " м\n";
            out << "  Минимальная мощность: " << displayNumber(range.minimum)
                << " м (скв. " << range.minimumBorehole << ")\n";
            out << "  Максимальная мощность: " << displayNumber(range.maximum)
                << " м (скв. " << range.maximumBorehole << ")\n";
        }
        else
        {
            out << "  Мощность: нет данных\n";
        }

        return out.str();
    }

    void writeBoreholeSummary(
        std::ostream& out,
        const Deposit& deposit,
        const GeologicalAnalysisResult& result,
        const Block& block)
    {
        out << "По скважинам:\n";
        out << "  " << std::left << std::setw(12) << "Скважина"
            << std::right << std::setw(14) << "ПИ, м"
            << std::setw(14) << "Сухое, м"
            << std::setw(16) << "Обводн., м"
            << std::setw(12) << "Образцов" << "\n";
        out << "  " << std::string(68, '-') << "\n";

        for (const std::string& number : block.getBoreholeNumbers())
        {
            const Borehole* borehole = deposit.findBorehole(number);
            const BoreholeAnalysisResult* r = findResult(result, number);

            if (borehole == nullptr || r == nullptr)
                continue;

            std::size_t samples = 0;
            for (const Sample& sample : borehole->getSamples())
            {
                if (overlapsUseful(sample, r->getGeology()))
                    ++samples;
            }

            const UsefulMineral& useful = r->getGeology().getUsefulMineral();

            out << "  " << std::left << std::setw(12) << number
                << std::right << std::setw(14) << displayNumber(useful.getTotalThickness())
                << std::setw(14) << displayNumber(useful.getDryThickness())
                << std::setw(16) << displayNumber(useful.getWaterBearingThickness())
                << std::setw(12) << samples << "\n";
        }

        out << "\n";
    }

    void writeScope(
        std::ostream& out,
        const Deposit& deposit,
        const GeologicalAnalysisResult& result,
        const Block* block,
        Scope scope,
        const std::string& title,
        bool includeKf = true)
    {
        const std::vector<SampleRef> samples =
            collectSamples(deposit, result, scope, block);

        out << title << "\n";
        out << "  Количество образцов: " << samples.size() << "\n";
        out << "  Суммарная мощность интервалов образцов: "
            << displayNumber(totalThickness(samples)) << " м\n\n";

        writeDistribution(
            out,
            scope == Scope::All ? samples : collectSamples(deposit, result, Scope::All, block),
            samples,
            "Гранулометрический состав по ГОСТ 8736-2025");

        // Для СП используется классификация из текущей лабораторной модели ТКП,
        // которая в GeoReport соответствует принятой классификации СП 5.01.04-2025.
        writeDistribution(
            out,
            scope == Scope::All ? samples : collectSamples(deposit, result, Scope::All, block),
            samples,
            "Гранулометрический состав по СП 5.01.04-2025");

        if (includeKf)
        {
            const std::vector<SampleRef> allSamples =
                collectSamples(deposit, result, Scope::All, block);
            writeKf(out, allSamples, samples);
        }
    }

    void writeDepositOverview(
        std::ostream& out,
        const Deposit& deposit,
        const GeologicalAnalysisResult& result)
    {
        const std::vector<SampleRef> all =
            collectSamples(deposit, result, Scope::All);
        const std::vector<SampleRef> useful =
            collectSamples(deposit, result, Scope::Useful);

        out << "============================================================\n";
        out << "ОБЩАЯ СТАТИСТИКА ПО МЕСТОРОЖДЕНИЮ\n";
        out << "============================================================\n\n";
        out << "Блоков: " << deposit.getBlocks().size() << "\n";
        out << "Скважин: " << deposit.getBoreholes().size() << "\n";
        out << "Всего образцов: " << all.size() << "\n";
        out << "Образцов, отнесённых к ПИ: " << useful.size() << "\n";
        out << "Образцов с ГОСТ-классификацией: " << classifiedGostCount(all) << "\n";
        out << "Образцов ПИ с ГОСТ-классификацией: " << classifiedGostCount(useful) << "\n";
        out << "Образцов с классификацией СП: " << classifiedSpCount(all) << "\n";
        out << "Образцов ПИ с классификацией СП: " << classifiedSpCount(useful) << "\n\n";

        out << "Полезное ископаемое:\n";
        out << thicknessSummary(
            deposit, result, nullptr, MineralCalculationPart::TotalUsefulMineral);
        out << "\n";
        out << "Сухое полезное ископаемое:\n";
        out << thicknessSummary(
            deposit, result, nullptr, MineralCalculationPart::DryUsefulMineral);
        out << "\n";
        out << "Обводнённое полезное ископаемое:\n";
        out << thicknessSummary(
            deposit, result, nullptr, MineralCalculationPart::WaterBearingUsefulMineral);
        out << "\n";

        writeDistribution(out, all, useful,
            "Гранулометрический состав по ГОСТ 8736-2025");
        writeDistribution(out, all, useful,
            "Гранулометрический состав по СП 5.01.04-2025");
        writeKf(out, all, useful);
    }

    void writeBlock(
        std::ostream& out,
        const Deposit& deposit,
        const GeologicalAnalysisResult& result,
        const Block& block)
    {
        const std::vector<SampleRef> all =
            collectSamples(deposit, result, Scope::All, &block);
        const std::vector<SampleRef> useful =
            collectSamples(deposit, result, Scope::Useful, &block);

        out << "============================================================\n";
        out << "БЛОК " << block.getName() << "\n";
        out << "============================================================\n\n";
        out << "Скважин в блоке: " << block.getBoreholeNumbers().size() << "\n";
        out << "Всего образцов: " << all.size() << "\n";
        out << "Образцов, отнесённых к ПИ: " << useful.size() << "\n\n";

        out << "ПОЛЕЗНОЕ ИСКОПАЕМОЕ\n";
        out << thicknessSummary(
            deposit, result, &block, MineralCalculationPart::TotalUsefulMineral) << "\n";

        out << "СУХОЕ ПОЛЕЗНОЕ ИСКОПАЕМОЕ\n";
        out << thicknessSummary(
            deposit, result, &block, MineralCalculationPart::DryUsefulMineral) << "\n";
        writeScope(out, deposit, result, &block, Scope::DryUseful,
                   "Статистика образцов сухого ПИ", true);
        out << "\n";

        out << "ОБВОДНЁННОЕ ПОЛЕЗНОЕ ИСКОПАЕМОЕ\n";
        out << thicknessSummary(
            deposit, result, &block, MineralCalculationPart::WaterBearingUsefulMineral) << "\n";
        writeScope(out, deposit, result, &block, Scope::WaterUseful,
                   "Статистика образцов обводнённого ПИ", true);
        out << "\n";

        out << "ОБЩАЯ СТАТИСТИКА ОБРАЗЦОВ БЛОКА\n";
        writeDistribution(out, all, useful,
            "Гранулометрический состав по ГОСТ 8736-2025");
        writeDistribution(out, all, useful,
            "Гранулометрический состав по СП 5.01.04-2025");
        writeKf(out, all, useful);

        writeBoreholeSummary(out, deposit, result, block);
    }


    std::string yesNo(bool value)
    {
        return value ? "да" : "нет";
    }

    std::string joinTypes(const std::vector<std::string>& values)
    {
        if (values.empty())
            return "не заданы";
        std::ostringstream out;
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if (i != 0) out << ", ";
            out << values[i];
        }
        return out.str();
    }

    std::string classificationSourceName(SandClassificationSource source)
    {
        switch (source)
        {
            case SandClassificationSource::Gost: return "ГОСТ";
            case SandClassificationSource::Tkp: return "ТКП / СП";
            case SandClassificationSource::GostAndTkp: return "ГОСТ и ТКП / СП";
        }
        return "не определён";
    }

    void writeSettingsSection(
        std::ostream& out,
        const MainIndicatorsSettings& mineral,
        const GroundwaterSettings& groundwater,
        const GroundwaterCalculationSettings& groundwaterCalculation,
        const DevelopmentSettings& development,
        const ReserveSummarySettings& reserve)
    {
        out << "============================================================\\n";
        out << "НАСТРОЙКИ РАСЧЁТА, ИСПОЛЬЗОВАННЫЕ ПРИ ЗАПУСКЕ\\n";
        out << "============================================================\\n\\n";
        out << "Полезное ископаемое и исключаемые породы:\\n";
        out << "  Типы пород, заданные как полезные: " << joinTypes(mineral.getUsefulRockTypes()) << "\\n";
        out << "  Песок включать в ПИ: " << yesNo(mineral.isSandUseful()) << "\\n";
        out << "  Супесь включать в ПИ: " << yesNo(mineral.isSupesUseful()) << "\\n";
        out << "  Суглинок включать в ПИ: " << yesNo(mineral.isSuglinokUseful()) << "\\n";
        out << "  Глину включать в ПИ: " << yesNo(mineral.isClayUseful()) << "\\n";
        out << "  Типы пород внешней вскрыши: " << joinTypes(mineral.getOverburdenRockTypes()) << "\\n";
        out << "  Типы пород подстилающего слоя: " << joinTypes(mineral.getUnderlyingRockTypes()) << "\\n";
        out << "  Минимальная мощность внешней вскрыши: " << displayNumber(mineral.getMinimumOverburdenThickness()) << " м\\n";
        out << "  Режим вскрыши: " << (mineral.getOverburdenMode() == OverburdenMode::FixedThickness
            ? "фиксированная мощность" : "по типу породы") << "\\n";
        out << "  Фиксированная мощность вскрыши: " << displayNumber(mineral.getFixedOverburdenThickness()) << " м\\n";
        out << "  Источник классификации песка для ПИ: "
            << classificationSourceName(mineral.getSandClassificationSource()) << "\\n";

        const GostSandClass gostClasses[] = {
            GostSandClass::VeryFine, GostSandClass::Fine, GostSandClass::VerySmall,
            GostSandClass::Small, GostSandClass::Medium, GostSandClass::Coarse,
            GostSandClass::IncreasedCoarseness
        };
        std::vector<std::string> includedGost;
        for (GostSandClass value : gostClasses)
            if (mineral.isGostSandClassUseful(value))
                includedGost.push_back(SandClassification::toString(value));
        out << "  Классы песка ГОСТ, включённые в ПИ: " << joinTypes(includedGost) << "\\n";

        const TkpSandClass tkpClasses[] = {
            TkpSandClass::Dusty, TkpSandClass::Fine, TkpSandClass::Medium,
            TkpSandClass::Coarse, TkpSandClass::Gravelly
        };
        std::vector<std::string> includedTkp;
        for (TkpSandClass value : tkpClasses)
            if (mineral.isTkpSandClassUseful(value))
                includedTkp.push_back(SandClassification::toString(value));
        out << "  Классы СП / ТКП, включённые в ПИ: " << joinTypes(includedTkp) << "\\n";

        out << "\\nУровень грунтовых вод и разработка:\\n";
        out << "  Учитывать только сухое ПИ: "
            << yesNo(groundwater.getUsefulMineralWaterMode() == UsefulMineralWaterMode::DryOnly) << "\\n";
        out << "  Сухой защитный слой над УГВ: " << displayNumber(groundwater.getDryLayerAboveGroundwater()) << " м\\n";
        out << "  Режим расчёта УГВ: ";
        switch (groundwaterCalculation.getMode())
        {
            case GroundwaterCalculationMode::Automatic: out << "автоматический"; break;
            case GroundwaterCalculationMode::MeanLevel: out << "средний уровень"; break;
            case GroundwaterCalculationMode::IndividualLevel: out << "индивидуальный уровень"; break;
        }
        out << "\\n  Порог доли скважин со вскрытым УГВ: "
            << displayNumber(groundwaterCalculation.getMajorityThreshold() * 100.0, 1) << " %\\n";
        out << "  Способ разработки: ";
        switch (development.getMethod())
        {
            case DevelopmentMethod::Custom: out << "пользовательский"; break;
            case DevelopmentMethod::Excavator: out << "экскаваторный"; break;
            case DevelopmentMethod::Dredger: out << "земснаряд"; break;
        }
        out << "\\n  Предельная мощность обводнённого ПИ: "
            << displayNumber(development.getWaterBearingMiningThickness()) << " м\\n";
        out << "  Сухая защитная подушка при разработке: "
            << displayNumber(development.getDryProtectiveLayerThickness()) << " м\\n";
        out << "  Метод определения площади блоков: "
            << (reserve.getAreaMode() == BlockAreaMode::ManualBoundary
                ? "по заданной границе" : "автоматическая выпуклая оболочка") << "\\n";

        out << "\\nПравила обработки проб:\\n";
        out << "  Монолиты с номером, начинающимся на «м», не считаются обычными пробами при определении ПРС.\\n";
        out << "  Образцы с суффиксом «аг» не считаются обычными пробами при определении ПРС.\\n";
        out << "  ПРС определяется по кровле первой обычной пробы; фиксированный интервал 0,00–0,50 м не предполагается.\\n";
        out << "  Категория распознаётся по суффиксу обозначения блока: A, B, C1, C2, P1, P2.\\n";
        out << "  A+B+C1 — промышленные запасы; C2 — забалансовые запасы; P1 и P2 — прогнозные ресурсы.\\n\\n";
    }

    std::string blockReserveCategory(const Block& block)
    {
        return BlockDesignation::parse(block.getName()).second;
    }

    void writeReserveCategoryGroups(
        std::ostream& out,
        const Deposit& deposit,
        const GeologicalAnalysisResult& result)
    {
        const auto writeCategories = [&](const std::vector<std::string>& categories,
                                         const std::string& heading,
                                         bool showForecastBreakdown)
        {
            out << "============================================================\\n"
                << heading << "\\n"
                << "============================================================\\n\\n";
            bool foundAny = false;

            for (const std::string& category : categories)
            {
                bool foundCategory = false;
                if (showForecastBreakdown)
                    out << "КАТЕГОРИЯ " << category << "\\n";

                for (const Block& block : deposit.getBlocks())
                {
                    if (blockReserveCategory(block) != category)
                        continue;
                    foundAny = true;
                    foundCategory = true;
                    writeBlock(out, deposit, result, block);
                    out << "\\n";
                }

                if (showForecastBreakdown && !foundCategory)
                    out << "  Блоки категории " << category << " не найдены.\\n\\n";
            }

            if (!foundAny)
                out << "Блоки этой группы в исходных данных не найдены.\\n\\n";
        };

        writeCategories({"A", "B", "C1"},
                        "ПРОМЫШЛЕННЫЕ ЗАПАСЫ (A + B + C1)", false);
        writeCategories({"C2"},
                        "ЗАБАЛАНСОВЫЕ ЗАПАСЫ (C2)", false);
        writeCategories({"P1", "P2"},
                        "ПРОГНОЗНЫЕ РЕСУРСЫ (P1 + P2)", true);

        bool haveUnclassified = false;
        for (const Block& block : deposit.getBlocks())
        {
            if (!blockReserveCategory(block).empty())
                continue;
            if (!haveUnclassified)
            {
                out << "============================================================\\n"
                    << "БЛОКИ БЕЗ РАСПОЗНАННОЙ КАТЕГОРИИ\\n"
                    << "============================================================\\n\\n";
                haveUnclassified = true;
            }
            writeBlock(out, deposit, result, block);
            out << "\\n";
        }
    }

    void writeDataQuality(
        std::ostream& out,
        const Deposit& deposit,
        const GeologicalAnalysisResult& result)
    {
        std::size_t missingCoordinates = 0;
        std::size_t missingElevation = 0;
        std::size_t missingKf = 0;
        std::size_t unclassifiedGost = 0;
        std::size_t unclassifiedSp = 0;
        std::size_t boreholesWithoutMineral = 0;

        const auto all = collectSamples(deposit, result, Scope::All);

        for (const Borehole& borehole : deposit.getBoreholes())
        {
            const Coordinate coordinate = borehole.getCoordinate();
            if (std::abs(coordinate.first) < EPS &&
                std::abs(coordinate.second) < EPS)
                ++missingCoordinates;

            if (std::abs(borehole.getCollarElevation()) < EPS)
                ++missingElevation;

            const BoreholeAnalysisResult* r =
                findResult(result, borehole.getBoreholeNumber());
            if (r == nullptr ||
                r->getIndicators().getUsefulMineralThickness() <= EPS)
                ++boreholesWithoutMineral;
        }

        for (const SampleRef& ref : all)
        {
            if (ref.sample->getGostSandClass() == GostSandClass::Unknown)
                ++unclassifiedGost;

            if (ref.sample->getTkpSandClass() == TkpSandClass::Unknown)
                ++unclassifiedSp;

            const ParameterValue* kf =
                ref.sample->findTKPParameter(KF_PARAMETER);
            if (kf == nullptr || !getNumericValue(*kf).has_value())
                ++missingKf;
        }

        out << "============================================================\n";
        out << "КОНТРОЛЬ ПОЛНОТЫ ДАННЫХ\n";
        out << "============================================================\n\n";
        out << "Скважин без координат: " << missingCoordinates << "\n";
        out << "Скважин без отметки устья: " << missingElevation << "\n";
        out << "Скважин без принятого ПИ: " << boreholesWithoutMineral << "\n";
        out << "Образцов без классификации ГОСТ: " << unclassifiedGost << "\n";
        out << "Образцов без классификации СП: " << unclassifiedSp << "\n";
        out << "Образцов без числового коэффициента фильтрации: " << missingKf << "\n";
    }
}

void DepositStatistics::write(
    const std::filesystem::path& outputPath,
    const Deposit& deposit,
    const GeologicalAnalysisResult& analysisResult,
    const MainIndicatorsSettings& mainIndicatorsSettings,
    const GroundwaterSettings& groundwaterSettings,
    const GroundwaterCalculationSettings& groundwaterCalculationSettings,
    const DevelopmentSettings& developmentSettings,
    const ReserveSummarySettings& reserveSummarySettings) const
{
    std::ofstream output(outputPath);
    if (!output.is_open())
        throw std::runtime_error(
            "Не удалось открыть файл статистики: " + outputPath.string());

    output << "СТАТИСТИКА МЕСТОРОЖДЕНИЯ\\n";
    output << "GeoReport\\n\\n";
    writeSettingsSection(output, mainIndicatorsSettings, groundwaterSettings,
                         groundwaterCalculationSettings, developmentSettings,
                         reserveSummarySettings);

    output << "Примечание: статистика по образцам приведена параллельно для всех образцов\\n";
    output << "и для образцов, которые по геологической модели отнесены к полезному ископаемому.\\n";
    output << "Для сухого и обводнённого ПИ используются отдельные выборки.\\n";
    output << "Разделение образцов выполняется по фактическому пересечению интервала\\n";
    output << "образца с соответствующей частью геологической модели.\\n\\n";

    writeDepositOverview(output, deposit, analysisResult);
    output << "\\n";
    writeReserveCategoryGroups(output, deposit, analysisResult);
    writeDataQuality(output, deposit, analysisResult);
}
