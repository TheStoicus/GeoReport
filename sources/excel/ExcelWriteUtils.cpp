#include "../../headers/excel/ExcelWriteUtils.h"

#include <OpenXLSX.hpp>

#include <cmath>
#include <algorithm>

using namespace OpenXLSX;

namespace ExcelWriteUtils
{
    std::string columnName(int column)
    {
        std::string result;

        while (column > 0)
        {
            --column;

            result.insert(result.begin(), static_cast<char>('A' + (column % 26)));

            column /= 26;
        }

        return result;
    }

    std::string cellAddress(int column, uint32_t row)
    {
        return columnName(column) + std::to_string(row);
    }

    void writeString(XLWorksheet& sheet, uint32_t row, int column, const std::string& value)
    {
        // Убираем управляющие символы, недопустимые в XML 1.0.
        // Их наличие в исходных текстовых лабораторных данных может
        // приводить к сообщению Excel о повреждённом содержимом result.xlsx.
        std::string sanitized;
        sanitized.reserve(value.size());
        for (const unsigned char c : value)
        {
            if (c == '\t' || c == '\n' || c == '\r' || c >= 0x20)
                sanitized.push_back(static_cast<char>(c));
        }
        sheet.cell(cellAddress(column, row)).value() = sanitized;
    }

    void writeDouble(XLWorksheet& sheet, uint32_t row, int column, double value)
    {
        /*
         * XLSX не допускает NaN/Infinity как числовое значение.
         * Запись таких значений через OpenXLSX может привести к тому,
         * что Excel при открытии сообщит об ошибке содержимого.
         * Поэтому невалидные расчётные значения оставляем пустыми.
         */
        if (!std::isfinite(value))
            return;

        sheet.cell(cellAddress(column, row)).value() = value;
    }

    double roundHalfUp(double value, int decimals)
    {
        if (!std::isfinite(value))
            return value;

        const double factor = std::pow(10.0, decimals);

        /*
         * Компенсируем погрешность представления десятичных чисел в double.
         * Например, 180.15 может храниться как: 180.149999999...
         * Поэтому добавляем небольшую поправку перед округлением.
         */
        const double epsilon = 1e-9 * std::max(1.0, std::abs(value));

        if (value >= 0.0)
            return std::floor((value + epsilon) * factor + 0.5) / factor;

        return std::ceil((value - epsilon) * factor - 0.5) / factor;
    }

    double roundToOneDecimal(double value)
    {
        return roundHalfUp(value, 1);
    }

    double roundToTwoDecimals(double value)
    {
        return roundHalfUp(value, 2);
    }
}