#include "../../headers/settings/ConsoleSettingsReader.h"

#include <algorithm>
#include <utility>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    int readInt(const std::string& prompt, int minValue, int maxValue)
    {
        for (;;)
        {
            std::cout << prompt;
            int value = 0;
            if (std::cin >> value && value >= minValue && value <= maxValue)
            {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            }

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Некорректный ввод. Повторите.\n";
        }
    }

    double readDouble(const std::string& prompt, double minValue, double maxValue)
    {
        for (;;)
        {
            std::cout << prompt;
            double value = 0.0;
            if (std::cin >> value && value >= minValue && value <= maxValue)
            {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            }

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Некорректное числовое значение. Повторите.\n";
        }
    }

    bool readYesNo(const std::string& prompt)
    {
        return readInt(prompt + " [1=да, 0=нет]: ", 0, 1) == 1;
    }


    std::vector<std::string> readOrderedBoreholes(
        const Block& block)
    {
        const std::vector<std::string>& available =
            block.getBoreholeNumbers();

        for (;;)
        {
            std::cout << "\nСкважины блока " << block.getName()
                      << ", доступные для контура: \n";

            for (const std::string& number : available)
                std::cout << "  " << number << ' ';

            std::cout << "\nВведите номера крайних скважин в порядке обхода контура\n"
                      << "(например: 1 5 8 12 10 3): ";

            std::string line;
            std::getline(std::cin, line);

            std::stringstream input(line);
            std::vector<std::string> selected;
            std::set<std::string> unique;
            std::string value;

            while (input >> value)
            {
                const auto it =
                    std::find(available.begin(), available.end(), value);

                if (it == available.end())
                {
                    selected.clear();
                    break;
                }

                if (!unique.insert(value).second)
                {
                    selected.clear();
                    break;
                }

                selected.push_back(value);
            }

            if (selected.size() >= 3)
                return selected;

            std::cout
                << "Нужно указать не менее трёх различных скважин "
                << "из списка блока. Повторите ввод.\n";
        }
    }
    std::vector<std::size_t> readIndexes(const std::string& prompt, std::size_t count)
    {
        std::cout << prompt;

        std::string line;
        std::getline(std::cin, line);

        std::stringstream input(line);
        std::set<std::size_t> indexes;
        std::size_t index = 0;

        while (input >> index)
        {
            if (index >= 1 && index <= count)
                indexes.insert(index);
        }

        return {indexes.begin(), indexes.end()};
    }

    std::string sandGostLabel(GostSandClass value)
    {
        return "Песок — " + SandClassification::toString(value) + " (ГОСТ)";
    }

    std::string sandTkpLabel(TkpSandClass value)
    {
        return "Песок — " + SandClassification::toString(value) + " (ТКП)";
    }

    template <typename GostSetter, typename TkpSetter>
    void readSandSelection(
        const std::string& title,
        int source,
        GostSetter gostSetter,
        TkpSetter tkpSetter)
    {
        struct Choice
        {
            int source = 0;
            GostSandClass gost = GostSandClass::Unknown;
            TkpSandClass tkp = TkpSandClass::Unknown;
            std::string label;
        };

        std::cout << "\n--- " << title << ": КЛАССЫ ПЕСКА ---\n";

        std::vector<Choice> choices;

        if (source == 1 || source == 3)
        {
            const GostSandClass classes[] =
            {
                GostSandClass::VeryFine,
                GostSandClass::Fine,
                GostSandClass::VerySmall,
                GostSandClass::Small,
                GostSandClass::Medium,
                GostSandClass::Coarse,
                GostSandClass::IncreasedCoarseness
            };

            for (const GostSandClass value : classes)
                choices.push_back({1, value, TkpSandClass::Unknown, sandGostLabel(value)});
        }

        if (source == 2 || source == 3)
        {
            const TkpSandClass classes[] =
            {
                TkpSandClass::Dusty,
                TkpSandClass::Fine,
                TkpSandClass::Medium,
                TkpSandClass::Coarse,
                TkpSandClass::Gravelly
            };

            for (const TkpSandClass value : classes)
                choices.push_back({2, GostSandClass::Unknown, value, sandTkpLabel(value)});
        }

        for (std::size_t i = 0; i < choices.size(); ++i)
            std::cout << "  " << i + 1 << " — " << choices[i].label << '\n';

        const auto selected = readIndexes(
            "Выберите классы песка через пробел: ",
            choices.size());

        for (const std::size_t index : selected)
        {
            const Choice& choice = choices[index - 1];

            if (choice.source == 1)
                gostSetter(choice.gost);
            else
                tkpSetter(choice.tkp);
        }
    }
}

