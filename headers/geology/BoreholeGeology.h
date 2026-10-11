#pragma once

#include "../domain/Overburden.h"
#include "../domain/SurfaceSoil.h"
#include "../domain/UnderlyingRock.h"
#include "../domain/UsefulMineral.h"

#include <optional>
#include <vector>

class BoreholeGeology
{
public:
    void setSurfaceSoil(const SurfaceSoil& surfaceSoil);

    bool hasSurfaceSoil() const;

    const std::optional<SurfaceSoil>& getSurfaceSoil() const;

    void setOverburden(const std::vector<Overburden>& overburden);
    void setUsefulMineral(const UsefulMineral& usefulMineral);
    void setUnderlyingRock(const std::vector<UnderlyingRock>& underlyingRock);

    const std::vector<Overburden>& getOverburden() const;
    const UsefulMineral& getUsefulMineral() const;
    const std::vector<UnderlyingRock>& getUnderlyingRock() const;
    double getTotalOverburdenThickness() const;
    double getTotalUnderlyingRockThickness() const;

private:
    std::optional<SurfaceSoil> surfaceSoil;

    std::vector<Overburden> overburden;

    UsefulMineral usefulMineral;

    std::vector<UnderlyingRock> underlyingRock;
};
