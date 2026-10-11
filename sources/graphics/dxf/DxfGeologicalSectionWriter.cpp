#include "graphics/dxf/DxfGeologicalSectionWriter.h"
#include "domain/Borehole.h"
#include "domain/Sample.h"
#include "utils/SandClassification.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    constexpr double EPS = 1e-9;
    struct Item { const Borehole* borehole{}; const BoreholeAnalysisResult* result{}; double x{}; };

    const BoreholeAnalysisResult* findResult(const GeologicalAnalysisResult& r,const std::string& n)
    { for(const auto& x:r.getBoreholes()) if(x.getBoreholeNumber()==n) return &x; return nullptr; }
    std::string fmt(double v,int p=2){std::ostringstream s;s<<std::fixed<<std::setprecision(p)<<v;return s.str();}
    std::string sandAbbrev(GostSandClass c)
    {
        switch(c){
            case GostSandClass::IncreasedCoarseness:return "пк";
            case GostSandClass::Coarse:return "к";
            case GostSandClass::Medium:return "с";
            case GostSandClass::Small:return "м";
            case GostSandClass::VerySmall:return "ом";
            case GostSandClass::Fine:return "т";
            case GostSandClass::VeryFine:return "от";
            default:return "";
        }
    }
    std::string rockTypeLabel(const Sample& sample)
    {
        const std::string rockType = sample.getRockType();

        if (rockType == "Песок")
        {
            std::string name = sample.getGostSoilType();
            if (name.empty()) return "Песок";
            if (name.find("Песок") == std::string::npos && name.find("песок") == std::string::npos)
                name = "Песок " + name;
            return name;
        }

        if (rockType == "Супесь" || rockType == "Суглинок")
            return sample.getTkpSoilType().empty() ? rockType : sample.getTkpSoilType();

        return sample.getSoilType();
    }

    double planDistance(const Borehole& a,const Borehole& b)
    { const auto ca=a.getCoordinate(),cb=b.getCoordinate(); const double dx=cb.second-ca.second,dy=cb.first-ca.first; return std::sqrt(dx*dx+dy*dy); }
}

