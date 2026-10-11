#include "domain/GeologicalModel.h"

#include <cassert>
#include <iostream>

using terranex::domain::Block;
using terranex::domain::Borehole;
using terranex::domain::Deposit;
using terranex::domain::Sample;
using terranex::domain::validateDeposit;

int main()
{
    Deposit deposit;
    deposit.id = "deposit-1";
    deposit.name = "Тестовое месторождение";

    deposit.blocks.push_back(Block{"block-b", "IIIB", "B"});
    deposit.blocks.push_back(Block{"block-c1", "IVC1", "C1"});

    Borehole borehole;
    borehole.id = "borehole-4";
    borehole.number = "4";
    borehole.x = 6092444.4884;
    borehole.y = 2209858.5177;
    borehole.elevation = 180.613;
    borehole.blockIds = {"block-b", "block-c1"};
    deposit.boreholes.push_back(borehole);

    Sample sample;
    sample.id = "sample-1";
    sample.number = "1";
    sample.boreholeId = "borehole-4";
    sample.depthFrom = 0.2;
    sample.depthTo = 0.7;
    sample.laboratory.gostParameters["gravel_gt_5mm_percent"] = 12.5;
    sample.laboratory.spParameters["particles_gt_2mm_percent"] = 31.0;
    deposit.samples.push_back(sample);

    assert(validateDeposit(deposit).empty());

    // Проверяем, что недопустимый интервал обнаруживается валидатором.
    deposit.samples.front().depthTo = deposit.samples.front().depthFrom;
    const auto issues = validateDeposit(deposit);
    assert(!issues.empty());

    bool foundIntervalIssue = false;
    for (const auto& issue : issues)
    {
        if (issue.code == "sample.interval.invalid")
            foundIntervalIssue = true;
    }
    assert(foundIntervalIssue);

    std::cout << "TerraNex domain model tests passed\n";
    return 0;
}
