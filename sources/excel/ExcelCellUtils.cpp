#include "../../headers/excel/ExcelCellUtils.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <string>

namespace
{
    std::string trim(const std::string& input)
    {
        const auto first = input.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return {};

        const auto last = input.find_last_not_of(" \t\r\n");
        return input.substr(first, last - first + 1);
    }

    bool tryParseDouble(const std::string& text, double& result)
    {
        std::string normalized = trim(text);

        for (char& c : normalized)
        {
            if (c == ',')
                c = '.';
        }

        if (normalized.empty())
            return false;

        char* end = nullptr;
        const double value = std::strtod(normalized.c_str(), &end);

        if (end == normalized.c_str())
            return false;

        while (*end != '\0')
        {
            if (!std::isspace(static_cast<unsigned char>(*end)))
                return false;
            ++end;
        }

        result = value;
        return true;
    }
}

std::string getCellAsString(const OpenXLSX::XLCell& cell)
{
    const auto type = cell.value().type();

    if (type == OpenXLSX::XLValueType::Empty)
        return {};

    if (type == OpenXLSX::XLValueType::String)
        return trim(cell.value().get<std::string>());

    if (type == OpenXLSX::XLValueType::Integer)
        return std::to_string(cell.value().get<int64_t>());

    if (type == OpenXLSX::XLValueType::Float)
        return std::to_string(cell.value().get<double>());

    if (type == OpenXLSX::XLValueType::Boolean)
        return cell.value().get<bool>() ? "True" : "False";

    return {};
}

ParameterValue getCellAsParameterValue(const OpenXLSX::XLCell& cell)
{
    const auto type = cell.value().type();

    if (type == OpenXLSX::XLValueType::Empty)
        return std::monostate{};

    if (type == OpenXLSX::XLValueType::Float)
        return cell.value().get<double>();

    if (type == OpenXLSX::XLValueType::Integer)
        return static_cast<double>(cell.value().get<int64_t>());

    if (type == OpenXLSX::XLValueType::String)
    {
        const std::string text = trim(cell.value().get<std::string>());

        if (text.empty())
            return std::monostate{};

        double numericValue = 0.0;
        if (tryParseDouble(text, numericValue))
            return numericValue;

        return text;
    }

    return std::monostate{};
}
