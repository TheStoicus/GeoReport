#include "../../headers/geology/BoreholeGeology.h"

void BoreholeGeology::setSurfaceSoil(const SurfaceSoil& value)
{
    surfaceSoil = value;
}

bool BoreholeGeology::hasSurfaceSoil() const
{
    return surfaceSoil.has_value();
}

const std::optional<SurfaceSoil>& BoreholeGeology::getSurfaceSoil() const
{
    return surfaceSoil;
}

void BoreholeGeology::setOverburden(const std::vector<Overburden>& value)
{
    overburden = value;
}

const std::vector<Overburden>& BoreholeGeology::getOverburden() const
{
    return overburden;
}

void BoreholeGeology::setUsefulMineral(const UsefulMineral& value)
{
    usefulMineral = value;
}

const UsefulMineral& BoreholeGeology::getUsefulMineral() const
{
    return usefulMineral;
}

void BoreholeGeology::setUnderlyingRock(const std::vector<UnderlyingRock>& value)
{
    underlyingRock = value;
}

const std::vector<UnderlyingRock>& BoreholeGeology::getUnderlyingRock() const
{
    return underlyingRock;
}

double BoreholeGeology::getTotalOverburdenThickness() const
{
    double thickness = 0.0;

    if (surfaceSoil.has_value())
        thickness += surfaceSoil->getThickness();

    for (const Overburden& section : overburden)
    {
        thickness += section.getThickness();
    }

    return thickness;
}

double BoreholeGeology::getTotalUnderlyingRockThickness() const
{
    double thickness = 0.0;

    for (const UnderlyingRock& section : underlyingRock)
    {
        thickness += section.getThickness();
    }

    return thickness;
}