#pragma once

#include <string>

#include "../domain/Deposit.h"

class ExcelReader
{
public:
    /*
     * Читает один исходный Excel-файл.
     * Лист 1: журнал блоков/скважин.
     * Лист 2: координаты и паспортные данные.
     * Лист 3: описания слоев и образцов.
     * Лист 4: лаборатория по ГОСТ.
     * Лист 5: лаборатория по ТКП.
     */
    Deposit read(const std::string& filePath) const;

private:
    void readBlockJournal(const std::string& filePath, Deposit& deposit) const;
    void readCoordinates(const std::string& filePath, Deposit& deposit) const;
    void readLayerDescriptions(const std::string& filePath, Deposit& deposit) const;
    void readLaboratory(const std::string& filePath, Deposit& deposit, unsigned int sheetNumber, bool gost) const;
};
