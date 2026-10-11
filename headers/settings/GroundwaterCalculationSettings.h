#pragma once


enum class GroundwaterCalculationMode
{
    Automatic,
    MeanLevel,
    IndividualLevel
};


class GroundwaterCalculationSettings
{
public:

    void setMode(
        GroundwaterCalculationMode mode);

    GroundwaterCalculationMode getMode() const;


    /*
     * Доля скважин с вскрытым УГВ,
     * начиная с которой УГВ считается преобладающим.
     *
     * Например:
     *
     * 0.50 = 50 %
     * 0.60 = 60 %
     * 0.70 = 70 %
     *
     * По умолчанию — 50 %.
     */
    void setMajorityThreshold(
        double threshold);

    double getMajorityThreshold() const;


private:

    GroundwaterCalculationMode mode =
        GroundwaterCalculationMode::Automatic;

    double majorityThreshold = 0.50;
};
