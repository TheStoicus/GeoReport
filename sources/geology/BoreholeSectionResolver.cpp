#include "../../headers/geology/BoreholeSectionResolver.h"

#include <algorithm>
#include <cmath>

FinalBoreholeSections
BoreholeSectionResolver::resolve(const SectionClassificationResult& classification,
    const std::vector<SectionDecision>& decisions) const
{
    FinalBoreholeSections result;

    /*
     * Сначала собираем все участки в один временный список.
     * Однозначные участки уже имеют окончательный тип.
     * Неоднозначные участки получат тип из решения пользователя.
     */
    std::vector<BoreholeSection> resolvedSections;

    // Однозначные участки.
    for (const BoreholeSection& section : classification.getSections())
    {
        resolvedSections.push_back(section);
    }

    // Неоднозначные участки.
    for (const BoreholeSection& ambiguousSection : classification.getAmbiguousSections())
    {
        const SectionDecision* decision = findDecision(ambiguousSection, decisions);

        // Если решения нет, участок пока не включаем в окончательную модель.
        if (decision == nullptr)
            continue;

        BoreholeSectionType finalType;

        if (decision->getDecision() == AmbiguousSectionDecision::UsefulMineral)
            finalType = BoreholeSectionType::UsefulMineral;
        else
            finalType = BoreholeSectionType::InternalOverburden;

        resolvedSections.emplace_back(ambiguousSection.getStartDepth(),
            ambiguousSection.getEndDepth(), finalType);
    }

    /*
     * ВАЖНО:
     * Неоднозначные участки могут находиться между
     * однозначными. Поэтому перед построением окончательной
     * модели обязательно сортируем все участки по глубине.
     */
    std::sort(resolvedSections.begin(), resolvedSections.end(),
        [](const BoreholeSection& left, const BoreholeSection& right)
        {
            if (left.getStartDepth() != right.getStartDepth())
                return left.getStartDepth() < right.getStartDepth();

            return left.getEndDepth() < right.getEndDepth();
        });

    /*
     * Теперь добавляем участки в правильном порядке.
     * FinalBoreholeSections::addSection()
     * автоматически объединит соседние участки одного типа.
     */
    for (const BoreholeSection& section : resolvedSections)
    {
        result.addSection(section);
    }

    return result;
}


const SectionDecision*
BoreholeSectionResolver::findDecision(const BoreholeSection& ambiguousSection,
    const std::vector<SectionDecision>& decisions) const
{
    constexpr double EPS = 1e-9;

    for (const SectionDecision& decision : decisions)
    {
        if (std::abs(decision.getStartDepth() - ambiguousSection.getStartDepth()) < EPS &&
            std::abs(decision.getEndDepth() - ambiguousSection.getEndDepth()) < EPS)
            return &decision;
    }

    return nullptr;
}