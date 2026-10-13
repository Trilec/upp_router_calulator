#include "Diagrams.h"
#include <cmath>
namespace Upp { namespace RouterApp {
using RouterCalc::Number;
DiagramScene CircleDiagram(const RouterCalc::Geometry& g, const String& unit, bool base, bool path, bool radii)
{
    DiagramScene s; s.extent = Sizef(720, 270);
    s.Box(0, 0, 720, 270, "#191f22", "none", 7);
    if(!g.IsValid() || g.templateR <= 0) return s;
    auto m = [&](double value) { return Number(value / (unit == "cm" ? 10 : 1)) + " " + unit; };
    // Normalised ratios: physical dimensions never become buffer dimensions.
    const double cx = 285, cy = 128, radius = 107;
    const double h = radius * (g.holeR / g.templateR), b = radius * (g.baseR / g.templateR);
    const double c = radius * (g.bitR / g.templateR), travel = radius * (g.pathR / g.templateR);
    const double bx = cx + travel;
    s.Circle(cx, cy, radius + 6, "#68583d", "#292a25", .7);
    s.Circle(cx, cy, radius, "#e6b972", "#191f22", 1.5);
    s.Circle(cx, cy, h, "#80d6bb", "#24423a", 1.2, .43);
    if(base) s.Circle(bx, cy, b, "#91b8db", "#91b8db", 1.2, .1);
    if(path && travel > 0) s.Circle(cx, cy, travel, "#99a6ae", "none", .8, 1, true);
    if(base) s.Line(cx, cy, cx + radius, cy, "#758c97", 1, true);
    s.Circle(bx, cy, c, "#f09987", "#f09987", 1.2, .5);
    s.Line(bx-c*.65,cy-c*.65,bx+c*.65,cy+c*.65,"#ffd1c4");
    s.Line(bx-c*.65,cy+c*.65,bx+c*.65,cy-c*.65,"#ffd1c4");
    s.Line(cx-4,cy,cx+4,cy,"#ecede8");s.Line(cx,cy-4,cx,cy+4,"#ecede8");
    s.Text(cx-13,cy+17,"O","#d8deda",12);
    s.Text(14,35,"Template opening","#e6b972",13);s.Text(14,54,"R "+m(g.templateR),"#e6b972",12);
    s.Line(14,63,140,63,"#e6b972");s.Line(140,63,cx-radius*.78,cy-radius*.63,"#e6b972");
    s.Text(14,124,"Finished hole","#80d6bb",13);s.Text(14,143,"R "+m(g.holeR),"#80d6bb",12);
    s.Line(14,152,143,152,"#80d6bb");s.Line(143,152,cx-h,cy,"#80d6bb");
    if(base) {
        s.Text(503,30,"Router guide Ø "+m(g.baseR*2),"#91b8db",13);
        s.Line(bx+b*.72,cy-b*.69,466,40,"#91b8db");s.Line(466,40,500,40,"#91b8db");
    }
    if(radii) {
        s.Line(cx,cy,cx-radius*.53,cy+radius*.848,"#e6b972");
        s.Line(cx,cy,cx-h,cy,"#80d6bb");
    }
    s.Box(502,65,203,172,"#1c2428","#38444a",7);
    s.Text(514,85,"CUTTER DETAIL","#a5b3b8",12);
    s.Text(514,103,"Magnified · not to scale","#7e909a",11);
    s.Circle(546,146,23,"#f09987","#543c36");
    s.Line(530,130,562,162,"#f09987",1.5);s.Line(530,162,562,130,"#f09987",1.5);
    s.Line(569,115,569,177,"#80d6bb",1.5);s.Line(546,146,569,146,"#ecede8");
    s.Text(580,143,"Finished","#80d6bb",12);s.Text(580,161,"hole edge","#80d6bb",12);
    s.Text(514,202,"Ø "+m(g.bitR*2)+" ÷ 2","#f09987",12);
    s.Text(514,223,"Cutting R "+m(g.bitR),"#f09987",12);
    s.Line(bx,cy,478,155,"#f09987",.8,true);s.Line(478,155,502,155,"#f09987");
    s.Text(14,259,"Cutter-centre travel R "+m(g.pathR),"#9cadb5",12);
    s.Text(510,259,"O = common centre","#71858f",11);
    return s;
}
DiagramScene CutterDiagram(double d,const String& unit)
{
    DiagramScene s;s.extent=Sizef(294,156);s.Box(.5,.5,293,155,"#171d20","#333b3e",8);
    s.Text(12,20,"MEASURE THE CUTTING WIDTH","#a0adb1",9,true);
    s.Box(54,29,18,52,"#7c898e","#b1bdc2",3);s.Box(43,77,40,43,"#543c36","#f09987");
    s.Line(44,82,64,94,"#f09987",2);s.Line(64,94,44,112,"#f09987",2);s.Line(82,82,63,103,"#f09987",2);s.Line(63,103,82,116,"#f09987",2);
    s.Line(72,48,100,48,"#7e909a");s.Text(108,51,"Shank — not this width","#99a6ae",10);
    s.Line(83,92,100,92,"#f09987");s.Text(108,92,"Outside cutting edges","#f09987",11);s.Text(108,109,"Side view · not to scale","#7e909a",10);
    s.Line(43,122,43,145,"#f09987");s.Line(83,122,83,145,"#f09987");s.Line(43,138,83,138,"#f09987");
    s.Line(43,138,49,135,"#f09987");s.Line(43,138,49,141,"#f09987");s.Line(83,138,77,135,"#f09987");s.Line(83,138,77,141,"#f09987");
    s.Text(108,142,"Ø "+Number(d/(unit=="cm"?10:1))+" "+unit,"#f09987",12,true);return s;
}
} }