void DxfGeologicalSectionWriter::write(const std::string& filePath,const Deposit& deposit,const GeologicalAnalysisResult& analysisResult,const SectionSettings& sectionSettings) const
{
    DxfDocument doc;
    doc.addLayer(DxfLayer("SECTION_BOREHOLES",7));
    doc.addLayer(DxfLayer("SECTION_BOUNDARIES",7));
    doc.addLayer(DxfLayer("SECTION_SCALE",7));
    doc.addLayer(DxfLayer("SECTION_LABELS",7));
    doc.addLayer(DxfLayer("SECTION_GROUNDWATER",5,"DASHED"));
    doc.addLayer(DxfLayer("SECTION_SAMPLES",3));

    double sectionOffsetX=0.0;
    int sectionNo=1;
    for(const auto& selected:sectionSettings.getSections())
    {
        std::vector<Item> items;
        for(const auto& n:selected)
        {
            const Borehole* b=deposit.findBorehole(n);
            const auto* r=findResult(analysisResult,n);
            if(b&&r) items.push_back({b,r,0.0});
        }
        if(items.size()<2) continue;
        for(std::size_t i=1;i<items.size();++i)
            items[i].x=items[i-1].x+planDistance(*items[i-1].borehole,*items[i].borehole);

        // Каждый следующий разрез располагается справа от предыдущего.
        // Между разрезами оставляем 100 единиц чертежа.
        for(auto& item : items) item.x += sectionOffsetX;

        double maxElevation=items.front().borehole->getCollarElevation();
        double minElevation=maxElevation;
        for(const auto& it:items){maxElevation=std::max(maxElevation,it.borehole->getCollarElevation());minElevation=std::min(minElevation,it.borehole->calculateBottomElevation());}
        const double scaleTop=std::ceil(maxElevation-EPS), scaleBottom=std::floor(minElevation+EPS);
        const double originY=0.0;
        const double sx=items.front().x-SCALE_OFFSET;

        doc.addLine("SECTION_SCALE",sx,originY+(scaleTop-maxElevation)*VERTICAL_SCALE,sx,originY+(scaleBottom-maxElevation)*VERTICAL_SCALE);
        for(double e=scaleTop;e>=scaleBottom-EPS;e-=1.0){double y=originY+(e-maxElevation)*VERTICAL_SCALE;doc.addLine("SECTION_SCALE",sx,y,sx+2,y);doc.addText("SECTION_SCALE",sx-8,y-TEXT_HEIGHT/2,TEXT_HEIGHT,fmt(e,0));}
        doc.addText("SECTION_LABELS",items.front().x,originY+8,4.0,"Геолого-литологический разрез №"+std::to_string(sectionNo));

        for(const auto& it:items)
        {
            const double collar=it.borehole->getCollarElevation();
            const double x=it.x;
            doc.addLine("SECTION_BOREHOLES",x,originY+(collar-maxElevation)*VERTICAL_SCALE,x,originY+(it.borehole->calculateBottomElevation()-maxElevation)*VERTICAL_SCALE);
            doc.addText("SECTION_LABELS",x-BOREHOLE_HALF_WIDTH,originY+(collar-maxElevation)*VERTICAL_SCALE+3,2.5,"Скв. "+it.borehole->getBoreholeNumber());

            for(const auto& s:it.borehole->getSamples())
            {
                if(s.getEndDepth()<=s.getStartDepth()+EPS) continue;

                const double yTop=originY+(collar-s.getStartDepth()-maxElevation)*VERTICAL_SCALE;
                const double yBottom=originY+(collar-s.getEndDepth()-maxElevation)*VERTICAL_SCALE;
                const double mid=(s.getStartDepth()+s.getEndDepth())/2.0;
                const double y=originY+(collar-mid-maxElevation)*VERTICAL_SCALE;
                const double labelX=x+BOREHOLE_HALF_WIDTH+2;

                std::string label;
                if (s.isMonolith())
                    label = "М" + s.getSampleNumber();
                else
                    label = s.getSampleNumber();

                if (s.isAgSample())
                    label += " аг";

                label += " " + rockTypeLabel(s);
                doc.addText("SECTION_SAMPLES",labelX,y-1.0,2.0,label);

                // Для монолита показываем только кровлю и одну границу.
                if (s.isMonolith())
                {
                    doc.addLine("SECTION_BOUNDARIES",x-BOREHOLE_HALF_WIDTH,yTop,
                                x+BOREHOLE_HALF_WIDTH,yTop);
                    doc.addText("SECTION_SAMPLES",labelX,yTop+0.5,1.6,fmt(s.getStartDepth()));
                    continue;
                }

                doc.addLine("SECTION_BOUNDARIES",x-BOREHOLE_HALF_WIDTH,yTop,
                            x+BOREHOLE_HALF_WIDTH,yTop);
                doc.addLine("SECTION_BOUNDARIES",x-BOREHOLE_HALF_WIDTH,yBottom,
                            x+BOREHOLE_HALF_WIDTH,yBottom);

                // Общая граница соседних образцов подписывается только один раз.
                const auto& samples = it.borehole->getSamples();
                bool hasPreviousSample = false;
                double previousEndDepth = 0.0;
                for (const auto& previous : samples)
                {
                    if (&previous == &s)
                        break;
                    if (previous.isMonolith())
                        continue;
                    if (previous.getEndDepth() > previous.getStartDepth() + EPS)
                    {
                        hasPreviousSample = true;
                        previousEndDepth = previous.getEndDepth();
                    }
                }

                if (!hasPreviousSample ||
                    std::abs(previousEndDepth - s.getStartDepth()) > EPS)
                {
                    doc.addText("SECTION_SAMPLES",labelX,yTop+0.5,1.6,fmt(s.getStartDepth()));
                }

                doc.addText("SECTION_SAMPLES",labelX,yBottom-1.8,1.6,fmt(s.getEndDepth()));
            }

            const double gw=it.borehole->getGroundwaterDepth();
            if(gw>EPS && gw<=it.borehole->getTotalDepth()+EPS)
            {
                const double y=originY+(collar-gw-maxElevation)*VERTICAL_SCALE;
                doc.addLine("SECTION_GROUNDWATER",x-BOREHOLE_HALF_WIDTH-2,y,x+BOREHOLE_HALF_WIDTH+2,y);
                doc.addText("SECTION_GROUNDWATER",x+BOREHOLE_HALF_WIDTH+3,y+0.5,2.0,"УГВ "+fmt(gw)+" м");
            }

            const double bottomY = originY +
                (collar - it.borehole->getTotalDepth() - maxElevation) * VERTICAL_SCALE;
            doc.addText(
                "SECTION_LABELS",
                x + BOREHOLE_HALF_WIDTH + 3,
                bottomY - 2.0,
                2.0,
                "Забой " + fmt(it.borehole->getTotalDepth()) + " м");
        }
        // Переходим вправо на ширину текущего разреза и оставляем
        // между разрезами 100 единиц чертежа.
        const double sectionWidth = items.back().x - items.front().x;
        sectionOffsetX = items.back().x + 100.0;
        if(sectionWidth <= EPS) sectionOffsetX = items.front().x + 100.0;
        ++sectionNo;
    }
    doc.save(filePath);
}
