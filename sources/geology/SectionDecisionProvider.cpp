#include "../../headers/geology/SectionDecisionProvider.h"

#include <iostream>
#include <limits>

std::vector<SectionDecision>
SectionDecisionProvider::getDecisions(
    const BoreholeSectionAnalysis& analysis) const
{
    std::vector<SectionDecision> decisions;

    const auto& ambiguous =
        analysis.getClassification().getAmbiguousSections();

    decisions.reserve(ambiguous.size());

    for (const BoreholeSection& section : ambiguous)
    {
        std::cout
            << "\nСкважина " << analysis.getBoreholeNumber()
            << ": неоднозначный интервал "
            << section.getStartDepth() << "–"
            << section.getEndDepth() << " м\n"
            << "  1 — полезное ископаемое\n"
            << "  2 — внутренняя вскрыша\n";

        int choice = 0;
        while (choice != 1 && choice != 2)
        {
            std::cout << "Ваш выбор: ";
            if (std::cin >> choice && (choice == 1 || choice == 2))
                break;

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Введите 1 или 2.\n";
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        decisions.emplace_back(
            section.getStartDepth(),
            section.getEndDepth(),
            choice == 1
                ? AmbiguousSectionDecision::UsefulMineral
                : AmbiguousSectionDecision::InternalOverburden);
    }

    return decisions;
}
