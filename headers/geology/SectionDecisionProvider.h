#pragma once

#include "../calculations/BoreholeSectionAnalysis.h"
#include "SectionDecision.h"

#include <vector>

/*
 * Интерактивный источник решений по неоднозначным интервалам.
 *
 * Пока интерфейс консольный. В дальнейшем этот класс можно заменить
 * оконным интерфейсом без изменения классификатора и расчётов.
 */
class SectionDecisionProvider
{
public:
    std::vector<SectionDecision> getDecisions(
        const BoreholeSectionAnalysis& analysis) const;
};
