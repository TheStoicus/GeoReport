#pragma once

#include <map>
#include <string>

#include "WeightedAverageResult.h"

class BoreholeResult
{
private:
    std::string boreholeNumber;

    double totalThickness = 0.0; // Общая расчетная мощность скважины. Сюда входит сумма мощностей всех образцов, которые допускаются к расчету.
    
    WeightedAverageResult weightedAverage; // Средневзвешенные значения параметров ГОСТ.

    /*
     * Расчетная мощность отдельно для каждого параметра ГОСТ.
     * Это необходимо потому, что конкретный параметр может отсутствовать или быть текстовым не во всех образцах скважины.
     * Например:
     *   Модуль крупности:
     *       значение = 2.35
     *       мощность = 4.20 м
     *   Другой параметр:
     *       значение = 5.10
     *       мощность = 5.80 м
     */
    std::map<std::string, double> parameterThickness;

public:
    void setBoreholeNumber(const std::string& number);
    const std::string& getBoreholeNumber() const;

    void setTotalThickness(double thickness);
    double getTotalThickness() const;

    void setWeightedAverage(const WeightedAverageResult& result);

    const WeightedAverageResult& getWeightedAverage() const;

    void setParameterThickness(const std::string& parameterName, double thickness);

    double getParameterThickness(const std::string& parameterName) const;

    const std::map<std::string, double>& getParameterThicknesses() const;
};