MainIndicatorsSettings ConsoleSettingsReader::read(const Deposit& deposit) const
{
    MainIndicatorsSettings settings;

    std::cout << "\n==============================================\n"
              << "       НАСТРОЙКА ГЕОЛОГИЧЕСКОЙ МОДЕЛИ\n"
              << "==============================================\n";

    readOverburdenSettings(deposit, settings);
    readUsefulMineralSettings(deposit, settings);
    readUnderlyingRockSettings(deposit, settings);
    readSurfaceSampleSettings(settings);

    return settings;
}

void ConsoleSettingsReader::readOverburdenSettings(
    const Deposit& deposit,
    MainIndicatorsSettings& settings) const
{
    std::cout << "\n--- ВСКРЫШНЫЕ ПОРОДЫ ---\n"
              << "1 — фиксированная мощность от поверхности\n"
              << "2 — по выбранным типам пород\n";

    const int mode = readInt("Способ определения вскрыши: ", 1, 2);

    if (mode == 1)
    {
        settings.setOverburdenMode(OverburdenMode::FixedThickness);
        settings.setFixedOverburdenThickness(
            readDouble("Мощность внешней вскрыши, м: ", 0.0, 100000.0));
        return;
    }

    settings.setOverburdenMode(OverburdenMode::ByRockType);
    readMinimumOverburdenThickness(settings);

    const auto rockTypes = collectRockTypes(deposit);
    printRockTypes(rockTypes);

    if (!rockTypes.empty())
    {
        const auto selected = readSelectedRockTypes(rockTypes);
        settings.setOverburdenRockTypes(selected);

        if (std::find(selected.begin(), selected.end(), "Песок") != selected.end())
        {
            std::cout << "\nИсточник классификации песка для вскрышных пород:\n"
                      << "1 — ГОСТ\n"
                      << "2 — ТКП\n"
                      << "3 — ГОСТ или ТКП\n";

            const int source = readInt("Источник классификации: ", 1, 3);
            settings.setOverburdenSandClassificationSource(
                source == 1 ? SandClassificationSource::Gost :
                source == 2 ? SandClassificationSource::Tkp :
                               SandClassificationSource::GostAndTkp);

            readSandSelection(
                "ВСКРЫШНЫЕ ПОРОДЫ",
                source,
                [&settings](GostSandClass value)
                { settings.setOverburdenGostSandClassUseful(value, true); },
                [&settings](TkpSandClass value)
                { settings.setOverburdenTkpSandClassUseful(value, true); });
        }
    }
}

void ConsoleSettingsReader::readMinimumOverburdenThickness(
    MainIndicatorsSettings& settings) const
{
    std::cout << "\nМинимальная внешняя вскрыша при непосредственном залегании ПИ под ПРС.\n"
              << "Введите 0, чтобы не применять это правило.\n";

    settings.setMinimumOverburdenThickness(
        readDouble("Минимальная мощность вскрыши, м: ", 0.0, 1000.0));
}

