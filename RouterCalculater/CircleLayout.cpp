#include "App.h"
namespace Upp { namespace RouterApp {
using RouterCalc::Number;
static int CLLogical(int px){return int(px*100.0/DPI(100)+.5);}
static void CLPlace(Ctrl& c,int x,int y,int w,int h){c.SetRect(DPI(x),DPI(y),DPI(w),DPI(h));}
static void CLPrepare(Image& image,int w,int h,const char* fill,const char* stroke){Size sz(DPI(w),DPI(h));if(image.GetSize()!=sz)image=PanelImage(sz,fill,stroke);}
int CircleCanvas::ContentHeight(int width) const
{
    (void)width;int bottom=max(842+(notes_open?82:0),122+PREVIEW_HEIGHT+12+210)+16;
    return bottom+((expanded_formula||expanded_safety)?400:236)+34;
}
void CircleCanvas::Layout()
{
    int width=CLLogical(GetSize().cx);if(width<700)return;
    left_width=width>=1100?300:292;right_x=left_width+16;right_width=width-right_x;
    notes_extra=notes_open?82:0;diagram_height=PREVIEW_HEIGHT;table_y=122+diagram_height+12;below_y=max(842+notes_extra,table_y+210)+16;
    int half=(width-16)/2,bottom_h=(expanded_formula||expanded_safety)?400:236;
    CLPrepare(left_back,left_width,842+notes_extra,"#1b2023","#333b3e");
    CLPrepare(result_back,right_width,110,"#242923","#575039");CLPrepare(diagram_back,right_width,diagram_height,"#1b2023","#333b3e");
    CLPrepare(table_back,right_width,210,"#1b2023","#333b3e");CLPrepare(info_back,half,bottom_h,"#1b2023","#333b3e");CLPrepare(safety_back,width-half-16,bottom_h,"#26251f","#504837");
    CLPlace(radius,left_width-148,37,57,26);CLPlace(diameter,left_width-88,37,70,26);
    CLPlace(hole,18,69,left_width-36,36);CLPlace(guide,18,185,left_width-36,36);CLPlace(cutter,18,302,left_width-36,36);
    int ys[]={69,185,302};for(int i=0;i<3;i++)CLPlace(suffix[i],left_width-51,ys[i]+8,29,20);
    CLPlace(hole_slider,18,128,left_width-36,18);CLPlace(guide_slider,18,245,left_width-36,18);CLPlace(cutter_slider,18,363,left_width-36,18);
    CLPlace(cutter_picture,18,387,left_width-36,128);
    CLPlace(new_design,left_width-84,543,66,26);CLPlace(saved,18,579,left_width-36,30);CLPlace(name,54,617,left_width-72,30);
    CLPlace(note_toggle,18,655,148,22);notes.Show(notes_open);CLPlace(notes,18,682,left_width-36,74);
    int bw=(left_width-50)/3;
    CLPlace(save,18,685+notes_extra,bw,32);CLPlace(settings,25+bw,685+notes_extra,bw,32);CLPlace(export_file,32+2*bw,685+notes_extra,left_width-50-2*bw,32);
    CLPlace(show_base,right_x+right_width-247,136,58,22);CLPlace(show_path,right_x+right_width-181,136,103,22);CLPlace(show_radii,right_x+right_width-72,136,58,22);
    CLPlace(diagram,right_x+10,174,right_width-20,diagram_height-132);
    CLPlace(copy,right_x+right_width-156,table_y+12,138,26);
    CLPlace(formula_more,18,below_y+196,190,24);CLPlace(safety_more,half+34,below_y+196,198,24);
}
void CircleCanvas::DrawResult(Draw& w)
{
    w.DrawImage(DPI(right_x),0,result_back);int split=right_x+int(right_width*.53);
    w.DrawRect(DPI(split),DPI(15),DPI(1),DPI(80),Line());
    TextAt(w,right_x+18,15,"TEMPLATE OPENING · MARK-OUT RADIUS",9,Amber(),true);TextAt(w,split+18,17,"FULL DIAMETER",9,Amber(),true);
    if(!geometry.IsValid()){TextAt(w,right_x+18,35,"—",36,Coral(),true);Wrap(w,right_x+18,77,right_width-36,geometry.error,10,Coral());return;}
    TextAt(w,right_x+18,57,"R",14,Amber(),true);String value=Number(geometry.templateR/Factor());TextAt(w,right_x+42,34,value,42,Amber(),true);
    int dx=CLLogical(GetTextSize(value,TextFont(42,true)).cx);TextAt(w,right_x+49+dx,59,unit,14,Amber(),true);
    TextAt(w,right_x+18,87,"Centre to the inside template edge.",10,Muted());
    TextAt(w,split+18,42,"Ø "+Number(geometry.templateR*2/Factor())+" "+unit,23,Ink(),true);
    double af=unit=="mm"?10:1;String au=unit=="mm"?"cm":"mm";
    TextAt(w,split+18,76,"R "+Number(geometry.templateR/af)+" "+au+" · Ø "+Number(geometry.templateR*2/af)+" "+au,11,Muted(),true);
}
void CircleCanvas::DrawTable(Draw& w)
{
    w.DrawImage(DPI(right_x),DPI(table_y),table_back);TextAt(w,right_x+18,table_y+17,"All dimensions",13,Ink(),false,true);
    const char* titles[]={"Circle","R (mm)","Ø (mm)","R (cm)","Ø (cm)"};int labelw=max(153,int(right_width*.35)),colw=(right_width-labelw-36)/4;
    TextAt(w,right_x+18,table_y+48,titles[0],10,Muted(),true);for(int c=1;c<5;c++)TextAt(w,right_x+labelw+colw*(c-1),table_y+48,titles[c],10,Muted(),true);
    const char* labels[]={"Template opening","Finished hole","Router base / guide","Cutter","Cutter-centre path"};Color colours[]={Amber(),Teal(),Blue(),Coral(),Muted()};
    double radii[]={geometry.templateR,geometry.holeR,geometry.baseR,geometry.bitR,geometry.pathR};
    for(int r=0;r<5;r++){
        int y=table_y+69+r*26;w.DrawRect(DPI(right_x+18),DPI(y),DPI(right_width-36),DPI(1),Line());
        w.DrawEllipse(DPI(right_x+18),DPI(y+11),DPI(5),DPI(5),colours[r]);TextAt(w,right_x+30,y+7,labels[r],11);
        double vals[]={radii[r],radii[r]*2,radii[r]/10,radii[r]/5};for(int c=0;c<4;c++)TextAt(w,right_x+labelw+colw*c,y+7,geometry.IsValid()?Number(vals[c]):String("—"),11,Ink(),true);
    }
}
void CircleCanvas::Paint(Draw& w)
{
    int width=CLLogical(GetSize().cx),half=(width-16)/2;w.DrawRect(GetSize(),Bg());w.DrawImage(0,0,left_back);DrawResult(w);w.DrawImage(DPI(right_x),DPI(122),diagram_back);DrawTable(w);
    TextAt(w,18,16,"Your measurements",14,Ink(),false,true);TextAt(w,18,44,"Finished hole",11,Ink(),false,true);
    TextAt(w,18,110,"R "+Measure(input.holeRadiusMm)+" · Ø "+Measure(input.holeRadiusMm*2),10,Muted());
    TextAt(w,18,163,"Router base / guide",11,Ink(),false,true);TextAt(w,left_width-87,164,"DIAMETER",9,Muted(),true);
    TextAt(w,18,226,"Circular edge at the template contact height.",10,Muted());
    TextAt(w,18,280,"Router cutter",11,Ink(),false,true);TextAt(w,left_width-87,281,"DIAMETER",9,Muted(),true);
    TextAt(w,18,344,"Cutting diameter — not the shank diameter.",10,Muted());
    for(int y:{151,268})w.DrawRect(DPI(18),DPI(y),DPI(left_width-36),DPI(1),Line());
    TextAt(w,18,524,"Cutter R = "+Measure(input.cutterDiameterMm)+" ÷ 2 = "+Measure(input.cutterDiameterMm/2),10,Coral(),true);
    TextAt(w,18,554,"SAVED DESIGNS",10,Muted(),true);TextAt(w,18,625,"Name",10,Muted());
    Wrap(w,18,729+notes_extra,left_width-36,status,10,dirty?Amber():Teal());
    Wrap(w,18,771+notes_extra,left_width-36,"Save stores your setups in the adjacent JSON library. Export creates and links a portable copy.",10,Muted());
    Wrap(w,18,812+notes_extra,left_width-36,"162 mm is provisional. Verify the actual circular contact edge.",10,Amber());
    TextAt(w,right_x+18,136,"How the circles fit",14,Ink(),false,true);TextAt(w,right_x+18,157,"TOP VIEW · FIXED AT 0° · NOT PRINT SCALE",9,Muted(),true);
    int ly=122+diagram_height-60,lx=right_x+18;const char* legend[]={"Template opening","Finished hole","Router base","Cutter","Centre path"};Color lc[]={Amber(),Teal(),Blue(),Coral(),Muted()};
    for(int i=0;i<5;i++){w.DrawRect(DPI(lx),DPI(ly+6),DPI(14),DPI(2),lc[i]);TextAt(w,lx+20,ly,legend[i],10,Muted());lx+=CLLogical(GetTextSize(legend[i],TextFont(10)).cx)+33;}
    Wrap(w,right_x+18,122+diagram_height-34,right_width-36,"Ideal concentric guide. Flat edges and taper are not compensated; measure at the actual contact height.",10,Muted());
    w.DrawImage(0,DPI(below_y),info_back);w.DrawImage(DPI(half+16),DPI(below_y),safety_back);
    TextAt(w,18,below_y+16,"Why this works",13,Ink(),false,true);w.DrawRect(DPI(18),DPI(below_y+42),DPI(half-36),DPI(60),Field());
    TextAt(w,28,below_y+53,"Template R = hole R + guide R − cutter R",12,Ink(),true);
    String equation=geometry.IsValid()?Number(geometry.templateR/Factor())+" = "+Number(geometry.holeR/Factor())+" + "+Number(geometry.baseR/Factor())+" − "+Number(geometry.bitR/Factor())+" "+unit:"Enter valid dimensions.";TextAt(w,28,below_y+79,equation,11,Amber(),true);
    String explanation=geometry.IsValid()?"The cutter centre travels "+Measure(geometry.pathR)+" from the common centre. The cutting radius reaches the finished hole; the guide radius reaches the template opening.":"No previous result is shown while measurements are invalid.";Wrap(w,18,below_y+118,half-36,explanation,11,Muted());
    if(expanded_formula)Wrap(w,18,below_y+240,half-36,"Template Ø = hole Ø + guide Ø − cutter Ø. The result is the template's inside opening, not its board edge or the tool centreline used to make it. Perimeter routing may leave a central waste piece; support it separately.",11,Muted());
    TextAt(w,half+34,below_y+16,"Before using this as a cutting dimension",13,Amber(),false,true);
    int y=Wrap(w,half+34,below_y+46,width-half-52,"A flat or tapered base changes the centre-to-contact distance. Use a genuinely concentric circular guide surface.",11,Muted());
    Wrap(w,half+34,y+12,width-half-52,"Unplug before measuring. Secure and support the template and workpiece. Make a test cut in scrap before committing the useful wood.",11,Muted());
    if(expanded_safety)Wrap(w,half+34,below_y+240,width-half-52,"The guide and cutter defaults are working measurements, not certified Bosch dimensions. Cutter width is different from shank size. Plunge depth is perpendicular to the top view and does not alter this equation for a straight cutter.",11,Muted());
    int footer=below_y+((expanded_formula||expanded_safety)?400:236)+16;TextAt(w,0,footer,"UPP ROUTER CALCULATOR · NATIVE C++ · LOCAL JSON",9,Muted(),true);
    TextAt(w,width-288,footer,"Calculated dimensions, not a machining tolerance.",9,Muted());
}
} }
