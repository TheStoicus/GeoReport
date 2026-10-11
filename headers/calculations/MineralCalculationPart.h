#pragma once

/*
 * Часть полезного ископаемого, для которой выполняется расчёт.
 *
 * TotalUsefulMineral — весь принятый к расчёту объём ПИ.
 * DryUsefulMineral   — только сухое ПИ.
 * WaterBearingUsefulMineral — только обводнённое ПИ.
 */
enum class MineralCalculationPart
{
    TotalUsefulMineral,
    DryUsefulMineral,
    WaterBearingUsefulMineral
};