void ConsoleSettingsReader::readUsefulMineralSettings(
    const Deposit& deposit,
    MainIndicatorsSettings& settings) const
{
    std::cout << "\n--- ПОЛЕЗНОЕ ИСКОПАЕМОЕ ---\n"
              << "Для песка выбирается конкретный класс, а не общий пункт 'Песок'.\n";

    std::cout << "\nИсточник классификации песка:\n"
              << "1 — ГОСТ\n"
              << "2 — ТКП\n"
              << "3 — ГОСТ или ТКП\n";

    const int source = readInt("Источник классификации: ", 1, 3);

    if (source == 1)
        settings.setSandClassificationSource(SandClassificationSource::Gost);
    else if (source == 2)
        settings.setSandClassificationSource(SandClassificationSource::Tkp);
    else
        settings.setSandClassificationSource(SandClassificationSource::GostAndTkp);

    struct Choice
    {
        std::string label;
        enum class Kind
        {
            GostSand,
            TkpSand,
            Supes,
            Suglinok,
            Clay,
            Other
        } kind;
        GostSandClass gost = GostSandClass::Unknown;
        TkpSandClass tkp = TkpSandClass::Unknown;
        std::string rockType;
    };

    std::vector<Choice> choices;

    if (source == 1 || source == 3)
    {
        const GostSandClass sandClasses[] =
        {
            GostSandClass::VeryFine,
            GostSandClass::Fine,
            GostSandClass::VerySmall,
            GostSandClass::Small,
            GostSandClass::Medium,
            GostSandClass::Coarse,
            GostSandClass::IncreasedCoarseness
        };

        for (const GostSandClass value : sandClasses)
            choices.push_back({sandGostLabel(value), Choice::Kind::GostSand, value, TkpSandClass::Unknown, {}});
    }

    if (source == 2 || source == 3)
    {
        const TkpSandClass sandClasses[] =
        {
            TkpSandClass::Dusty,
            TkpSandClass::Fine,
            TkpSandClass::Medium,
            TkpSandClass::Coarse,
            TkpSandClass::Gravelly
        };

        for (const TkpSandClass value : sandClasses)
            choices.push_back({sandTkpLabel(value), Choice::Kind::TkpSand, GostSandClass::Unknown, value, {}});
    }

    choices.push_back({"Супесь", Choice::Kind::Supes, GostSandClass::Unknown, TkpSandClass::Unknown, "Супесь"});
    choices.push_back({"Суглинок", Choice::Kind::Suglinok, GostSandClass::Unknown, TkpSandClass::Unknown, "Суглинок"});
    choices.push_back({"Глина", Choice::Kind::Clay, GostSandClass::Unknown, TkpSandClass::Unknown, "Глина"});

    // Добавляем прочие реальные типы, которые встретились в Excel.
    const auto actualRockTypes = collectRockTypes(deposit);
    for (const std::string& rockType : actualRockTypes)
    {
        if (rockType == "Песок" ||
            rockType == "Супесь" ||
            rockType == "Суглинок" ||
            rockType == "Глина")
        {
            continue;
        }

        choices.push_back({rockType, Choice::Kind::Other, GostSandClass::Unknown, TkpSandClass::Unknown, rockType});
    }

    std::cout << "\nДоступные типы полезного ископаемого:\n";
    for (std::size_t i = 0; i < choices.size(); ++i)
        std::cout << "  " << i + 1 << " — " << choices[i].label << '\n';

    const auto selected = readIndexes(
        "Введите номера через пробел: ",
        choices.size());

    std::vector<std::string> usefulRockTypes;

    for (const std::size_t index : selected)
    {
        const Choice& choice = choices[index - 1];

        switch (choice.kind)
        {
            case Choice::Kind::GostSand:
                settings.setGostSandClassUseful(choice.gost, true);
                if (std::find(usefulRockTypes.begin(), usefulRockTypes.end(), "Песок") == usefulRockTypes.end())
                    usefulRockTypes.push_back("Песок");
                break;

            case Choice::Kind::TkpSand:
                settings.setTkpSandClassUseful(choice.tkp, true);
                if (std::find(usefulRockTypes.begin(), usefulRockTypes.end(), "Песок") == usefulRockTypes.end())
                    usefulRockTypes.push_back("Песок");
                break;

            case Choice::Kind::Supes:
                usefulRockTypes.push_back("Супесь");
                settings.setSupesUseful(true);
                break;

            case Choice::Kind::Suglinok:
                usefulRockTypes.push_back("Суглинок");
                settings.setSuglinokUseful(true);
                break;

            case Choice::Kind::Clay:
                usefulRockTypes.push_back("Глина");
                settings.setClayUseful(true);
                break;

            case Choice::Kind::Other:
                usefulRockTypes.push_back(choice.rockType);
                break;
        }
    }

    settings.setUsefulRockTypes(usefulRockTypes);
}

void ConsoleSettingsReader::readUnderlyingRockSettings(
    const Deposit& deposit,
    MainIndicatorsSettings& settings) const
{
    std::cout << "\n--- ПОДСТИЛАЮЩИЕ ПОРОДЫ ---\n"
              << "По умолчанию подстилающими считаются интервалы ниже последнего ПИ.\n"
              << "Можно дополнительно принудительно исключить выбранные типы из ПИ.\n";

    if (!readYesNo("Задать дополнительные типы подстилающих пород?"))
        return;

    const auto rockTypes = collectRockTypes(deposit);
    printRockTypes(rockTypes);

    if (!rockTypes.empty())
    {
        const auto selected = readSelectedRockTypes(rockTypes);
        settings.setUnderlyingRockTypes(selected);

        if (std::find(selected.begin(), selected.end(), "Песок") != selected.end())
        {
            std::cout << "\nИсточник классификации песка для подстилающих пород:\n"
                      << "1 — ГОСТ\n"
                      << "2 — ТКП\n"
                      << "3 — ГОСТ или ТКП\n";

            const int source = readInt("Источник классификации: ", 1, 3);
            settings.setUnderlyingSandClassificationSource(
                source == 1 ? SandClassificationSource::Gost :
                source == 2 ? SandClassificationSource::Tkp :
                               SandClassificationSource::GostAndTkp);

            readSandSelection(
                "ПОДСТИЛАЮЩИЕ ПОРОДЫ",
                source,
                [&settings](GostSandClass value)
                { settings.setUnderlyingGostSandClassUseful(value, true); },
                [&settings](TkpSandClass value)
                { settings.setUnderlyingTkpSandClassUseful(value, true); });
        }
    }
}

