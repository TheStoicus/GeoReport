#pragma once


enum class DevelopmentMethod
{
    Custom,
    Excavator,
    Dredger
};


class DevelopmentSettings
{
public:

    void setMethod(
        DevelopmentMethod method);

    DevelopmentMethod getMethod() const;


    /*
     * Максимальная мощность обводнённого
     * полезного ископаемого, которую разрешено
     * учитывать при разработке.
     *
     * Для экскаватора по умолчанию:
     *
     * 2.5 м
     */
    void setWaterBearingMiningThickness(
        double thickness);

    double getWaterBearingMiningThickness() const;


    /*
     * Сухая подушка над уровнем грунтовых вод,
     * которая включается в полезное ископаемое.
     *
     * Например:
     *
     * 0.5 м
     * 1.0 м
     */
    void setDryProtectiveLayerThickness(
        double thickness);

    double getDryProtectiveLayerThickness() const;


private:

    DevelopmentMethod method =
        DevelopmentMethod::Custom;


    double waterBearingMiningThickness = 0.0;


    double dryProtectiveLayerThickness = 0.0;
};
