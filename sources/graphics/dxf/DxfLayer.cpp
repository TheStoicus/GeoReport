#include "../../../headers/graphics/dxf/DxfLayer.h"


DxfLayer::DxfLayer(
    const std::string& name,
    int color)
    : name(name),
      color(color),
      lineType("CONTINUOUS")
{
}


DxfLayer::DxfLayer(
    const std::string& name,
    int color,
    const std::string& lineType)
    : name(name),
      color(color),
      lineType(lineType)
{
}


const std::string&
DxfLayer::getName() const
{
    return name;
}


int
DxfLayer::getColor() const
{
    return color;
}


const std::string&
DxfLayer::getLineType() const
{
    return lineType;
}
