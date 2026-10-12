#include "StavePage.h"
namespace Upp { namespace RouterApp {
using RouterCalc::Number;
static int SLogical(int px){return int(px*100.0/DPI(100)+.5);}
static void SPlace(Ctrl& c,int x,int y,int w,int h){c.SetRect(DPI(x),DPI(y),DPI(w),DPI(h));}
static void SPanel(Image& image,int w,int h,const char* fill,const char* line){Size sz(DPI(w),DPI(h));if(image.GetSize()!=sz)image=PanelImage(sz,fill,line);}
void StaveCanvas::Layout()
{
    int width=SLogical(GetSize().cx);if(width<700)return;
    left_width=320;right_x=336;right_width=width-right_x;int half=(left_width-48)/2;
    SPanel(left_back,left_width,966,"#1b2023","#333b3e");SPanel(result_back,right_width,110,"#242923","#575039");
    SPanel(preview_back,right_width,PREVIEW_HEIGHT,"#1b2023","#333b3e");SPanel(list_back,right_width,350,"#1b2023","#333b3e");
    SPlace(radius,left_width-148,37,57,26);SPlace(diameter,left_width-88,37,70,26);SPlace(fields[OUTER],18,69,left_width-36,36);
    const int ys[]={138,200,303,365,427,533};
    const int ids[][2]={{COUNT,WALL},{OUT_ALLOW,IN_ALLOW},{DEPTH,TOP_TRIM},{REACH_GAP,BOTTOM_TRIM},{PLATE,KERF},{SPACER,EVERY}};
    for(int row=0;row<6;row++)for(int c=0;c<2;c++)SPlace(fields[ids[row][c]],18+c*(half+12),ys[row],half,30);
    SPlace(joints,18,596,left_width-36,30);SPlace(create,left_width-84,648,66,26);SPlace(saved,18,690,left_width-36,30);
    SPlace(name,18,750,left_width-36,30);SPlace(notes,18,791,left_width-36,30);
    int bw=(left_width-50)/3;SPlace(save,18,833,bw,32);SPlace(settings,25+bw,833,bw,32);SPlace(export_file,32+2*bw,833,left_width-50-2*bw,32);
    SPlace(inspect,right_x+right_width-202,136,184,26);SPlace(preview,right_x+10,175,right_width-20,PREVIEW_HEIGHT-124);
}
void StaveCanvas::Paint(Draw& w)
{
    w.DrawRect(GetSize(),Bg());w.DrawImage(0,0,left_back);w.DrawImage(DPI(right_x),0,result_back);
    w.DrawImage(DPI(right_x),DPI(122),preview_back);int list_y=134+PREVIEW_HEIGHT;w.DrawImage(DPI(right_x),DPI(list_y),list_back);
    TextAt(w,18,16,"Your shell",14,Ink(),false,true);TextAt(w,18,44,"Finished outside",11,Ink(),false,true);
    int half=(left_width-48)/2;const int ys[]={119,181,284,346,408,514};
    const char* labels[][2]={{"Main staves","Finished wall"},{"Outside padding","Inside padding"},{"Finished depth","Top trim"},{"Router end gap","Bottom trim"},{"Base plate thickness","End-cut kerf"},{"Straight strip thickness","Every Nth joint"}};
    for(int r=0;r<6;r++)for(int c=0;c<2;c++)TextAt(w,18+c*(half+12),ys[r],labels[r][c],10,Muted());
    for(int y:{242,470,636})w.DrawRect(DPI(18),DPI(y),DPI(left_width-36),DPI(1),Line());
    TextAt(w,18,258,"DEPTH & MOUNTING",10,Amber(),true);TextAt(w,18,488,"OPTIONAL STRAIGHT STRIPS",10,Coral(),true);
    TextAt(w,18,576,"Selected joints (optional)",10,Muted());TextAt(w,18,657,"SAVED DESIGNS",10,Muted(),true);TextAt(w,18,731,"Name",10,Muted());
    TextAt(w,left_width-56,16,unit,11,Muted(),true);
    Wrap(w,18,878,left_width-36,status,10,dirty?Amber():Teal());
    Wrap(w,18,925,left_width-36,"Dimensions use "+unit+". Counts stay whole. Save keeps the setup beside the executable.",10,Muted());
    int split=right_x+int(right_width*.42);
    TextAt(w,right_x+18,15,"EDGE BEVEL · FROM SQUARE",9,Amber(),true);TextAt(w,split+16,15,"ROUGH STOCK / CUT LENGTH",9,Amber(),true);
    w.DrawRect(DPI(split),DPI(15),DPI(1),DPI(80),Line());
    if(geometry.IsValid()){
        TextAt(w,right_x+18,34,Number(geometry.bevelDegrees)+"°",38,Amber(),true);
        TextAt(w,right_x+18,84,Number(geometry.divisionDegrees)+"° per main stave",11,Muted());
        TextAt(w,split+16,40,M(geometry.stockThicknessMm)+" thick",20,Ink(),true);
        TextAt(w,split+16,76,M(geometry.blankLengthMm)+" per length",12,Muted(),true);
    }else Wrap(w,right_x+18,40,right_width-36,geometry.error,12,Coral());
    TextAt(w,right_x+18,138,"How the segments fit",14,Ink(),false,true);TextAt(w,right_x+18,158,"TOP VIEW + AXIAL SECTION · NOT PRINT SCALE",9,Muted(),true);
    TextAt(w,right_x+18,122+PREVIEW_HEIGHT-53,"Gold: finished outside   Green: bore   Coral: straight strips",10,Muted());
    String note=geometry.warning.IsEmpty()?"Straight cylindrical shell. The measured router end gap is reserved as sacrificial stock.":geometry.warning;
    Wrap(w,right_x+18,122+PREVIEW_HEIGHT-33,right_width-36,note,10,geometry.warning.IsEmpty()?Muted():Amber());
    TextAt(w,right_x+18,list_y+16,"Cut list & centre references",13,Ink(),false,true);
    int name_width=142,rest=right_width-name_width-36,cw=rest/5;int start=right_x+18+name_width;
    const char* columns[]={"Qty","Outer","Inner","Stock","Bevel"};
    TextAt(w,right_x+18,list_y+48,"Part group",10,Muted());for(int c=0;c<5;c++)TextAt(w,start+c*cw,list_y+48,columns[c],10,Muted());
    if(!geometry.IsValid()){TextAt(w,right_x+18,list_y+85,"No cut list while measurements are invalid.",12,Muted());return;}
    for(int r=0;r<groups.GetCount();r++){
        const auto& q=groups[r];int y=list_y+71+r*25;w.DrawRect(DPI(right_x+18),DPI(y),DPI(right_width-36),DPI(1),Line());
        TextAt(w,right_x+18,y+6,q.name,10,Ink());String values[]={AsString(q.count),Number(q.outer/Factor()),Number(q.inner/Factor()),Number(q.depth/Factor()),Number(q.bevel)+"°"};
        for(int c=0;c<5;c++)TextAt(w,start+c*cw,y+6,values[c],10,Ink(),true);
    }
    const auto& p=geometry.staves[min(selected,geometry.staves.GetCount()-1)];
    TextAt(w,right_x+18,list_y+217,Format("STAVE %d",p.index)+" · outer "+M(p.outerWidthMm)+" · inner "+M(p.innerWidthMm),11,Amber(),true);
    TextAt(w,right_x+18,list_y+238,"Centre → outer flat "+M(geometry.outerApothemMm)+" · inner flat "+M(geometry.innerApothemMm),11,Muted());
    TextAt(w,right_x+18,list_y+259,"Face midpoint offset "+M(p.centreOffsetMm)+" along local tangent",10,Muted());
    TextAt(w,right_x+18,list_y+281,"Main stock "+M(input.count*(geometry.blankLengthMm+input.endKerfMm))+" · strip stock "+M(geometry.spacers.GetCount()*(geometry.blankLengthMm+input.endKerfMm)),11,Ink());
    Wrap(w,right_x+18,list_y+305,right_width-36,"Stock totals include one separating end-cut kerf per piece; cut lengths exclude it. Allow separately for initial squaring, ripping and defects. Export includes assembly order.",10,Muted());
    TextAt(w,right_x,972,"Planning geometry, not a safe machine path. Dry-fit before glue; verify every setup.",10,Muted());
}
StavePage::StavePage(bool load_disk):canvas(load_disk)
{
    StyleScroll(scroll);scroll.SetScrollMode(UIPANELSCROLL_VERTICAL);Add(scroll.SizePos());scroll.Content().Add(canvas);
}
void StavePage::Layout()
{
    int width=max(850,SLogical(GetSize().cx)-18);SPlace(canvas,0,0,width,canvas.ContentHeight());scroll.Layout();
}
void StavePage::ApplyTheme(){StyleScroll(scroll);canvas.ApplyTheme();}
} }
