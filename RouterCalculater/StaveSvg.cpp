#include "StavePage.h"
#include <cmath>
namespace Upp { namespace RouterApp {
static String SN(double n){return RouterCalc::Number(n);}
static String ST(double x,double y,const String& text,const char* color,int size=12){return "<text x=\""+SN(x)+"\" y=\""+SN(y)+"\" font-family=\"Segoe UI\" font-size=\""+SN(size)+"\" fill=\""+color+"\">"+DeXml(text)+"</text>";}
static String SL(double x,double y,double xx,double yy,const char* color,const char* extra=""){return "<line x1=\""+SN(x)+"\" y1=\""+SN(y)+"\" x2=\""+SN(xx)+"\" y2=\""+SN(yy)+"\" stroke=\""+color+"\" "+extra+"/>";}
static String SR(double x,double y,double w,double h,const char* fill,const char* stroke){return "<rect x=\""+SN(x)+"\" y=\""+SN(y)+"\" width=\""+SN(w)+"\" height=\""+SN(max(0.0,h))+"\" fill=\""+fill+"\" stroke=\""+stroke+"\"/>";}
String StaveSvg(const RouterCalc::StaveInput& x,const RouterCalc::StaveGeometry& g,const String& unit,int selected)
{
    String s="<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"840\" height=\"450\" viewBox=\"0 0 840 450\">";s<<SR(0,0,840,450,"#191f22","none");
    if(!g.IsValid())return s+ST(210,218,"Enter valid shell dimensions to see the geometry.","#9ca8aa",15)+"</svg>";
    auto m=[&](double n){return SN(n/(unit=="cm"?10:1))+" "+unit;};
    const double cx=276,cy=218,k=163/g.maximumBlankRadiusMm;
    auto polygon=[&](const Pointf* v,const char* fill,const char* stroke){String p="<path d=\"";for(int j=0;j<4;j++)p<<(j?"L":"M")<<SN(cx+v[j].x*k)<<" "<<SN(cy-v[j].y*k);p<<"Z\" fill=\""<<fill<<"\" stroke=\""<<stroke<<"\" stroke-width=\".9\"/>";return p;};
    for(const auto& p:g.staves)s<<polygon(p.vertices,p.index==selected+1?"#68583d":"#24423a",p.index==selected+1?"#e6b972":"#91b8db");
    for(const auto& p:g.spacers)s<<polygon(p.vertices,"#f09987","#f09987");
    for(int i=0;i<2;i++)s<<"<circle cx=\""<<SN(cx)<<"\" cy=\""<<SN(cy)<<"\" r=\""<<SN((i?g.innerRadiusMm:x.outerRadiusMm)*k)<<"\" fill=\"none\" stroke=\""<<(i?"#80d6bb":"#e6b972")<<"\" stroke-width=\"1.5\"/>";
    s<<SL(cx-6,cy,cx+6,cy,"#ecede8")<<SL(cx,cy-6,cx,cy+6,"#ecede8")<<ST(cx-16,cy+20,"O","#ecede8",11);
    s<<SL(cx,cy,cx+x.outerRadiusMm*k,cy,"#e6b972","stroke-dasharray=\"4 5\"")<<ST(cx+20,cy-9,"R "+m(x.outerRadiusMm),"#e6b972",11);
    if(x.count<=48)for(int i=0;i<x.count;i++){double a=i*6.283185307179586/x.count;int tx=int(cx+180*std::cos(a)),ty=int(cy-180*std::sin(a));s<<ST(tx-4,ty+4,AsString(i+1),"#7e909a",10);}
    s<<ST(18,24,"FINISHED SHELL + GLUE-UP STOCK","#a5b3b8",11)<<ST(18,44,"Outside Ø "+m(x.outerRadiusMm*2)+" · inside Ø "+m(g.innerRadiusMm*2),"#9ca8aa",11);
    s<<ST(18,415,"Bevel "+SN(g.bevelDegrees)+"° from square · division "+SN(g.divisionDegrees)+"°","#e6b972",12);
    s<<ST(18,436,"O → outer flat "+m(g.outerApothemMm)+" · inner flat "+m(g.innerApothemMm),"#9ca8aa",11);
    s<<SL(501,24,501,426,"#333b3e");
    s<<ST(525,24,"DEPTH & MOUNTING END","#a5b3b8",11)<<ST(525,43,"Schematic · not a tool-clearance simulation","#7e909a",10);
    double bottom=330,scale=245/g.blankLengthMm,top=bottom-g.blankLengthMm*scale;
    double finish_top=bottom-g.finishedTopZMm*scale,finish_bottom=bottom-g.finishedBottomZMm*scale;
    s<<SR(551,top,106,245,"#292a25","#91b8db");
    s<<SR(551,finish_top,106,finish_bottom-finish_top,"#24423a","#80d6bb");
    double gap_top=bottom-x.reachGapMm*scale;
    if(x.reachGapMm>0)s<<SR(551,gap_top,106,bottom-gap_top,"#543c36","#f09987");
    for(int i=1;i<5;i++)s<<SL(551+i*106.0/5,top,551+i*106.0/5,bottom,"#758c97");
    s<<SL(604,top-10,604,bottom+30,"#9ca8aa","stroke-dasharray=\"4 4\"");
    if(x.basePlateMm>0)s<<SR(539,bottom+3,130,18,"#7c898e","#b1bdc2");
    s<<SL(661,finish_top,684,finish_top,"#80d6bb")<<SL(661,finish_bottom,684,finish_bottom,"#80d6bb")<<SL(679,finish_top,679,finish_bottom,"#80d6bb");
    s<<ST(691,(finish_top+finish_bottom)/2-6,"Finished depth","#80d6bb",11)<<ST(691,(finish_top+finish_bottom)/2+12,m(x.depthMm),"#80d6bb",13);
    s<<ST(525,67,"Cut each length: "+m(g.blankLengthMm),"#e6b972",12);
    s<<ST(525,369,"No-go above plate: "+m(x.reachGapMm),"#f09987",11);
    s<<ST(525,388,"Plate thickness: "+m(x.basePlateMm)+" (separate)","#9ca8aa",11);
    s<<ST(525,407,"Finish starts at Z = "+m(g.finishedBottomZMm),"#80d6bb",11);
    s<<ST(525,426,"Z 0 = plate top · +Z points into the shell","#7e909a",10);
    return s+"</svg>";
}
} }
