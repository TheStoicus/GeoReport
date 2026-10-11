#pragma once

#include "../../application/GeologicalAnalysisResult.h"
#include "../../domain/Deposit.h"

#include "DxfDocument.h"

class DxfColumnWriter
{
public:
    void write(const std::string& filePath, const Deposit& deposit,
        const GeologicalAnalysisResult& analysisResult) const;

private:
    static constexpr double COLUMN_WIDTH = 5.0; // Ширина геологической колонки.
    static constexpr double DEPTH_SCALE = 10.0; // Вертикальный масштаб: 1 метр глубины = 10 единиц DXF.
    static constexpr double COLUMN_SPACING = 60.0; // Расстояние между колонками.
    static constexpr double TEXT_HEIGHT = 2.5; // Высота текста.
    static constexpr double TEXT_OFFSET = 2.0; // Расстояние от колонки до подписей.
    static constexpr double DEPTH_MARK_LENGTH = 3.0; // Длина деления глубинной шкалы.
};