void ConsoleSettingsReader::readSurfaceSampleSettings(
    MainIndicatorsSettings& settings) const
{
    std::cout << "\n--- ПОВЕРХНОСТНЫЙ ОБРАЗЕЦ ---\n"
              << "1 — исключить из классификации\n"
              << "2 — считать полезным ископаемым\n"
              << "3 — считать внешней вскрышей / ПРС\n";

    const int mode = readInt("Ваш выбор: ", 1, 3);

    if (mode == 1)
        settings.setSurfaceSampleMode(SurfaceSampleMode::Exclude);
    else if (mode == 2)
        settings.setSurfaceSampleMode(SurfaceSampleMode::UsefulMineral);
    else
        settings.setSurfaceSampleMode(SurfaceSampleMode::ExternalOverburden);
}

std::vector<std::string>
ConsoleSettingsReader::collectRockTypes(const Deposit& deposit)
{
    std::vector<std::string> result =
    {
        "Песок",
        "Супесь",
        "Суглинок",
        "Глина"
    };

    for (const Borehole& borehole : deposit.getBoreholes())
    {
        for (const Sample& sample : borehole.getSamples())
        {
            if (sample.isAgSample() || sample.isMonolith())
                continue;

            const std::string rockType = sample.getRockType();
            if (rockType.empty())
                continue;

            if (std::find(result.begin(), result.end(), rockType) == result.end())
                result.push_back(rockType);
        }
    }

    return result;
}

void ConsoleSettingsReader::printRockTypes(
    const std::vector<std::string>& rockTypes)
{
    if (rockTypes.empty())
    {
        std::cout << "Типы пород не найдены.\n";
        return;
    }

    std::cout << "\nДоступные типы пород:\n";
    for (std::size_t i = 0; i < rockTypes.size(); ++i)
        std::cout << "  " << i + 1 << " — " << rockTypes[i] << '\n';
}

std::vector<std::string>
ConsoleSettingsReader::readSelectedRockTypes(
    const std::vector<std::string>& rockTypes)
{
    const auto indexes = readIndexes(
        "Введите номера через пробел: ",
        rockTypes.size());

    std::vector<std::string> result;
    for (const std::size_t index : indexes)
        result.push_back(rockTypes[index - 1]);

    return result;
}

ReserveSummarySettings ConsoleSettingsReader::readReserveSummarySettings(
    const Deposit& deposit) const
{
    ReserveSummarySettings settings;

    std::cout
        << "\n--- КОНТУРЫ И ПЛОЩАДИ БЛОКОВ ---\n"
        << "1 — автоматически: выпуклая оболочка всех скважин блока\n"
        << "2 — вручную: задать крайние скважины, через которые проходит контур\n";

    const int mode =
        readInt("Ваш выбор: ", 1, 2);

    settings.setAreaMode(
        mode == 1
            ? BlockAreaMode::AutomaticConvexHull
            : BlockAreaMode::ManualBoundary);

    for (const Block& block : deposit.getBlocks())
    {
        ReserveSummaryBlockSettings value;
        value.blockName = block.getName();

        if (mode == 2)
        {
            value.boundaryBoreholes =
                readOrderedBoreholes(block);
        }

        settings.addBlock(value);
    }

    return settings;
}

