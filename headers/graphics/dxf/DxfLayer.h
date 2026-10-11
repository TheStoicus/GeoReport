#pragma once

#include <string>

class DxfLayer
{
public:
    DxfLayer() = default;

    DxfLayer(
        const std::string& name,
        int color);

    DxfLayer(
        const std::string& name,
        int color,
        const std::string& lineType);

    const std::string& getName() const;
    int getColor() const;
    const std::string& getLineType() const;

private:
    std::string name;
    int color = 7;
    std::string lineType = "CONTINUOUS";
};
