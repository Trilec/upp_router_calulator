#include "Window.h"
#include <cmath>
#include <chrono>
namespace Upp { namespace RouterApp {
int StaveCanvas::SelfTest(const String& folder)
{
    int passed=0,failed=0;String report;
    auto check=[&](bool ok,const char* label){if(ok)++passed;else{++failed;report<<"FAIL: "<<label<<"\n";}};
    auto eq=[](double a,double b){return std::abs(a-b)<1e-8;};
    check(geometry.IsValid()&&eq(geometry.bevelDegrees,9),"20-stave default bevel");
    check(kind=="diameter"&&eq(fields[OUTER].GetValue(),350),"drums default to outside diameter");
    fields[OUTER].SetTextUtf8("1000");fields[OUTER].WhenChange();
    check(eq(input.outerRadiusMm,500)&&eq(geometry.innerRadiusMm,490),"one-metre drum input");
    radius.WhenAction();check(eq(fields[OUTER].GetValue(),500),"radius switch preserves outside size");
    ChangeUnit("cm");check(eq(fields[OUTER].GetValue(),50)&&eq(fields[COUNT].GetValue(),20),"centimetres do not convert counts");
    ChangeUnit("mm");diameter.WhenAction();
    fields[SPACER].SetTextUtf8("2");fields[SPACER].WhenChange();
    check(geometry.IsValid()&&geometry.spacers.GetCount()==20,"straight strips at every joint");
    fields[EVERY].SetTextUtf8("3");fields[EVERY].WhenChange();
    check(geometry.IsValid()&&geometry.spacers.GetCount()==6&&!geometry.warning.IsEmpty(),"non-dividing cadence warns but closes");
    joints.SetTextUtf8("1,7,20");joints.WhenChange();
    check(geometry.IsValid()&&geometry.spacers.GetCount()==3,"explicit joints override cadence");
    check(geometry.staves[0].leftSpacerMm==2&&geometry.staves[0].rightSpacerMm==2,"last joint wraps to first stave");
    fields[DEPTH].SetTextUtf8("300");fields[DEPTH].WhenChange();
    check(eq(geometry.blankLengthMm,319)&&eq(geometry.finishedBottomZMm,17),"depth includes end gap and trims");
    fields[PLATE].SetTextUtf8("25");fields[PLATE].WhenChange();
    check(eq(geometry.blankLengthMm,319)&&eq(geometry.assemblyHeightMm,344),"plate does not get added to stave length");
    fields[COUNT].SetTextUtf8("20.5");fields[COUNT].WhenChange();
    check(!geometry.IsValid()&&!save.IsEnabled()&&!export_file.IsEnabled(),"fractional stave count rejected");
    fields[COUNT].SetTextUtf8("20");fields[COUNT].WhenChange();
    fields[WALL].SetTextUtf8("");fields[WALL].WhenChange();check(!geometry.IsValid(),"incomplete measurements clear result");
    fields[WALL].SetTextUtf8("10");fields[WALL].WhenChange();check(geometry.IsValid(),"valid edit restores result");
    joints.SetTextUtf8("1,1");joints.WhenChange();check(!geometry.IsValid(),"duplicate joint rejected");
    joints.SetTextUtf8("1,7,20");joints.WhenChange();
    path=AppendFileName(folder,"stave-ui-"+AsString(Uuid::Create())+".json");disk_text.Clear();disk_exists=false;
    name.SetTextUtf8("Workshop shell - 2 mm strips");notes.SetTextUtf8("Measured end gap 15 mm.");
    check(Save(false)&&!dirty&&current==0,"save named shell");String original_path=path;
    New(false);LoadDesign(0,false);
    check(eq(input.outerRadiusMm,500)&&input.selectedJoints=="1,7,20"&&eq(input.depthMm,300),"reopen all stave parameters");
    name.SetTextUtf8("Renamed shell");name.WhenChange();check(Save(false),"rename and save shell");
    check(((ValueArray)library["designs"]).GetCount()==1,"rename does not add another design");
    check(LoadPath(original_path,false),"JSON round trip through actual UI");
    Value before=library;String bad=AppendFileName(folder,"bad-staves-"+AsString(Uuid::Create())+".json");SaveFile(bad,"{broken");
    check(!LoadPath(bad,false)&&library==before,"corrupt import preserves saved library");FileDelete(bad);
    String clean=LoadFile(path);SaveFile(path,"external edit");
    check(!Save(false)&&LoadFile(path)=="external edit","UI save respects external edits");SaveFile(path,clean);
    New(false);name.SetTextUtf8("18-stave shell with 2 mm strips");notes.Clear();status="Preview - not saved";
    input=RouterCalc::StaveInput();input.count=18;input.spacerThicknessMm=2;input.spacerEvery=3;unit="mm";kind="diameter";selected=0;Sync();Recompute(false);
    bool original_theme=IsDark();
    for(bool dark:{true,false}){
        SetDark(dark);ApplyTheme();
        for(int width:{920,1320}){
            SetRect(0,0,DPI(width),DPI(ContentHeight()));Layout();
            check(Rect(GetSize()).Contains(export_file.GetRect())&&Rect(GetSize()).Contains(preview.GetRect()),"stave controls stay inside content");
            int renders=preview.RenderCount();Layout();Read();
            check(preview.RenderCount()==renders,"identical layout and input do not rerasterise");
            ImageDraw draw(GetSize());DrawCtrl(draw);
            check(preview.RenderCount()==renders,"painting uses the cached raster");
            String filename=Format("native-staves-%s-%d.png",dark?"dark":"light",width);
            check(PNGEncoder().SaveFile(AppendFileName(folder,filename),draw),"stave theme screenshot");
        }
    }
    SetDark(original_theme);ApplyTheme();
    FileDelete(original_path);FileDelete(original_path+".bak");dirty=false;
    report<<"StaveUi: "<<passed<<" passed, "<<failed<<" failed\n";Cout()<<report;SaveFile(AppendFileName(folder,"stave-ui-results.txt"),report);return failed;
}
int RouterWindow::SelfTest(const String& folder)
{
    int failed=circle.canvas.SelfTest(folder)+staves.canvas.SelfTest(folder),passed=0;String report;
    auto check=[&](bool ok,const char* label){if(ok)++passed;else{++failed;report<<"FAIL: "<<label<<"\n";}};
    check(tabs.GetCount()==2,"circle and drum tabs are registered");
    for(bool dark:{true,false}){
        SetDark(dark);ApplyTheme();
        for(int active:{0,1}){
            tabs.SetActiveTab(active);UpdateUnits();Layout();circle.Layout();staves.Layout();Ctrl::ProcessEvents();
            check(tabs.GetActiveTab()==active,"tab selection");
            ImageDraw draw(GetSize());DrawCtrl(draw);
            String file=String(active?"window-staves-":"window-circle-")+(dark?"dark":"light")+".png";
            check(PNGEncoder().SaveFile(AppendFileName(folder,file),draw),"window screenshot");
        }
    }
    SvgView cache;cache.SetRect(0,0,DPI(600),DPI(300));
    String source=CircleSvg(RouterCalc::Calculate(RouterCalc::Inputs()),"mm",true,true,true);
    cache.SetSource(source);cache.Layout();int initial=cache.RenderCount();
    auto begin=std::chrono::steady_clock::now();
    for(int i=0;i<1000;i++){cache.SetSource(source);cache.Layout();}
    double millis=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
    check(cache.RenderCount()==initial,"1000 unchanged SVG updates produce zero rerenders");
    SetDark(!IsDark());cache.Layout();check(cache.RenderCount()==initial+1,"theme change invalidates raster once");
    SetDark(true);ApplyTheme();tabs.SetActiveTab(0);UpdateUnits();
    report<<"WindowUi: "<<passed<<" passed; total GUI failures "<<failed<<"\n";
    report<<"1000 unchanged SVG source/layout updates: "<<RouterCalc::Number(millis)<<" ms; raster rebuilds: 0\n";
    Cout()<<report;SaveFile(AppendFileName(folder,"window-ui-results.txt"),report);return failed;
}
} }
