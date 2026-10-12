#include "Window.h"
namespace Upp { namespace RouterApp {
static int WLogical(int px) { return int(px*100.0/DPI(100)+.5); }
static void WPlace(Ctrl& c,int x,int y,int w,int h) { c.SetRect(DPI(x),DPI(y),DPI(w),DPI(h)); }
RouterWindow::RouterWindow(bool load_disk):circle(load_disk),staves(load_disk),persist_preferences(load_disk)
{
    Title("UPP Router Calculator");Sizeable().Zoomable();
    SetMinSize(Size(DPI(980),DPI(600)));SetRect(0,0,DPI(1320),DPI(900));
    preferences_path=AppendFileName(GetExeFolder(),"router_calculator.preferences.json");
    if(load_disk&&FileExists(preferences_path)) {
        preferences_exist=true;
        FileIn f(preferences_path);
        if(f.IsOpen()&&f.GetSize()<65536) {
            preferences_text=f.Get((int)f.GetSize());
            Value v;
            try { v=ParseJSON(preferences_text); } catch(...) { v=Value(); }
            if(v.Is<ValueMap>()&&v["schemaVersion"]==1&&v["dark"].Is<bool>())SetDark((bool)v["dark"]);
        }
    }
    tabs.Add(circle,"Calculate router circle");tabs.Add(staves,"Drum staves");
    tabs.EnableCloseButtons(false).EnableDragHandles(false).EnableDragReorder(false);
    Add(tabs);Add(mm);Add(cm);Add(theme);Add(mark);
    mm.WhenAction=[this]{ChangeUnit("mm");};cm.WhenAction=[this]{ChangeUnit("cm");};
    theme.WhenAction=[this]{SetDark(!IsDark());ApplyTheme();SavePreferences();};
    theme.Tip("Switch dark / light theme. Your choice is stored beside the executable.");
    circle.canvas.WhenDisplay=[this]{UpdateUnits();};staves.canvas.WhenDisplay=[this]{UpdateUnits();};
    tabs.WhenAction=[this]{UpdateUnits();};
    mark.SetSource("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"52\" height=\"52\" viewBox=\"0 0 52 52\"><circle cx=\"26\" cy=\"26\" r=\"23\" fill=\"none\" stroke=\"#e6b972\" stroke-width=\"1.5\"/><circle cx=\"26\" cy=\"26\" r=\"11\" fill=\"none\" stroke=\"#80d6bb\"/><circle cx=\"33\" cy=\"26\" r=\"16\" fill=\"#91b8db\" fill-opacity=\".1\" stroke=\"#91b8db\"/><circle cx=\"33\" cy=\"26\" r=\"4\" fill=\"#f09987\"/></svg>");
    ApplyTheme();
    WhenClose=[this]{if(circle.canvas.AllowDiscard()&&staves.canvas.AllowDiscard()){SavePreferences();Close();}};
}
void RouterWindow::UpdateUnits()
{
    String next=tabs.GetActiveTab()==1?staves.canvas.GetUnit():circle.canvas.GetUnit();
    if(next==shown_unit)return;
    shown_unit=next;StyleButton(mm,"mm",next=="mm");StyleButton(cm,"cm",next=="cm");
}
void RouterWindow::ChangeUnit(const String& unit)
{
    if(tabs.GetActiveTab()==1)staves.canvas.ChangeUnit(unit);else circle.canvas.ChangeUnit(unit);
    UpdateUnits();
}
void RouterWindow::ApplyTheme()
{
    UiTab::Style s=UiTab::StyleDefault();s.tab_font=TextFont(12);s.tab_extent=DPI(34);s.body_gap=DPI(12);s.visual=UITAB_UNDERLINE;
    s.metrics.frame_enabled=false;s.metrics.radius=0;s.metrics.content_margin=Rect(0,0,0,0);s.metrics.shadow.enabled=false;
    s.tab_metrics.radius=0;s.tab_metrics.shadow.enabled=false;s.active_frame_color=Amber();s.indicator_thickness=DPI(2);s.expand_tabs=false;
    for(int i=0;i<4;i++){s.palette.face[i]=Bg();s.palette.ink[i]=Ink();s.tab_palette.face[i]=Bg();s.tab_palette.ink[i]=Amber();s.tab_palette.frame[i]=Amber();}
    tabs.SetCustomStyle(s).SetVisual(UITAB_UNDERLINE);
    circle.canvas.ApplyTheme();StyleScroll(circle.scroll);staves.ApplyTheme();
    StyleButton(theme,IsDark()?"Light theme":"Dark theme");shown_unit.Clear();UpdateUnits();mark.Layout();Refresh();
}
bool RouterWindow::SavePreferences()
{
    if(!persist_preferences)return true;
    ValueMap v;v.Add("schemaVersion",1);v.Add("dark",IsDark());String error;
    if(RouterCalc::WriteJsonFile(preferences_path,v,preferences_text,preferences_exist,error))return true;
    theme.Tip("Theme is active, but could not be saved: "+error);return false;
}
void RouterWindow::Layout()
{
    int width=WLogical(GetSize().cx),height=WLogical(GetSize().cy),content=min(1340,width-32),left=(width-content)/2;
    WPlace(mark,left,12,36,36);WPlace(mm,left+content-216,16,42,28);WPlace(cm,left+content-170,16,42,28);WPlace(theme,left+content-116,16,116,28);
    WPlace(tabs,left,59,content,max(1,height-71));
}
void RouterWindow::Paint(Draw& w)
{
    w.DrawRect(GetSize(),Bg());int width=WLogical(GetSize().cx),content=min(1340,width-32),left=(width-content)/2;
    TextAt(w,left+47,10,"Router Calculator",22,Ink(),false,true);
    TextAt(w,left+47,36,"Workshop geometry. Fewer surprises in the timber.",11,Muted());
}
bool RouterWindow::Key(dword key,int count)
{
    if(key==K_CTRL_S){if(tabs.GetActiveTab()==1)staves.canvas.Save();else circle.canvas.SaveDesign();return true;}
    if(key==K_CTRL_O){if(tabs.GetActiveTab()==1)staves.canvas.Open();else circle.canvas.OpenLibrary();return true;}
    return TopWindow::Key(key,count);
}
} }
