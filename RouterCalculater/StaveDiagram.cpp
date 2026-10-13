#include "Diagrams.h"
#include <cmath>
namespace Upp { namespace RouterApp {
using RouterCalc::Number;
DiagramScene StaveDiagram(const RouterCalc::StaveInput& x, const RouterCalc::StaveGeometry& g, const String& unit, int selected)
{
    DiagramScene s; s.extent = Sizef(720, 270); s.Box(0,0,720,270,"#191f22");
    if(!g.IsValid() || g.maximumBlankRadiusMm <= 0 || g.blankLengthMm <= 0) return s;
    auto m = [&](double value) { return Number(value / (unit == "cm" ? 10 : 1)) + " " + unit; };
    const double cx = 174, cy = 128, radius = 103;
    auto polygon = [&](const Pointf* vertices, const char* fill, const char* stroke) {
        Pointf points[4];
        for(int j = 0; j < 4; ++j)
            points[j] = Pointf(cx + radius * (vertices[j].x / g.maximumBlankRadiusMm),
                              cy - radius * (vertices[j].y / g.maximumBlankRadiusMm));
        s.Polygon(points,fill,stroke,.9);
    };
    for(const auto& part : g.staves)
        polygon(part.vertices,part.index==selected+1?"#68583d":"#24423a",part.index==selected+1?"#e6b972":"#91b8db");
    for(const auto& strip : g.spacers) polygon(strip.vertices,"#f09987","#f09987");
    s.Circle(cx,cy,radius*(x.outerRadiusMm/g.maximumBlankRadiusMm),"#e6b972","none",1.5);
    s.Circle(cx,cy,radius*(g.innerRadiusMm/g.maximumBlankRadiusMm),"#80d6bb","none",1.5);
    s.Line(cx-4,cy,cx+4,cy,"#ecede8");s.Line(cx,cy-4,cx,cy+4,"#ecede8");s.Text(cx-13,cy+18,"O","#ecede8",12);
    s.Line(cx,cy,cx+radius*(x.outerRadiusMm/g.maximumBlankRadiusMm),cy,"#e6b972",1,true);
    s.Text(cx+8,cy-11,"R "+m(x.outerRadiusMm),"#e6b972",12);
    if(x.count <= 40) for(int i = 0; i < x.count; ++i) {
        const double angle = i * 6.283185307179586 / x.count;
        s.Text(cx+116*std::cos(angle)-4,cy-116*std::sin(angle)+4,AsString(i+1),"#7e909a",10);
    }
    s.Text(12,259,"Bevel "+Number(g.bevelDegrees)+"° · division "+Number(g.divisionDegrees)+"°","#e6b972",12);
    s.Line(342,12,342,257,"#333b3e");
    s.Text(365,23,"DEPTH & MOUNTING","#a5b3b8",13);
    s.Text(365,43,"Cut length "+m(g.blankLengthMm),"#e6b972",12);
    const double top = 61, bottom = 220, height = bottom-top;
    const double finish_top = bottom-height*(g.finishedTopZMm/g.blankLengthMm);
    const double finish_bottom = bottom-height*(g.finishedBottomZMm/g.blankLengthMm);
    s.Box(376,top,84,height,"#292a25","#91b8db");
    s.Box(376,finish_top,84,finish_bottom-finish_top,"#24423a","#80d6bb");
    const double gap_top = bottom-height*(x.reachGapMm/g.blankLengthMm);
    if(x.reachGapMm>0) s.Box(376,gap_top,84,bottom-gap_top,"#543c36","#f09987");
    for(int i=1;i<4;i++) s.Line(376+i*21,top,376+i*21,bottom,"#758c97");
    s.Line(418,top-6,418,bottom+17,"#9ca8aa",1,true);
    if(x.basePlateMm>0) s.Box(366,bottom+3,104,12,"#7c898e","#b1bdc2");
    s.Line(464,finish_top,480,finish_top,"#80d6bb");s.Line(464,finish_bottom,480,finish_bottom,"#80d6bb");
    s.Line(476,finish_top,476,finish_bottom,"#80d6bb");
    s.Text(495,83,"Finished depth","#80d6bb",13);s.Text(495,105,m(x.depthMm),"#80d6bb",14);
    s.Text(495,144,"End gap "+m(x.reachGapMm),"#f09987",12);
    s.Text(495,166,"Plate "+m(x.basePlateMm)+" (separate)","#9ca8aa",12);
    s.Text(495,191,"Finish starts at Z "+m(g.finishedBottomZMm),"#80d6bb",12);
    s.Text(495,218,"Schematic · not tool clearance","#7e909a",11);
    s.Text(365,259,"Z 0 = plate top · +Z into the shell","#7e909a",11);
    return s;
}
} }
