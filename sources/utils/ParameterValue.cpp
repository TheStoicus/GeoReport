#include "../../headers/utils/ParameterValue.h"

bool isNumeric(const ParameterValue& value)
{
    return std::holds_alternative<double>(value);
}

bool isText(const ParameterValue& value)
{
    return std::holds_alternative<std::string>(value);
}

bool isEmpty(const ParameterValue& value)
{
    return std::holds_alternative<std::monostate>(value);
}

std::optional<double> getNumericValue(const ParameterValue& value)
{
    if (!isNumeric(value))
        return std::nullopt;

    return std::get<double>(value);
}

std::string getTextValue(const ParameterValue& value)
{
    if (!isText(value))
        return {};

    return std::get<std::string>(value);
}
