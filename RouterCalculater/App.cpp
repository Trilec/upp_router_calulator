#include "App.h"
#include <cmath>
namespace Upp { namespace RouterApp {
using RouterCalc::Number;
static int Logical(int px){return int(px * 100.0 / DPI(100) + .5);}
static void Place(Ctrl& c,int x,int y,int w,int h){c.SetRect(DPI(x),DPI(y),DPI(w),DPI(h));}
String CircleCanvas::Measure(double mm) const{return Number(mm/Factor())+" "+unit;}
CircleCanvas::CircleCanvas(bool load_disk)
{
    library=RouterCalc::NewLibrary();library_path=AppendFileName(GetExeFolder(),"router_calculator.designs.json");
    for(UiFloatEdit* e:{&hole,&guide,&cutter}){StyleEdit(*e,22);e->ShowSpin(false).Precision(5).Step(.1);Add(*e);e->WhenChange=[this]{if(!syncing)ReadInputs();};e->WhenAction=[this]{if(!syncing)ReadInputs();};}
    hole.Tip("Finished hole: choose radius or diameter. The value is not a template dimension.");
    guide.Tip("Diameter of the circular, concentric guide edge at the actual template contact height.");
    cutter.Tip("Cutting diameter across the outside cutting edges, not the shank or plunge depth.");
    for(int i=0;i<3;i++){suffix[i].SetInk(Muted()).SetFont(TextFont(13,true));suffix[i].SetLabel("mm");Add(suffix[i]);}
    for(UiSlider* s:{&hole_slider,&guide_slider,&cutter_slider}){Add(*s);s->ExpandTrack().SetTrackInset(0).SetCancelReverts();}
    auto slide=[this](int which){
        if(syncing)return;
        double value=which==0?hole_slider.GetValue():which==1?guide_slider.GetValue():cutter_slider.GetValue();
        double& target=which==0?input.holeRadiusMm:which==1?input.guideDiameterMm:input.cutterDiameterMm;
        if(value==target)return;
        target=value;
        SyncControls();Recompute();
    };
    hole_slider.WhenChanging=hole_slider.WhenAction=[slide]{slide(0);};
    guide_slider.WhenChanging=guide_slider.WhenAction=[slide]{slide(1);};
    cutter_slider.WhenChanging=cutter_slider.WhenAction=[slide]{slide(2);};
    StyleButton(radius,"Radius",true);StyleButton(diameter,"Diameter");
    radius.WhenAction=[this]{ChangeKind("radius");};diameter.WhenAction=[this]{ChangeKind("diameter");};
    StyleButton(new_design,"+ New");new_design.WhenAction=[this]{NewDesign();};
    StyleButton(save,"Save",true);save.WhenAction=[this]{SaveDesign();};
    StyleButton(settings,"Settings");settings.WhenAction=[this]{Settings();};
    StyleButton(export_file,"Export");export_file.WhenAction=[this]{ExportLibrary();};
    StyleButton(copy,"Copy measurements");copy.WhenAction=[this]{CopyMeasurements();};
    StyleButton(note_toggle,"▸ Notes (optional)");note_toggle.SetFrameWidth(0);
    note_toggle.WhenAction=[this]{notes_open=!notes_open;note_toggle.SetText(notes_open?"▾ Notes (optional)":"▸ Notes (optional)");WhenExtent();};
    StyleButton(formula_more,"▸ Calculation details");formula_more.SetFrameWidth(0);
    formula_more.WhenAction=[this]{expanded_formula=!expanded_formula;formula_more.SetText(expanded_formula?"▾ Calculation details":"▸ Calculation details");WhenExtent();};
    StyleButton(safety_more,"▸ Guide and cutter notes");safety_more.SetFrameWidth(0);
    safety_more.WhenAction=[this]{expanded_safety=!expanded_safety;safety_more.SetText(expanded_safety?"▾ Guide and cutter notes":"▸ Guide and cutter notes");WhenExtent();};
    for(UiButton* b:{&radius,&diameter,&new_design,&save,&settings,&export_file,&copy,&note_toggle,&formula_more,&safety_more})Add(*b);
    StyleDropdown(saved);saved.SetPlaceholderText("No saved designs yet");Add(saved);
    saved.WhenSelect=[this](int index){if(!syncing&&index>=0)LoadDesign(index);};
    StyleEdit(name);name.SetPlaceholder("Name this design");name.SetTextUtf8("Untitled design");Add(name);
    StyleEdit(notes);notes.SetPlaceholder("Cutter, material, jig or project notes…");Add(notes);notes.Hide();
    name.WhenChange=notes.WhenChange=[this]{if(!syncing){dirty=true;Refresh();}};
    StyleCheck(show_base,"Base");StyleCheck(show_path,"Centre path");StyleCheck(show_radii,"Radii");
    for(UiCheckBox* c:{&show_base,&show_path,&show_radii}){c->SetChecked(true);Add(*c);c->WhenAction=[this]{Recompute();};}
    Add(diagram);Add(cutter_picture);
    status="Not saved yet";ApplyTheme();SyncControls();Recompute(false);
    if(load_disk&&FileExists(library_path))LoadPath(library_path,false);
}
void CircleCanvas::ApplyTheme()
{
    int theme=IsDark()?1:0;if(applied_theme==theme)return;applied_theme=theme;
    for(UiFloatEdit* e:{&hole,&guide,&cutter})StyleEdit(*e,22);
    StyleEdit(name);StyleEdit(notes);StyleDropdown(saved);
    StylePrimary(save,"Save");StyleButton(settings,"Settings");StyleButton(export_file,"Export");StyleButton(new_design,"+ New");StyleButton(copy,"Copy measurements");
    StyleButton(note_toggle,notes_open?"▾ Notes (optional)":"▸ Notes (optional)");note_toggle.SetFrameWidth(0);
    StyleButton(formula_more,expanded_formula?"▾ Calculation details":"▸ Calculation details");formula_more.SetFrameWidth(0);
    StyleButton(safety_more,expanded_safety?"▾ Guide and cutter notes":"▸ Guide and cutter notes");safety_more.SetFrameWidth(0);
    StyleCheck(show_base,"Base");StyleCheck(show_path,"Centre path");StyleCheck(show_radii,"Radii");
    StyleSlider(hole_slider,Amber());StyleSlider(guide_slider,Blue());StyleSlider(cutter_slider,Coral());
    for(Label& l:suffix)l.SetInk(Muted()).SetFont(TextFont(11,true));
    left_back=result_back=diagram_back=table_back=info_back=safety_back=Image();styled_kind.Clear();SyncControls();Layout();diagram.Layout();cutter_picture.Layout();Refresh();
}
void CircleCanvas::SetSlider(UiSlider& s,double value,double low,double high,Color colour)
{
    if(!std::isfinite(value)||IsNull(value)||value<=0)return;
    (void)colour; // Palette is configured once per theme, not on each input event.
    double lo=min(low,value),hi=max(high,value*1.1);
    if(s.GetMin()!=lo||s.GetMax()!=hi)s.SetRange(lo,hi);
    if(s.GetValue()!=value)s.SetValue(value);
}
void CircleCanvas::SyncControls()
{
    syncing=true;
    auto set=[](UiFloatEdit& e,double v){double old;if(e.TryGetValue(old)&&old==v)return;if(e.IsEmpty()&&IsNull(v))return;e.SetValue(v);};
    set(hole,input.holeRadiusMm*(kind=="diameter"?2:1)/Factor());set(guide,input.guideDiameterMm/Factor());set(cutter,input.cutterDiameterMm/Factor());
    for(Label& l:suffix)l.SetLabel(unit);
    if(styled_kind!=kind){StyleButton(radius,"Radius",kind=="radius");StyleButton(diameter,"Diameter",kind=="diameter");styled_kind=kind;}
    SetSlider(hole_slider,input.holeRadiusMm,.5,150,Amber());SetSlider(guide_slider,input.guideDiameterMm,30,240,Blue());SetSlider(cutter_slider,input.cutterDiameterMm,1,32,Coral());
    syncing=false;WhenDisplay();
}
void CircleCanvas::ReadInputs()
{
    double v;RouterCalc::Inputs next;
    next.holeRadiusMm=hole.TryGetValue(v)?v*Factor()/(kind=="diameter"?2:1):double(Null);
    next.guideDiameterMm=guide.TryGetValue(v)?v*Factor():double(Null);
    next.cutterDiameterMm=cutter.TryGetValue(v)?v*Factor():double(Null);
    auto same=[](double a,double b){return a==b||(IsNull(a)&&IsNull(b));};
    if(same(next.holeRadiusMm,input.holeRadiusMm)&&same(next.guideDiameterMm,input.guideDiameterMm)&&same(next.cutterDiameterMm,input.cutterDiameterMm))return;
    input=next;
    syncing=true;
    SetSlider(hole_slider,input.holeRadiusMm,.5,150,Amber());SetSlider(guide_slider,input.guideDiameterMm,30,240,Blue());SetSlider(cutter_slider,input.cutterDiameterMm,1,32,Coral());
    syncing=false;Recompute();
}
void CircleCanvas::Recompute(bool changed)
{
    geometry=RouterCalc::Calculate(input);if(changed){dirty=true;status="Unsaved changes";}
    save.Enable(geometry.IsValid());copy.Enable(geometry.IsValid());
    diagram.SetSource(CircleSvg(geometry,unit,show_base.IsChecked(),show_path.IsChecked(),show_radii.IsChecked()));
    cutter_picture.SetSource(CutterSvg(input.cutterDiameterMm,unit));
    Refresh();
}
void CircleCanvas::ChangeUnit(const String& next)
{
    if(next==unit)return;
    if(!geometry.IsValid()){ReportError("Finish the current measurements before changing units.");return;}
    unit=next;SyncControls();Recompute();
}
void CircleCanvas::ChangeKind(const String& next)
{
    if(next==kind)return;
    if(!geometry.IsValid()){ReportError("Finish the current measurements before switching radius and diameter.");return;}
    kind=next;SyncControls();Recompute();
}
void CircleCanvas::ReportError(const String& error,bool modal){status=error;Refresh();if(modal)Exclamation(DeQtf(error));}
bool CircleCanvas::AllowDiscard()
{
    if(!dirty)return true;
    int answer=PromptYesNoCancel("Save changes to the current router design?");
    if(answer<0)return false;
    return answer==0||SaveDesign();
}
void CircleCanvas::ApplySettings(const Value& s)
{
    unit=(String)s["unit"];kind=(String)s["kind"];
    show_base.SetChecked((bool)s["showBase"]);show_path.SetChecked((bool)s["showPath"]);show_radii.SetChecked((bool)s["showRadii"]);
}
void CircleCanvas::NewDesign(bool ask)
{
    if(ask&&!AllowDiscard())return;
    current=-1;ApplySettings(library["settings"]);input=RouterCalc::Inputs();
    input.guideDiameterMm=(double)library["settings"]["defaultGuideDiameterMm"];
    input.cutterDiameterMm=(double)library["settings"]["defaultCutterDiameterMm"];
    input.holeRadiusMm=max(37.5,input.cutterDiameterMm/2);
    syncing=true;name.SetTextUtf8("Untitled design");notes.Clear();saved.ClearSelection();syncing=false;
    dirty=false;status="Not saved yet";SyncControls();Recompute(false);
}
void CircleCanvas::RefreshDesignList()
{
    syncing=true;saved.Clear();ValueArray designs=library["designs"];
    for(int i=0;i<designs.GetCount();i++)saved.Add((String)designs[i]["name"],i);
    if(current>=0&&current<designs.GetCount())saved.Select(current);else saved.ClearSelection();
    syncing=false;
}
void CircleCanvas::LoadDesign(int index,bool ask)
{
    if(ask&&!AllowDiscard()){syncing=true;if(current>=0)saved.Select(current);else saved.ClearSelection();syncing=false;return;}
    ValueArray designs=library["designs"];if(index<0||index>=designs.GetCount())return;
    Value d=designs[index];current=index;input=RouterCalc::DesignInputs(d);unit=(String)d["display"]["unit"];kind=(String)d["display"]["kind"];
    syncing=true;name.SetTextUtf8((String)d["name"]);notes.SetTextUtf8((String)d["notes"]);saved.Select(index);syncing=false;
    dirty=false;status="Loaded saved design";SyncControls();Recompute(false);
}
bool CircleCanvas::StageCurrent(Value& candidate,int& index,String& error)
{
    if(!geometry.IsValid()){error=geometry.error;return false;}
    String title=TrimBoth(name.GetTextUtf8());
    if(title.IsEmpty()||title.GetCount()>100){error="Give the design a name of 1 to 100 characters.";return false;}
    ValueMap root(library);ValueArray designs=library["designs"];
    ValueMap d(RouterCalc::NewDesign(input,title,unit,kind,notes.GetTextUtf8()));index=current;
    if(index>=0&&index<designs.GetCount()){
        d.Set("id",designs[index]["id"]);d.Set("createdAt",designs[index]["createdAt"]);designs.Set(index,d);
    }else{index=designs.GetCount();designs.Add(d);}
    ValueMap prefs(root["settings"]);prefs.Set("showBase",show_base.IsChecked());prefs.Set("showPath",show_path.IsChecked());prefs.Set("showRadii",show_radii.IsChecked());
    root.Set("settings",prefs);root.Set("designs",designs);candidate=root;
    return RouterCalc::ValidateLibrary(candidate,error);
}
bool CircleCanvas::SaveDesign(bool show_error)
{
    Value candidate;int selected;String error;
    if(!StageCurrent(candidate,selected,error)||!RouterCalc::WriteLibrary(library_path,candidate,disk_text,disk_exists,error)){ReportError(error,show_error);return false;}
    library=candidate;current=selected;dirty=false;status="Saved to "+GetFileName(library_path);RefreshDesignList();Refresh();return true;
}
bool CircleCanvas::ExportLibrary()
{
    Value candidate;int selected;String error;
    if(!StageCurrent(candidate,selected,error)){ReportError(error);return false;}
    FileSel fs;fs.Type("Router design library","*.json").AllFilesType().DefaultExt("json");fs.Set(library_path);
    if(!fs.ExecuteSaveAs("Export router designs"))return false;
    String destination=~fs,expected;bool exists=FileExists(destination);
    if(NormalizePath(destination)==NormalizePath(library_path)){expected=disk_text;exists=disk_exists;}
    else if(exists)expected=LoadFile(destination);
    if(!RouterCalc::WriteLibrary(destination,candidate,expected,exists,error)){ReportError(error);return false;}
    library=candidate;library_path=destination;disk_text=expected;disk_exists=true;current=selected;dirty=false;
    status="Exported and linked "+GetFileName(destination);RefreshDesignList();Refresh();return true;
}
bool CircleCanvas::LoadPath(const String& path,bool modal)
{
    FileIn file(path);String error;Value candidate;
    if(!file.IsOpen()||file.GetSize()>RouterCalc::MAX_LIBRARY_BYTES){ReportError("Cannot read the library, or the file exceeds 2 MiB.",modal);return false;}
    String text=file.Get((int)file.GetSize());
    if(!RouterCalc::ParseLibrary(text,candidate,error)){ReportError(error,modal);return false;}
    library=candidate;library_path=path;disk_text=text;disk_exists=true;current=-1;dirty=false;
    ApplySettings(library["settings"]);RefreshDesignList();
    ValueArray ds=library["designs"];if(ds.GetCount())LoadDesign(ds.GetCount()-1,false);else NewDesign(false);
    status="Opened "+GetFileName(path);Refresh();return true;
}
void CircleCanvas::OpenLibrary()
{
    if(!AllowDiscard())return;
    FileSel fs;fs.Type("Router design library","*.json").AllFilesType();fs.Set(library_path);
    if(fs.ExecuteOpen("Open router designs"))LoadPath(~fs);
}
void CircleCanvas::CopyMeasurements()
{
    if(!geometry.IsValid())return;
    String s="UPP ROUTER CALCULATOR\n";
    s<<"Finished hole: R "<<Number(geometry.holeR)<<" mm / diameter "<<Number(geometry.holeR*2)<<" mm\n";
    s<<"Template opening: R "<<Number(geometry.templateR)<<" mm / diameter "<<Number(geometry.templateR*2)<<" mm\n";
    s<<"Guide diameter: "<<Number(input.guideDiameterMm)<<" mm\nCutting diameter: "<<Number(input.cutterDiameterMm)<<" mm\n";
    s<<"Template R = hole R + guide R - cutter R\nAssumes a circular concentric guide; verify actual contact height and test in scrap.";
    WriteClipboardText(s);status="Measurements copied";Refresh();
}
class SettingsDialog : public TopWindow {
public:
    UiFloatEdit guide,cutter;UiDropdown units,kinds;UiCheckBox base,path,radii;
    UiButton apply,cancel,open,export_file,duplicate,remove;
    Value preferences;String filename;int design_count;
    SettingsDialog(const Value& prefs,const String& file,int count):preferences(prefs),filename(file),design_count(count)
    {
        Title("Router Calculator · Settings");SetRect(0,0,DPI(620),DPI(590));
        StyleEdit(guide);StyleEdit(cutter);guide.ShowSpin(false).Precision(5);cutter.ShowSpin(false).Precision(5);
        guide.SetValue((double)prefs["defaultGuideDiameterMm"]);cutter.SetValue((double)prefs["defaultCutterDiameterMm"]);
        StyleDropdown(units);units.Add("Millimetres (mm)","mm").Add("Centimetres (cm)","cm").SelectByData(prefs["unit"]);
        StyleDropdown(kinds);kinds.Add("Radius","radius").Add("Diameter","diameter").SelectByData(prefs["kind"]);
        StyleCheck(base,"Router base");StyleCheck(path,"Centre path");StyleCheck(radii,"Radii");
        base.SetChecked((bool)prefs["showBase"]);path.SetChecked((bool)prefs["showPath"]);radii.SetChecked((bool)prefs["showRadii"]);
        StyleButton(apply,"Save settings",true);StyleButton(cancel,"Close");StyleButton(open,"Open JSON…");StyleButton(export_file,"Export library…");StyleButton(duplicate,"Duplicate design");StyleButton(remove,"Delete design");
        Ctrl* controls[]={&guide,&cutter,&units,&kinds,&base,&path,&radii,&apply,&cancel,&open,&export_file,&duplicate,&remove};
        for(Ctrl* c:controls)Add(*c);
        apply.WhenAction=[this]{double g,b;if(!guide.TryGetValue(g)||!cutter.TryGetValue(b)||g<=b||g>10000||b<=0){Exclamation("Use valid millimetre diameters: guide larger than cutter, no more than 10,000 mm.");return;}
            ValueMap s(preferences);s.Set("unit",units.GetSelectedData());s.Set("kind",kinds.GetSelectedData());s.Set("defaultGuideDiameterMm",g);s.Set("defaultCutterDiameterMm",b);
            s.Set("showBase",base.IsChecked());s.Set("showPath",path.IsChecked());s.Set("showRadii",radii.IsChecked());preferences=s;Break(1);};
        cancel.WhenAction=[this]{Break(0);};open.WhenAction=[this]{Break(2);};export_file.WhenAction=[this]{Break(3);};duplicate.WhenAction=[this]{Break(4);};remove.WhenAction=[this]{Break(5);};WhenClose=[this]{Break(0);};
    }
    void Layout() override
    {
        Place(units,24,118,277,36);Place(kinds,319,118,277,36);Place(guide,24,207,277,36);Place(cutter,319,207,277,36);
        Place(base,24,276,145,26);Place(path,188,276,143,26);Place(radii,350,276,100,26);
        Place(open,24,402,135,35);Place(export_file,169,402,147,35);Place(duplicate,24,458,160,35);Place(remove,195,458,140,35);
        Place(cancel,376,532,90,36);Place(apply,477,532,119,36);
    }
    void Paint(Draw& w) override
    {
        w.DrawRect(GetSize(),Bg());TextAt(w,24,24,"Settings & saved designs",23,Ink(),false,true);
        TextAt(w,24,65,"Defaults apply to new designs. Existing saved measurements stay unchanged.",12,Muted());
        TextAt(w,24,94,"Default units",12);TextAt(w,319,94,"Default input",12);
        TextAt(w,24,182,"Default guide diameter (mm)",12);TextAt(w,319,182,"Default cutting diameter (mm)",12);
        TextAt(w,24,258,"Diagram visibility",11,Muted());
        w.DrawRect(DPI(24),DPI(320),DPI(572),DPI(1),Line());
        TextAt(w,24,338,"DESIGN LIBRARY",10,Amber(),true);TextAt(w,24,361,GetFileName(filename),12,Ink(),true);
        TextAt(w,24,383,Format("%d saved design(s) · JSON schema 1 · millimetres",design_count),10,Muted());
        TextAt(w,24,503,"Saves keep a .bak copy and refuse to overwrite external changes.",11,Muted());
    }
};
void CircleCanvas::Settings()
{
    ValueArray designs=library["designs"];
    SettingsDialog dlg(library["settings"],library_path,designs.GetCount());dlg.remove.Enable(current>=0);dlg.duplicate.Enable(geometry.IsValid());
    int result=dlg.Run();String error;
    if(result==1){ValueMap next(library);next.Set("settings",dlg.preferences);
        if(!RouterCalc::WriteLibrary(library_path,next,disk_text,disk_exists,error)){ReportError(error);return;}
        library=next;show_base.SetChecked((bool)dlg.preferences["showBase"]);show_path.SetChecked((bool)dlg.preferences["showPath"]);show_radii.SetChecked((bool)dlg.preferences["showRadii"]);
        status="Settings saved";Recompute(false);
    }
    if(result==2)OpenLibrary();if(result==3)ExportLibrary();
    if(result==4){if(!AllowDiscard())return;current=-1;syncing=true;name.SetTextUtf8(name.GetTextUtf8()+" copy");saved.ClearSelection();syncing=false;dirty=true;SaveDesign();}
    if(result==5&&current>=0){if(!AllowDiscard()||!PromptYesNo("Delete the selected saved design? A backup of the previous library will be kept."))return;
        ValueArray ds=library["designs"];ds.Remove(current);ValueMap next(library);next.Set("designs",ds);
        if(!RouterCalc::WriteLibrary(library_path,next,disk_text,disk_exists,error)){ReportError(error);return;}
        library=next;current=-1;RefreshDesignList();NewDesign(false);status="Design deleted; previous library retained in .bak";Refresh();
    }
}
CirclePage::CirclePage(bool load_disk):canvas(load_disk)
{
    UiScrollPanel::Style s=UiScrollPanel::StyleDefault();s.metrics.frame_enabled=false;s.metrics.radius=0;s.metrics.content_margin=Rect(0,0,0,0);s.metrics.shadow.enabled=false;
    scroll.SetCustomStyle(s).SetBaseColors(Bg(),Bg(),Ink(),0,0).SetScrollMode(UIPANELSCROLL_VERTICAL);
    Add(scroll.SizePos());scroll.Content().Add(canvas);canvas.WhenExtent=[this]{Layout();};
}
void CirclePage::Layout()
{
    int width=max(850,Logical(GetSize().cx)-18);Place(canvas,0,0,width,canvas.ContentHeight(width));scroll.Layout();
}
int CircleCanvas::SelfTest(const String& folder)
{
    int passed=0,failed=0;String report;
    auto check=[&](bool ok,const char* name){if(ok)passed++;else{failed++;report<<"FAIL: "<<name<<"\n";}};
    auto eq=[](double a,double b){return std::abs(a-b)<1e-8;};
    check(geometry.IsValid()&&eq(geometry.templateR,114.5),"default geometry");
    hole.SetTextUtf8("32");hole.WhenChange();check(eq(geometry.templateR,109),"live numeric input");
    diameter.WhenAction();check(eq(hole.GetValue(),64)&&eq(input.holeRadiusMm,32),"radius to diameter control");
    ChangeUnit("cm");check(eq(hole.GetValue(),6.4)&&eq(geometry.templateR,109),"mm to cm control");
    ChangeUnit("mm");radius.WhenAction();check(eq(hole.GetValue(),32),"round trip display");
    hole.SetTextUtf8("");hole.WhenChange();check(!geometry.IsValid()&&!save.IsEnabled(),"incomplete input disables save");
    hole.SetTextUtf8("3");hole.WhenChange();check(!geometry.IsValid(),"undersized hole");
    hole_slider.SetValue(37.5);hole_slider.WhenChanging();check(geometry.IsValid()&&eq(input.holeRadiusMm,37.5),"live slider binding");
    library_path=AppendFileName(folder,"ui-test-"+AsString(Uuid::Create())+".json");disk_text.Clear();disk_exists=false;
    name.SetTextUtf8("Native test Ø75");notes.SetTextUtf8("Cutting edges, not the shank.");
    check(SaveDesign(false),"save design to disk");check(!dirty&&current==0,"save clears dirty flag");
    NewDesign(false);LoadDesign(0,false);check(name.GetTextUtf8()=="Native test Ø75"&&eq(input.holeRadiusMm,37.5),"reload controls and notes");
    name.SetTextUtf8("Renamed native test");name.WhenChange();check(dirty&&SaveDesign(false),"rename updates design");
    ValueArray ds=library["designs"];check(ds.GetCount()==1,"rename does not duplicate");
    String saved_path=library_path;check(LoadPath(saved_path,false),"reopen JSON library");
    Value before=library;String bad=AppendFileName(folder,"ui-bad-"+AsString(Uuid::Create())+".json");SaveFile(bad,"{invalid");
    check(!LoadPath(bad,false)&&library==before,"corrupt import leaves library intact");FileDelete(bad);
    show_base.SetChecked(false);show_base.WhenAction();check(eq(geometry.templateR,114.5),"diagram visibility cannot change geometry");show_base.SetChecked(true);
    NewDesign(false);input=RouterCalc::Inputs();SyncControls();Recompute(false);
    for(int width:{920,1320}){
        SetRect(0,0,DPI(width),DPI(ContentHeight(width)));Layout();
        check(Rect(GetSize()).Contains(hole.GetRect()),"numeric input within content");check(Rect(GetSize()).Contains(export_file.GetRect()),"save toolbar within content");
        check(Rect(GetSize()).Contains(diagram.GetRect()),"diagram within content");check(!hole.GetRect().Intersects(guide.GetRect()),"input fields do not overlap");
        ImageDraw draw(GetSize());DrawCtrl(draw);Image image=draw;
        check(PNGEncoder().SaveFile(AppendFileName(folder,Format("native-circle-%d.png",width)),image),"native page capture");
    }
    SettingsDialog dlg(library["settings"],saved_path,1);dlg.Layout();ImageDraw settings_draw(dlg.GetSize());dlg.DrawCtrl(settings_draw);
    check(PNGEncoder().SaveFile(AppendFileName(folder,"native-settings.png"),settings_draw),"settings capture");
    FileDelete(saved_path);FileDelete(saved_path+".bak");dirty=false;
    report<<"RouterCalcUi: "<<passed<<" passed, "<<failed<<" failed\n";Cout()<<report;SaveFile(AppendFileName(folder,"ui-test-results.txt"),report);return failed;
}
} }
