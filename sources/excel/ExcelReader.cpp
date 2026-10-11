#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <ctime>

#include "../../headers/excel/ExcelReader.h"
#include "../../headers/excel/ExcelCellUtils.h"

#include <OpenXLSX.hpp>

namespace
{
    std::vector<std::string> splitBlockNames(const std::string& text)
    {
        std::vector<std::string> result;
        std::string current;

        for (char c : text)
        {
            if (c == ',' || c == ';' || c == '/' || c == '\\')
            {
                if (!current.empty())
                {
                    result.push_back(current);
                    current.clear();
                }
            }
            else
            {
                current += c;
            }
        }

        if (!current.empty())
            result.push_back(current);

        return result;
    }


    bool openDocument(OpenXLSX::XLDocument& document, const std::string& path)
    {
        try
        {
            document.open(path);
            return true;
        }
        catch (const std::exception& error)
        {
            std::cerr << "Не удалось открыть Excel-файл: " << path << "\n"
                << "Причина: " << error.what() << '\n';

            return false;
        }
        catch (...)
        {
            std::cerr << "Не удалось открыть Excel-файл: " << path << '\n';

            return false;
        }
    }
}

// Чтение всего Excel-файла
Deposit ExcelReader::read(const std::string& filePath) const
{
    Deposit deposit;

    readBlockJournal(filePath, deposit); // Лист 1 — журнал блоков и скважин.
    readCoordinates(filePath, deposit); // Лист 2 — координаты и паспортные данные.
    readLayerDescriptions(filePath, deposit); // Лист 3 пока намеренно не используется.
    readLaboratory(filePath, deposit, 4, true); // Лист 4 — лаборатория по ГОСТ.

    /*
     * Лист 5 — лаборатория по ТКП.
     * Числовые параметры ТКП не используются в средневзвешенных ГОСТ-расчетах.
     * При этом название грунта по ТКП сохраняется отдельно в Sample::tkpSoilType().
     */
    readLaboratory(filePath, deposit, 5, false);

    return deposit;
}

// Лист 1 Журнал блоков / скважин
void ExcelReader::readBlockJournal(const std::string& filePath, Deposit& deposit) const
{
    OpenXLSX::XLDocument document;

    if (!openDocument(document, filePath))
        return;

    try
    {
        auto worksheet = document.workbook().sheet(1).get<OpenXLSX::XLWorksheet>();

        const uint32_t rows = worksheet.rowCount();

        for (uint32_t row = 2; row <= rows; ++row)
        {
            const std::string blockName = getCellAsString(worksheet.cell(row, 1));
            const std::string boreholeNumber = getCellAsString(worksheet.cell(row, 2));

            if (boreholeNumber.empty())
                continue;

            Borehole* borehole = deposit.findBorehole(boreholeNumber);

            // Если скважины еще нет — создаем.
            if (borehole == nullptr)
            {
                Borehole newBorehole;

                newBorehole.setBoreholeNumber(boreholeNumber);

                const auto depth =  getNumericValue(getCellAsParameterValue(worksheet.cell(row, 3)));
                const auto groundwater = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 4)));

                if (depth)
                    newBorehole.setTotalDepth(*depth);

                if (groundwater)
                    newBorehole.setGroundwaterDepth(*groundwater);

                const auto date = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 6)));
                if (date)
                {
                    // Excel stores dates as serial numbers.  2026-01-01 is 46023.
                    const int serial = static_cast<int>(*date);
                    if (serial > 40000)
                    {
                        const int days = serial - 25569;
                        std::time_t t = static_cast<std::time_t>(days) * 86400;
                        std::tm tm{};
                        gmtime_r(&t, &tm);
                        newBorehole.setDrillingYear(tm.tm_year + 1900);
                    }
                }

                deposit.addBorehole(newBorehole);

                borehole = deposit.findBorehole(boreholeNumber);
            }
            else
            {
                const auto depth = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 3)));
                const auto groundwater = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 4)));

                if (depth)
                    borehole->setTotalDepth(*depth);

                if (groundwater)
                    borehole->setGroundwaterDepth(*groundwater);

                const auto date = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 6)));
                if (date)
                {
                    const int serial = static_cast<int>(*date);
                    if (serial > 40000)
                    {
                        const int days = serial - 25569;
                        std::time_t t = static_cast<std::time_t>(days) * 86400;
                        std::tm tm{};
                        gmtime_r(&t, &tm);
                        borehole->setDrillingYear(tm.tm_year + 1900);
                    }
                }
            }

            // Блок
            if (blockName.empty())
                continue;

            const std::vector<std::string> blockNames = splitBlockNames(blockName);

            for (std::string name : blockNames)
            {
                while (!name.empty() && name.front() == ' ')
                {
                    name.erase(name.begin());
                }

                while (!name.empty() && name.back() == ' ')
                {
                    name.pop_back();
                }

                if (name.empty())
                    continue;

                Block* block = deposit.findBlock(name);

                if (block == nullptr)
                {
                    deposit.addBlock(Block(name));
                    block = deposit.findBlock(name);
                }

                if (block != nullptr)
                    block->addBorehole(boreholeNumber);
            }
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Ошибка чтения листа 1: "
            << error.what() << '\n';
    }

    document.close();
}

