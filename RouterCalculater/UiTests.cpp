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
    SetRect(0,0,DPI(920),DPI(ContentHeight()));Layout();ChangeUnit("cm");
    for(int d:{30,40,50,60}){fields[OUTER].SetValue(d);fields[OUTER].WhenChange();check(geometry.IsValid()&&eq(input.outerRadiusMm,d*5)&&eq(geometry.innerRadiusMm,d*5-10),"30/40/50/60 cm outside diameter");}
    ChangeUnit("mm");fields[OUTER].SetTextUtf8("40cm");fields[OUTER].WhenChange();check(geometry.IsValid()&&eq(input.outerRadiusMm,200),"explicit cm suffix in mm display");
    int good_raster=preview.RenderCount();fields[WALL].SetTextUtf8("40cm");fields[WALL].WhenChange();
    check(!geometry.IsValid()&&fields[WALL].HasIssue()&&preview.IsRetainingInvalid()&&preview.RenderCount()==good_raster,"oversized radial wall retains preview and marks wall");
    radius.WhenAction();check(eq(fields[OUTER].GetValue(),200)&&!geometry.IsValid(),"radius switch allowed with invalid wall");
    ChangeUnit("cm");check(eq(fields[OUTER].GetValue(),20)&&eq(fields[WALL].GetValue(),40),"unit conversion allowed with invalid wall");
    diameter.WhenAction();fields[WALL].SetValue(1);fields[WALL].WhenChange();check(geometry.IsValid()&&!preview.IsRetainingInvalid()&&eq(geometry.innerRadiusMm,190),"correction restores 40 cm shell");
    fields[OUTER].MouseWheel(Point(5,5),120,0);check(eq(fields[OUTER].GetValue(),40.1),"wheel step 0.1 cm");
    fields[OUTER].MouseWheel(Point(5,5),120,K_ALT);check(eq(fields[OUTER].GetValue(),41.1),"Alt wheel step 1 cm");
    fields[OUTER].MouseWheel(Point(5,5),120,K_SHIFT);check(eq(fields[OUTER].GetValue(),51.1),"Shift wheel step 10 cm");
    fields[COUNT].MouseWheel(Point(5,5),120,0);check(input.count==21,"stave wheel remains integral");
    fields[WALL].Clear();fields[WALL].WhenChange();ChangeUnit("mm");check(fields[WALL].IsEmpty()&&eq(fields[OUTER].GetValue(),511),"incomplete field preserved during conversion");
    fields[WALL].SetValue(10);fields[WALL].WhenChange();fields[OUTER].SetValue(22.05);fields[OUTER].WhenChange();
    fields[OUTER].MouseWheel(Point(5,5),-120,0);check(geometry.IsValid()&&eq(fields[OUTER].GetValue(),22.05)&&fields[OUTER].HasIssue(),"wheel stops before bore allowance is consumed");
    New(false);ChangeUnit("mm");
    double range=outside_slider.GetMax();outside_slider.SetValue(400);outside_slider.WhenChanging();
    check(geometry.IsValid()&&eq(input.outerRadiusMm,200)&&eq(fields[OUTER].GetValue(),400),"outside slider drives diameter and geometry");
    int raster=preview.RenderCount();outside_slider.WhenAction();check(preview.RenderCount()==raster,"slider commit does not repeat changed preview");
    outside_slider.SetValue(450);outside_slider.WhenChanging();check(eq(outside_slider.GetMax(),range),"outside range stays fixed during drag");
    radius.WhenAction();ChangeUnit("cm");outside_slider.SetValue(500);outside_slider.WhenChanging();
    check(eq(fields[OUTER].GetValue(),25)&&eq(input.outerRadiusMm,250),"outside slider preserves radius and cm semantics");
    count_slider.SetValue(24);count_slider.WhenChanging();check(input.count==24&&eq(geometry.bevelDegrees,7.5),"native stave count slider drives bevel");
    fields[COUNT].SetValue(100);fields[COUNT].WhenChange();check(eq(count_slider.GetValue(),100)&&count_slider.GetMax()>=100,"typed count expands count slider");
    fields[OUTER].SetValue(100);fields[OUTER].WhenChange();check(eq(outside_slider.GetValue(),2000)&&outside_slider.GetMax()>=2000,"typed large radius expands physical slider");
    outside_slider.SetValue(1);outside_slider.WhenChanging();check(geometry.IsValid()&&eq(input.outerRadiusMm,1000)&&fields[OUTER].HasIssue(),"outside slider rejects impossible shell without erasing it");
    int height=ContentHeight();formula_more.WhenAction();check(ContentHeight()>height,"drum explanation expands");formula_more.WhenAction();check(ContentHeight()==height,"drum explanation collapses");
    New(false);ChangeUnit("mm");ChangeKind("diameter");
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
            check(fields[TOP_TRIM].GetRect().top==fields[BOTTOM_TRIM].GetRect().top&&fields[DEPTH].GetRect().top==fields[TOP_TRIM].GetRect().top,"depth and both trims share one row");
            check(fields[PLATE].GetRect().top==fields[REACH_GAP].GetRect().top&&fields[KERF].GetRect().top==fields[PLATE].GetRect().top,"mounting dimensions share second row");
            check(saved.GetScreenRect().bottom<fields[OUTER].GetScreenRect().top,"saved designs precede measurements");
            check(outside_slider.GetSize().cx>0&&count_slider.GetSize().cx>0&&cut_table.GetSize().cy>=DPI(132),"sliders laid out and all five cut groups have room");
            check(ContentHeight()<=740,"collapsed drum page stays compact");
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
    check(version.GetText().Find(APP_VERSION)>=0,"visible badge uses central application version");
    check(version.GetText().Find(APP_CONFIGURATION)>=0,"visible badge identifies debug or release");
    check(!version.GetRect().Intersects(heading.GetRect())&&!version.GetRect().Intersects(mm.GetRect()),"version badge does not overlap header controls");
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
    DiagramView cache;cache.SetRect(0,0,DPI(600),DPI(300));
    auto source=[](){return CircleDiagram(RouterCalc::Calculate(RouterCalc::Inputs()),"mm",true,true,true);};
    cache.SetScene(source());cache.Layout();int initial=cache.RenderCount();
    auto begin=std::chrono::steady_clock::now();
    for(int i=0;i<1000;i++){cache.SetScene(source());cache.Layout();}
    double millis=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();
    check(cache.RenderCount()==initial,"1000 unchanged native scene updates produce zero rerenders");
    SetDark(!IsDark());cache.Layout();check(cache.RenderCount()==initial+1,"theme change invalidates raster once");
    initial=cache.RenderCount();cache.SetScene(CircleDiagram(RouterCalc::Calculate(RouterCalc::Inputs()),"cm",true,true,true));check(cache.RenderCount()==initial,"unit-label change does not redraw curves");
    Size pixels=cache.Raster().GetSize();RouterCalc::Inputs huge;huge.holeRadiusMm=100000;cache.SetScene(CircleDiagram(RouterCalc::Calculate(huge),"mm",true,true,true));check(cache.Raster().GetSize()==pixels,"large real-world dimension does not enlarge image buffer");
    UnitEdit value;double n;value.SetTextUtf8("40 cm");check(value.TryGetValue(n)&&std::abs(n-400)<1e-9,"unit editor suffix conversion");value.ConvertUnit("cm");check(std::abs(value.GetValue()-40)<1e-9,"unit editor physical value preserved");
    value.SetTextUtf8("1e");check(!value.TryGetValue(n),"incomplete exponent rejected");value.SetValue(30);value.MouseWheel(Point(0,0),60,0);check(std::abs(value.GetValue()-30)<1e-9,"partial wheel notch is accumulated");value.MouseWheel(Point(0,0),60,0);check(std::abs(value.GetValue()-30.1)<1e-9,"two half-notches produce one step");
    SetDark(true);ApplyTheme();tabs.SetActiveTab(0);UpdateUnits();
    report<<"WindowUi: "<<passed<<" passed; total GUI failures "<<failed<<"\n";
    report<<"1000 unchanged native scene source/layout updates: "<<RouterCalc::Number(millis)<<" ms; raster rebuilds: 0\n";
    Cout()<<report;SaveFile(AppendFileName(folder,"window-ui-results.txt"),report);return failed;
}
} }
