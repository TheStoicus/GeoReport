#pragma once

#include <optional>
#include <string>
#include <variant>

/*
 * Лабораторное значение может быть:
 *   - числом;
 *   - текстом;
 *   - отсутствовать.
 * Это важно: текст вроде "светлее эталона" нельзя превращать в 0.0, иначе при последующих расчетах мы потеряем смысл исходных данных.
 */
using ParameterValue = std::variant<std::monostate, double, std::string>;

bool isNumeric(const ParameterValue& value);
bool isText(const ParameterValue& value);
bool isEmpty(const ParameterValue& value);

std::optional<double> getNumericValue(const ParameterValue& value);
std::string getTextValue(const ParameterValue& value);
