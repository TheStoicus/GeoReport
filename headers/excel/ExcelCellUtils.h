#pragma once

#include <OpenXLSX.hpp>
#include <string>

#include "../utils/ParameterValue.h"

std::string getCellAsString(const OpenXLSX::XLCell& cell);
ParameterValue getCellAsParameterValue(const OpenXLSX::XLCell& cell);
