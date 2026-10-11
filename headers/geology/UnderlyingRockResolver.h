#pragma once

#include "../domain/Borehole.h"
#include "../domain/UnderlyingRock.h"
#include "../domain/UsefulMineral.h"
#include "../calculations/BoreholeSectionAnalysis.h"

#include <string>
#include <vector>

class UnderlyingRockResolver
{
public:
    std::vector<UnderlyingRock> resolve(
        const Borehole& borehole,
        const BoreholeSectionAnalysis& analysis,
        const UsefulMineral& countedUsefulMineral) const;

private:
    static void addInterval(
        std::vector<UnderlyingRock>& result,
        double startDepth,
        double endDepth,
        const std::string& name);

    static std::string findUnderlyingRockName(
        const Borehole& borehole,
        double depth);
};
