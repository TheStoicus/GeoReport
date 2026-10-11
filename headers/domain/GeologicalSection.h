#pragma once

class GeologicalSection
{
public:
    GeologicalSection() = default;

    GeologicalSection(double roofDepth, double bottomDepth);

    void setRoofDepth(double depth);
    void setBottomDepth(double depth);

    double getRoofDepth() const;
    double getBottomDepth() const;
    double getThickness() const;

private:
    double roofDepth = 0.0;
    double bottomDepth = 0.0;
};
