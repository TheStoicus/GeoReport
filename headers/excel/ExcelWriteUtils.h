#pragma once

#include <cstdint>
#include <string>

namespace OpenXLSX
{
    class XLWorksheet;
}

namespace ExcelWriteUtils
{
    std::string columnName(int column);
    std::string cellAddress(int column, uint32_t row);

    void writeString(OpenXLSX::XLWorksheet& sheet, uint32_t row,
        int column, const std::string& value);

    void writeDouble(OpenXLSX::XLWorksheet& sheet, uint32_t row,
        int column, double value);

    double roundToOneDecimal(double value);
    double roundToTwoDecimals(double value);
}
