#include "App.h"
#include <cmath>
namespace Upp { namespace RouterApp {
Font TextFont(int px,bool mono,bool bold)
{
    static const int mono_face=Font::FindFaceNameIndex("Consolas");
    static const int sans_face=Font::FindFaceNameIndex("Segoe UI");
    int face=mono?mono_face:sans_face;
    Font f(face>=0?face:Font::GetStdFont().GetFace(),DPI(px));
    return bold ? f.Bold() : f;
}
void StyleButton(UiButton& b,const String& text,bool selected)
{
    UiButton::Style s = UiButton::StyleDefault();
    s.font=TextFont(12); s.metrics.radius=DPI(7); s.metrics.frame_width=DPI(1);
    s.metrics.content_margin=Rect(DPI(8),DPI(3),DPI(8),DPI(3));
    s.metrics.shadow.enabled=false; s.metrics.focus_color=Teal();
    b.SetCustomStyle(s).SetText(text);
    b.SetBaseColors(selected ? ThemeColor("#323b3e") : Panel(), selected ? Amber() : Line(), selected ? Ink() : Muted(), 8, 12);
}
void StyleEdit(UiBaseEdit& e,int px)
{
    UiBaseEdit::Style s=UiBaseEdit::StyleDefault();
    s.font=TextFont(px,px>18); s.metrics.radius=DPI(7); s.metrics.shadow.enabled=false;
    s.metrics.content_margin=Rect(DPI(px>18?11:9),DPI(3),DPI(px>18?38:9),DPI(3));
    s.caret_color=Teal(); s.selection_color=IsDark()?Color(51,90,80):Color(187,220,210); s.selection_ink=Ink(); s.placeholder_ink=Muted();
    e.SetCustomStyle(s).SetBaseColors(Field(),Line(),Ink(),0,0).SetFocusColor(Teal());
}
void StyleCheck(UiCheckBox& c,const String& text)
{
    UiCheckBox::Style s=UiCheckBox::StyleDefault();
    s.font=TextFont(11); s.indicator_size=DPI(13); s.indicator_gap=DPI(5);
    s.metrics.face_enabled=false; s.metrics.frame_enabled=false; s.metrics.shadow.enabled=false;
    s.indicator_metrics.radius=DPI(2);s.indicator_metrics.shadow.enabled=false;
    for(int i=0;i<4;i++){s.palette.ink[i]=Muted();s.indicator_palette.ink[i]=Bg();s.indicator_palette.frame[i]=Blue();s.indicator_palette.face[i]=Blue();}
    c.SetCustomStyle(s).SetText(text);
}
void StyleDropdown(UiDropdown& d)
{
    UiDropdown::Style s=UiDropdown::StyleDefault();
    // The control mono-tints its indicator using icon[], independently of ink[].
    for(int i=0;i<4;i++)s.palette.icon[i]=Ink();
    s.font=TextFont(12);s.metrics.radius=DPI(7);s.metrics.shadow.enabled=false;
    s.metrics.content_margin=Rect(DPI(9),DPI(4),DPI(9),DPI(4));
    s.popup_item_style.font=TextFont(12);s.popup_background_color=Panel();s.popup_frame_color=Line();
    for(int i=0;i<4;i++){s.popup_item_style.palette.ink[i]=Ink();s.popup_item_style.palette.face[i]=Panel();}
    d.SetCustomStyle(s).SetBaseColors(Field(),Line(),Ink(),5,8);
    d.ShowDragHandle(false).ShowSelectionBadge(false).SetPopupItemHeight(DPI(32));
}
void TextAt(Draw& w,int x,int y,const String& text,int px,Color ink,bool mono,bool bold)
{
    w.DrawText(DPI(x),DPI(y),text,TextFont(px,mono,bold),IsNull(ink)?Ink():ink);
}
int Wrap(Draw& w,int x,int y,int width,const String& text,int px,Color ink)
{
    Font f=TextFont(px); String line;
    for(const String& word:Split(text,' ')) {
        String next=line.IsEmpty()?word:line+" "+word;
        if(!line.IsEmpty() && GetTextSize(next,f).cx>DPI(width)) {
            TextAt(w,x,y,line,px,ink);y+=px+6;line=word;
        } else line=next;
    }
    if(!line.IsEmpty()) {TextAt(w,x,y,line,px,ink);y+=px+6;}
    return y;
}
static String N(double v){return RouterCalc::Number(v);}
static String SvgStart(int width,int height)
{
    return Format("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\">",width,height,width,height);
}
static String T(double x,double y,const String& text,const char* colour,int size=12,bool mono=false)
{
    return "<text x=\""+N(x)+"\" y=\""+N(y)+"\" font-family=\""+(mono?"Consolas":"Segoe UI")+"\" font-size=\""+N(size)+"\" fill=\""+colour+"\">"+DeXml(text)+"</text>";
}
static String L(double x1,double y1,double x2,double y2,const char* colour,double width=1,const char* extra="")
{
    return "<line x1=\""+N(x1)+"\" y1=\""+N(y1)+"\" x2=\""+N(x2)+"\" y2=\""+N(y2)+"\" stroke=\""+colour+"\" stroke-width=\""+N(width)+"\" "+extra+"/>";
}
static String C(double x,double y,double r,const char* stroke,const char* fill="none",const char* extra="")
{
    return "<circle cx=\""+N(x)+"\" cy=\""+N(y)+"\" r=\""+N(max(0.0,r))+"\" stroke=\""+stroke+"\" fill=\""+fill+"\" "+extra+"/>";
}
String CircleSvg(const RouterCalc::Geometry& g,const String& unit,bool base_on,bool path_on,bool radii_on)
{
    String s=SvgStart(840,450);s<<"<rect width=\"840\" height=\"450\" rx=\"7\" fill=\"#191f22\"/>";
    if(!g.IsValid())return s+T(250,220,"Enter valid dimensions to see the geometry.","#9ca8aa",15)+"</svg>";
    auto M=[&](double mm){return N(mm/(unit=="cm"?10:1))+" "+unit;};
    double k=185/g.templateR, h=g.holeR*k,b=g.baseR*k,c=g.bitR*k,p=g.pathR*k;
    double cx=378,cy=225,bx=cx+p,tx=cx+185;
    s<<C(cx,cy,196,"#68583d","#292a25","stroke-width=\"0.7\"");
    s<<C(cx,cy,185,"#e6b972","#191f22","stroke-width=\"2\"");
    s<<C(cx,cy,h,"#80d6bb","#24423a","fill-opacity=\".43\" stroke-width=\"1.5\"");
    if(base_on)s<<C(bx,cy,b,"#91b8db","#91b8db","fill-opacity=\".1\" stroke-width=\"1.5\"");
    if(path_on&&p>0)s<<C(cx,cy,p,"#99a6ae","none","stroke-dasharray=\"5 5\"");
    if(base_on)s<<L(cx,cy,tx,cy,"#758c97",1,"stroke-dasharray=\"4 5\" opacity=\".6\"");
    s<<C(bx,cy,c,"#f09987","#f09987","fill-opacity=\".5\" stroke-width=\"1.6\"");
    s<<L(bx-c*.65,cy-c*.6,bx+c*.65,cy+c*.6,"#ffd1c4")<<L(bx-c*.65,cy+c*.6,bx+c*.65,cy-c*.6,"#ffd1c4");
    s<<L(cx-5,cy,cx+5,cy,"#ecede8")<<L(cx,cy-5,cx,cy+5,"#ecede8")<<T(cx-12,cy+18,"O","#d8deda",11,true);
    if(base_on)s<<C(tx,cy,3.5,"#e6b972","#191f22");
    s<<L(cx-185*.67,cy-185*.742,169,74,"#e6b972",.85)<<L(169,74,47,74,"#e6b972",.85);
    s<<T(47,54,"Template opening","#e6b972")<<T(47,69,"R "+M(g.templateR),"#e6b972",11,true);
    s<<L(cx-h*.71,cy-h*.704,167,162,"#80d6bb",.85)<<L(167,162,47,162,"#80d6bb",.85);
    s<<T(47,142,"Finished hole","#80d6bb")<<T(47,157,"R "+M(g.holeR),"#80d6bb",11,true);
    if(base_on){s<<L(bx+b*std::cos(-.65),cy+b*std::sin(-.65),625,80,"#91b8db",.85)<<L(625,80,648,80,"#91b8db",.85);s<<T(650,59,"Router base","#91b8db")<<T(650,77,"Ø "+M(g.baseR*2),"#91b8db",11,true);}
    if(radii_on){
        double ux=std::cos(2.12),uy=std::sin(2.12),ex=cx+185*ux,ey=cy+185*uy;
        s<<L(cx,cy,ex,ey,"#e6b972")<<L(ex-uy*5,ey+ux*5,ex+uy*5,ey-ux*5,"#e6b972");
        s<<T(cx+124*ux-100,cy+124*uy,"R "+M(g.templateR),"#e6b972",11,true);
        s<<L(cx,cy,cx-h,cy,"#80d6bb")<<L(cx-h,cy-4,cx-h,cy+4,"#80d6bb");
        if(h>25)s<<T(cx-h*.8,cy-10,"R "+N(g.holeR/(unit=="cm"?10:1)),"#80d6bb",11,true);
    }
    s<<"<rect x=\"630\" y=\"184\" width=\"182\" height=\"210\" rx=\"10\" fill=\"#1c2428\" stroke=\"#38444a\"/>";
    s<<T(645,207,"CUTTER DETAIL","#a5b3b8",10,true)<<T(645,224,"Magnified · not to scale","#7e909a",10);
    s<<C(693,295,29,"#f09987","#543c36")<<L(673,275,713,315,"#f09987",2)<<L(673,315,713,275,"#f09987",2);
    s<<L(722,254,722,332,"#80d6bb",1.7)<<L(693,295,722,295,"#ecede8",1.1);
    s<<T(735,282,"Hole","#80d6bb")<<T(735,299,"edge","#80d6bb");
    s<<T(645,350,"Ø "+M(g.bitR*2)+" ÷ 2","#f09987",11,true)<<T(645,369,"R "+M(g.bitR),"#f09987",11,true)<<T(645,386,"Centre → cutting edge","#819398",10);
    s<<L(bx,cy,610,286,"#f09987",.8,"stroke-dasharray=\"3 5\" opacity=\".55\"")<<L(610,286,630,286,"#f09987",.8);
    s<<T(47,415,"Cutter-centre travel R "+M(g.pathR),"#9cadb5",11,true)<<T(650,435,"O = common centre","#71858f",10);
    return s+"</svg>";
}
String CutterSvg(double d,const String& unit)
{
    String s=SvgStart(294,156);
    s<<"<rect x=\".5\" y=\".5\" width=\"293\" height=\"155\" rx=\"8\" fill=\"#171d20\" stroke=\"#333b3e\"/>";
    s<<T(12,20,"MEASURE THE CUTTING WIDTH","#a0adb1",9,true);
    s<<"<rect x=\"54\" y=\"29\" width=\"18\" height=\"52\" rx=\"3\" fill=\"#7c898e\" stroke=\"#b1bdc2\"/><path d=\"M43 77H83V120H43Z\" fill=\"#543c36\" stroke=\"#f09987\"/><path d=\"M44 82L64 94L44 112 M82 82L63 103L82 116\" fill=\"none\" stroke=\"#f09987\" stroke-width=\"2\"/>";
    s<<L(72,48,100,48,"#7e909a")<<T(108,51,"Shank — not this width","#99a6ae",10);
    s<<L(83,92,100,92,"#f09987")<<T(108,92,"Outside cutting edges","#f09987",11)<<T(108,109,"Side view · not to scale","#7e909a",10);
    s<<L(43,122,43,145,"#f09987")<<L(83,122,83,145,"#f09987")<<L(43,138,83,138,"#f09987");
    s<<L(43,138,49,135,"#f09987")<<L(43,138,49,141,"#f09987")<<L(83,138,77,135,"#f09987")<<L(83,138,77,141,"#f09987");
    s<<T(108,142,"Ø "+N(d/(unit=="cm"?10:1))+" "+unit,"#f09987",12,true);
    return s+"</svg>";
}
} }