// Лист 2 Координаты и паспортные данные
void ExcelReader::readCoordinates(const std::string& filePath, Deposit& deposit) const
{
    OpenXLSX::XLDocument document;

    if (!openDocument(document, filePath))
        return;

    try
    {
        auto worksheet = document.workbook().sheet(2).get<OpenXLSX::XLWorksheet>();
        const uint32_t rows = worksheet.rowCount();

        for (uint32_t row = 2; row <= rows; ++row)
        {
            const std::string number = getCellAsString(worksheet.cell(row, 1));

            if (number.empty())
                continue;

            Borehole* borehole = deposit.findBorehole(number);

            if (borehole == nullptr)
            {
                Borehole newBorehole;

                newBorehole.setBoreholeNumber(number);

                deposit.addBorehole(newBorehole);

                borehole = deposit.findBorehole(number);
            }

            const auto x = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 2)));
            const auto y = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 3)));
            const auto latitude = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 4)));
            const auto longitude = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 5)));
            const auto elevation = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 6)));

            if (x && y)
                borehole->setCoordinate(*x, *y, CoordinateSystem::SK1963);

            // Широта и долгота wgs84 пока не используются в расчетах GeoReport.
            (void)latitude;
            (void)longitude;

            if (elevation)
                borehole->setCollarElevation(*elevation);
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Ошибка чтения листа 2: " << error.what() << '\n';
    }

    document.close();
}

// Лист 3 Описания слоев Пока не используется.
void ExcelReader::readLayerDescriptions(const std::string& filePath, Deposit& deposit) const
{
    (void)filePath;
    (void)deposit;
}

