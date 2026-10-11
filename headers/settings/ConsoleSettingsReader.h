#pragma once

#include "MainIndicatorsSettings.h"
#include "GroundwaterCalculationSettings.h"
#include "GroundwaterSettings.h"
#include "DevelopmentSettings.h"
#include "ReserveSummarySettings.h"
#include "SectionSettings.h"
#include "../domain/Deposit.h"

class ConsoleSettingsReader
{
public:
    MainIndicatorsSettings read(const Deposit& deposit) const;
    GroundwaterCalculationSettings readGroundwaterCalculationSettings() const;
    GroundwaterSettings readGroundwaterSettings() const;
    DevelopmentSettings readDevelopmentSettings() const;
    ReserveSummarySettings readReserveSummarySettings(const Deposit& deposit) const;
    SectionSettings readSectionSettings(const Deposit& deposit) const;

private:
    void readOverburdenSettings(const Deposit& deposit, MainIndicatorsSettings& settings) const;
    void readUsefulMineralSettings(const Deposit& deposit, MainIndicatorsSettings& settings) const;
    void readMinimumOverburdenThickness(MainIndicatorsSettings& settings) const;
    void readUnderlyingRockSettings(const Deposit& deposit, MainIndicatorsSettings& settings) const;
    void readSurfaceSampleSettings(MainIndicatorsSettings& settings) const;

    static std::vector<std::string> collectRockTypes(const Deposit& deposit);
    static void printRockTypes(const std::vector<std::string>& rockTypes);
    static std::vector<std::string> readSelectedRockTypes(const std::vector<std::string>& rockTypes);
};
