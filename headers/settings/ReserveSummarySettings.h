#pragma once

#include <string>
#include <vector>

/*
 * Способ определения контура/площади блока.
 *
 * AutomaticConvexHull — программа строит выпуклую оболочку всех скважин
 * блока. Это полностью автоматический, но приближённый способ: внутренние
 * скважины не влияют на контур, однако выпуклая оболочка может проходить
 * дальше фактической границы геологического блока.
 *
 * ManualBoundary — пользователь задаёт крайние скважины в порядке обхода
 * фактического контура. Этот режим является точным для принятого пользователем
 * контура и одновременно используется для отрисовки контура на плане.
 */
enum class BlockAreaMode
{
    AutomaticConvexHull,
    ManualBoundary
};

struct ReserveSummaryBlockSettings
{
    std::string blockName;

    // Скважины фактического контура в порядке обхода.
    // Используется только в ManualBoundary.
    std::vector<std::string> boundaryBoreholes;
};

class ReserveSummarySettings
{
public:
    void setAreaMode(BlockAreaMode value);
    BlockAreaMode getAreaMode() const;

    void addBlock(const ReserveSummaryBlockSettings& value);

    const std::vector<ReserveSummaryBlockSettings>& getBlocks() const;

    const ReserveSummaryBlockSettings* findBlock(
        const std::string& blockName) const;

private:
    BlockAreaMode areaMode = BlockAreaMode::AutomaticConvexHull;
    std::vector<ReserveSummaryBlockSettings> blocks;
};