// Листы 4 и 5 Лабораторные данные
// gost == true → лист 4, ГОСТ
// gost == false → лист 5, ТКП
void ExcelReader::readLaboratory(const std::string& filePath, Deposit& deposit, unsigned int sheetNumber, bool gost) const
{
    OpenXLSX::XLDocument document;

    if (!openDocument(document, filePath))
        return;

    try
    {
        auto worksheet = document.workbook().sheet(sheetNumber).get<OpenXLSX::XLWorksheet>();
        const uint32_t rows = worksheet.rowCount();

        // Соответствие:
        // номер колонки Excel
        //        ↓
        // имя параметра в программе
        std::map<int, std::string> parameterNames;

        if (gost)
        {
            parameterNames =
            {
                {5,  "Грансостав гравия >5 мм"},
                {6,  "Грансостав песка <5 мм"},
                {7,  "Грансостав гравия >70 мм"},
                {8,  "Грансостав гравия 70-40 мм"},
                {9,  "Грансостав гравия 40-20 мм"},
                {10, "Грансостав гравия 20-10 мм"},
                {11, "Грансостав гравия 10-5 мм"},
                {12, "Грансостав песка 2.5 мм"},
                {13, "Грансостав песка 1.25 мм"},
                {14, "Грансостав песка 0.63 мм"},
                {15, "Грансостав песка 0.315 мм"},
                {16, "Грансостав песка 0.16 мм"},
                {17, "Грансостав песка <0.16 мм"},
                {18, "Глинистые в песке, %"},
                {19, "Глинистые в гравии, %"},
                {20, "Глинистые в породе, %"},
                {21, "Колориметрическая проба в песке"},
                {22, "Колориметрическая проба в гравии"},
                {23, "Модуль крупности песка"},

                /*
                 * ВАЖНО:
                 * Колонка 24 — название/группа песка по ГОСТ.
                 * Мы пока оставляем этот параметр в карте ГОСТ и дополнительно сохраняем его отдельно в Sample.
                 */
                {24, "Группа песка по ГОСТ 8736-2025"}
            };
        }
        else
        {
            parameterNames =
            {
                {5,  "ТКП >10 мм"},
                {6,  "ТКП 10-5 мм"},
                {7,  "ТКП 5-2 мм"},
                {8,  "ТКП 2-1 мм"},
                {9,  "ТКП 1-0.5 мм"},
                {10, "ТКП 0.5-0.25 мм"},
                {11, "ТКП 0.25-0.1 мм"},
                {12, "ТКП <0.1 мм"},
                {13, "ТКП <0.05 мм"},
                {14, "ТКП 0.05-0.01 мм"},
                {15, "ТКП 0.01-0.005 мм"},
                {16, "ТКП <0.005 мм"},
                {17, "ТКП >2 мм"},
                {18, "ТКП 2-0.25 мм"},
                {19, "ТКП 0.25-0.05 мм"},
                {20, "ТКП 0.05 мм"},
                {21, "Естеств.влажность %"},
                {22, "Плотность грунта"},
                {23, "Плотность сухого грунта"},
                {24, "Плотность частиц"},
                {25, "Коэффициент пористости грунта"},
                {26, "Пористость,%"},
                {27, "Степень влажности"},
                {28, "Угол естественного откоса, сухой"},
                {29, "Угол естественного откоса, под водой"},
                {30, "Коэфф. фильтрации, м/сут"},
                {31, "Граница текучести"},
                {32, "Граница раскатки"},
                {33, "Число пластичности"},
                {34, "Показ. текучести"},
                {35, "Потери при прокаливании"},
                {36, "Оптимальная влажность"},
                {37, "Максимальная плотность"},
                {38, "Показатель неоднородности"},
                {39, "Наименование грунта"}
            };
        }

        // Лабораторные данные начинаются со строки 7.
        for (uint32_t row = 7; row <= rows; ++row)
        {
            const std::string boreholeNumber = getCellAsString(worksheet.cell(row, 1));
            const std::string sampleNumber = getCellAsString(worksheet.cell(row, 2));

            if (boreholeNumber.empty() || sampleNumber.empty())
                continue;

            Borehole* borehole = deposit.findBorehole(boreholeNumber);

            if (borehole == nullptr)
            {
                std::cerr << "Предупреждение: скважина " << boreholeNumber
                    << " из лабораторного листа " << sheetNumber << " не найдена.\n";

                continue;
            }

            Sample* sample = nullptr;

            // Ищем уже существующий образец.
            for (Sample& candidate : borehole->getSamplesModifiable())
            {
                if (candidate.getSampleNumber() == sampleNumber)
                {
                    sample = &candidate;
                    break;
                }
            }

            // Если образца еще нет — создаем.
            if (sample == nullptr)
            {
                Sample newSample;

                newSample.setSampleNumber(sampleNumber);

                const auto startDepth = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 3)));
                const auto endDepth = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 4)));

                if (startDepth)
                    newSample.setStartDepth(*startDepth);

                if (endDepth)
                    newSample.setEndDepth(*endDepth);

                newSample.calculateThickness();

                borehole->addSample(newSample);

                sample = &borehole->getSamplesModifiable().back();
            }

            // Обновляем глубины образца.
            // Это особенно важно, потому что один и тот же образец может присутствовать на обоих листах.
            const auto startDepth = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 3)));
            const auto endDepth = getNumericValue(getCellAsParameterValue(worksheet.cell(row, 4)));

            if (startDepth)
                sample->setStartDepth(*startDepth);

            if (endDepth)
                sample->setEndDepth(*endDepth);

            sample->calculateThickness();

            // ОТДЕЛЬНОЕ СОХРАНЕНИЕ НАЗВАНИЯ ГРУНТА
            if (gost)
            {
                /*
                 * Лист 4:
                 * колонка 24 = "Группа песка по ГОСТ 8736-2025"
                 * Сохраняем исходный текст отдельно.
                 */
                const std::string gostSoilType = getCellAsString(worksheet.cell(row, 24));

                if (!gostSoilType.empty())
                    sample->setGostSoilType(gostSoilType);
            }
            else
            {
                /*
                 * Лист 5:
                 * колонка 19 = "Наименование грунта"
                 * Именно это значение сейчас используем как название грунта по ТКП.
                 */
                const std::string tkpSoilType = getCellAsString(worksheet.cell(row, 39));

                if (!tkpSoilType.empty())
                    sample->setTkpSoilType(tkpSoilType);
            }

            // ЛАБОРАТОРНЫЕ ПАРАМЕТРЫ
            /*
             * Существующее поведение сохраняем полностью.
             * Все значения по ГОСТ попадают в GOSTParameters.
             * Все значения по ТКП попадают в TKPParameters.
             * Это нужно, чтобы не сломать существующие калькуляторы и таблицы результатов.
             */
            for (const auto& [column, parameterName] : parameterNames)
            {
                const ParameterValue value = getCellAsParameterValue(worksheet.cell(row, column));

                if (gost)
                    sample->setGOSTParameter(parameterName, value);
                else
                    sample->setTKPParameter(parameterName, value);
            }
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Ошибка чтения лабораторного листа " << sheetNumber
            << ": " << error.what() << '\n';
    }

    document.close();
}
