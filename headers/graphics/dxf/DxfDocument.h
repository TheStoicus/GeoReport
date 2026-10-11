#pragma once

#include "DxfLayer.h"

#include <fstream>
#include <string>
#include <utility>
#include <vector>

class DxfDocument
{
public:
    void addLayer(const DxfLayer& layer);
    void addCircle(const std::string& layerName, double x, double y, double radius);
    void addText(const std::string& layerName, double x, double y, double height, const std::string& text);
    void addLine(const std::string& layerName, double x1, double y1, double x2, double y2);

    // Добавляет замкнутую или незамкнутую полилинию LWPOLYLINE.
    void addPolyline(
        const std::string& layerName,
        const std::vector<std::pair<double, double>>& points,
        bool closed = false,
        const std::string& lineType = "");

    void save(const std::string& filePath) const;

private:
    void writeHeader(std::ofstream& file) const;
    void writeLayers(std::ofstream& file) const;
    void writeEntities(std::ofstream& file) const;

    struct Entity
    {
        std::string data;
    };

    std::vector<DxfLayer> layers;
    std::vector<Entity> entities;
};