GroundwaterCalculationSettings
ConsoleSettingsReader::readGroundwaterCalculationSettings() const
{
    GroundwaterCalculationSettings settings;

    std::cout << "\n--- ГРУНТОВЫЕ ВОДЫ: РЕЖИМ РАСЧЁТА ---\n"
              << "1 — автоматически: средний уровень при большинстве водоносных скважин\n"
              << "2 — принудительно средний уровень\n"
              << "3 — индивидуальный уровень каждой скважины\n";

    const int mode = readInt("Ваш выбор: ", 1, 3);

    if (mode == 1)
        settings.setMode(GroundwaterCalculationMode::Automatic);
    else if (mode == 2)
        settings.setMode(GroundwaterCalculationMode::MeanLevel);
    else
        settings.setMode(GroundwaterCalculationMode::IndividualLevel);

    if (mode == 1)
    {
        const double percent = readDouble(
            "Доля водоносных скважин для режима среднего УГВ, %: ",
            0.0,
            100.0);
        settings.setMajorityThreshold(percent / 100.0);
    }

    return settings;
}

GroundwaterSettings ConsoleSettingsReader::readGroundwaterSettings() const
{
    GroundwaterSettings settings;

    std::cout << "\n--- ПОЛЕЗНОЕ ИСКОПАЕМОЕ И УГВ ---\n"
              << "1 — учитывать сухое + обводнённое ПИ\n"
              << "2 — учитывать только сухое ПИ\n";

    const int mode = readInt("Ваш выбор: ", 1, 2);

    settings.setUsefulMineralWaterMode(
        mode == 1
            ? UsefulMineralWaterMode::DryAndWaterBearing
            : UsefulMineralWaterMode::DryOnly);

    settings.setDryLayerAboveGroundwater(
        readDouble("Сухая подушка над расчётным УГВ, м: ", 0.0, 1000.0));

    return settings;
}

DevelopmentSettings ConsoleSettingsReader::readDevelopmentSettings() const
{
    DevelopmentSettings settings;

    std::cout << "\n--- РАЗРАБОТКА ОБВОДНЁННОГО ПИ ---\n"
              << "1 — пользовательское ограничение\n"
              << "2 — экскаватор (2.5 м)\n"
              << "3 — земснаряд / без ограничения\n";

    const int mode = readInt("Способ разработки: ", 1, 3);

    if (mode == 1)
    {
        settings.setMethod(DevelopmentMethod::Custom);
        settings.setWaterBearingMiningThickness(
            readDouble("Максимальная мощность обводнённого ПИ, м (0 = без ограничения): ", 0.0, 100000.0));
    }
    else if (mode == 2)
    {
        settings.setMethod(DevelopmentMethod::Excavator);
    }
    else
    {
        settings.setMethod(DevelopmentMethod::Dredger);
        settings.setWaterBearingMiningThickness(0.0);
    }

    return settings;
}

SectionSettings ConsoleSettingsReader::readSectionSettings(
    const Deposit& deposit) const
{
    SectionSettings settings;

    const auto& boreholes = deposit.getBoreholes();

    if (boreholes.size() < 2)
    {
        std::cout << "\nНедостаточно скважин для построения разреза.\n";
        return settings;
    }

    std::cout << "\n==============================================\n"
              << "       НАСТРОЙКА ГЕОЛОГО-ЛИТОЛОГИЧЕСКИХ РАЗРЕЗОВ\n"
              << "==============================================\n";

    const int sectionCount = readInt(
        "Количество разрезов: ",
        1,
        100);

    std::vector<std::vector<std::string>> sections;
    sections.reserve(static_cast<std::size_t>(sectionCount));

    for (int sectionIndex = 1; sectionIndex <= sectionCount; ++sectionIndex)
    {
        for (;;)
        {
            std::cout << "\nРазрез №" << sectionIndex << ". Доступные скважины:\n";

            for (const Borehole& borehole : boreholes)
                std::cout << "  " << borehole.getBoreholeNumber() << ' ';

            std::cout
                << "\nВведите номера скважин в порядке построения разреза\n"
                << "(например: 1 5 8 12 15): ";

            std::string line;
            std::getline(std::cin, line);

            std::stringstream input(line);
            std::vector<std::string> selected;
            std::set<std::string> unique;
            std::string value;

            while (input >> value)
            {
                const auto it = std::find_if(
                    boreholes.begin(),
                    boreholes.end(),
                    [&value](const Borehole& borehole)
                    {
                        return borehole.getBoreholeNumber() == value;
                    });

                if (it == boreholes.end() || !unique.insert(value).second)
                {
                    selected.clear();
                    break;
                }

                selected.push_back(value);
            }

            if (selected.size() >= 2)
            {
                sections.push_back(std::move(selected));
                break;
            }

            std::cout
                << "Нужно указать не менее двух различных существующих скважин. "
                << "Повторите ввод.\n";
        }
    }

    settings.setSections(sections);
    return settings;
}
