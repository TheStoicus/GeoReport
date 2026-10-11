#pragma once

#include "../utils/ParameterValue.h"
#include "../utils/SandClassification.h"

#include <map>
#include <string>

class Sample
{
public:
    Sample() = default;

    Sample(
        const std::string& sampleNumber,
        double startDepth,
        double endDepth);

    void setSampleNumber(const std::string& sampleNumber);
    const std::string& getSampleNumber() const;

    bool isMonolith() const;

    // Специальный образец, вручную помеченный пользователем как "аг".
    bool isAgSample() const;

    bool isSurfaceSample() const;

    double getStartDepth() const;
    double getEndDepth() const;
    double getThickness() const;

    void setStartDepth(double depth);
    void setEndDepth(double depth);
    void calculateThickness();

    void setGostSoilType(const std::string& soilType);
    const std::string& getGostSoilType() const;

    void setTkpSoilType(const std::string& soilType);
    const std::string& getTkpSoilType() const;

    std::string getSoilType() const;
    std::string getRockType() const;

    GostSandClass getGostSandClass() const;
    TkpSandClass getTkpSandClass() const;

    void setGOSTParameter(
        const std::string& name,
        const ParameterValue& value);

    const std::map<std::string, ParameterValue>&
    getGOSTParameters() const;

    const ParameterValue*
    findGOSTParameter(const std::string& name) const;

    void setTKPParameter(
        const std::string& name,
        const ParameterValue& value);

    const std::map<std::string, ParameterValue>&
    getTKPParameters() const;

    const ParameterValue*
    findTKPParameter(const std::string& name) const;

private:
    std::string sampleNumber;

    bool monolith = false;
    bool agSample = false;

    double startDepth = 0.0;
    double endDepth = 0.0;
    double thickness = 0.0;

    std::string gostSoilType;
    std::string tkpSoilType;

    std::map<std::string, ParameterValue> gostParameters;
    std::map<std::string, ParameterValue> tkpParameters;
};
