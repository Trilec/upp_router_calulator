#include "StavePage.h"
#include <cmath>
namespace Upp { namespace RouterApp {
using RouterCalc::Number;
String StaveCanvas::M(double value) const { return Number(value/Factor())+" "+unit; }
StaveCanvas::StaveCanvas(bool load_disk)
{
    library=RouterCalc::NewStaveLibrary();path=AppendFileName(GetExeFolder(),"router_calculator.staves.json");
    for(auto& f:fields){Add(f);f.ShowSpin(false).Precision(5);f.WhenChange=[this]{if(!syncing)Read();};f.WhenAction=[this]{if(!syncing)Read();};}
    fields[COUNT].Step(1);fields[EVERY].Step(1);
    fields[OUTER].Tip("Finished outside size, excluding the material reserved for milling.");
    fields[COUNT].Tip("Number of main staves. Straight spacer strips do not add angular divisions.");
    fields[WALL].Tip("Finished radial wall thickness. Outside radius minus inside radius.");
    fields[OUT_ALLOW].Tip("Minimum radial material outside the finished shell, at the middle of each flat.");
    fields[IN_ALLOW].Tip("Minimum radial material inside the finished bore, including the inner polygon corners.");
    fields[REACH_GAP].Tip("Measured axial gap from the plate TOP to the lowest surface your complete cutter and holder can machine. Not the router base diameter.");
    fields[PLATE].Tip("Separate mounting plate below Z=0. Adds assembly height, not stave length.");
    fields[BOTTOM_TRIM].Tip("Extra axial trimming stock beyond the inaccessible mounting-end section.");
    fields[KERF].Tip("One separating end cut per length, after squaring the stock. Stock estimate only; never subtracted from glued dimensions.");
    fields[SPACER].Tip("Thickness normal to the parallel strip faces. Zero means no strips. These are straight-cut, not wedge-shaped.");
    fields[EVERY].Tip("Insert a strip after every Nth stave. A selected-joint list overrides this cadence.");
    Add(joints);joints.SetPlaceholder("Optional: 3,6,9 — overrides cadence");joints.WhenChange=[this]{if(!syncing)Read();};
    Add(name);Add(notes);name.SetPlaceholder("Design name");notes.SetPlaceholder("Fixture, timber or cutter notes");
    name.WhenChange=notes.WhenChange=[this]{if(!syncing){dirty=true;status="Unsaved changes";Refresh();}};
    Add(saved);saved.SetPlaceholderText("No saved shell designs");saved.WhenSelect=[this](int i){if(!syncing&&i>=0)LoadDesign(i);};
    Add(inspect);inspect.WhenSelect=[this](int i){if(!syncing&&i>=0&&geometry.IsValid()){selected=i;preview.SetSource(StaveSvg(input,geometry,unit,selected));Refresh();}};
    for(UiButton* b:{&radius,&diameter,&create,&save,&settings,&export_file})Add(*b);
    radius.WhenAction=[this]{ChangeKind("radius");};diameter.WhenAction=[this]{ChangeKind("diameter");};create.WhenAction=[this]{New();};
    save.WhenAction=[this]{Save();};settings.WhenAction=[this]{Settings();};export_file.WhenAction=[this]{Export();};Add(preview);
    syncing=true;name.SetTextUtf8("Untitled shell");syncing=false;ApplyTheme();Sync();Recompute(false);
    if(load_disk&&FileExists(path))LoadPath(path,false);
}
void StaveCanvas::ApplyTheme()
{
    int theme=IsDark()?1:0;if(applied_theme==theme)return;applied_theme=theme;
    for(auto& f:fields)StyleEdit(f,18);StyleEdit(fields[OUTER],22);StyleEdit(joints);StyleEdit(name);StyleEdit(notes);
    StyleDropdown(saved);StyleDropdown(inspect);StyleButton(radius,"Radius",kind=="radius");StyleButton(diameter,"Diameter",kind=="diameter");StyleButton(create,"+ New");
    StylePrimary(save,"Save");StyleButton(settings,"Settings");StyleButton(export_file,"Export");
    left_back=result_back=preview_back=list_back=Image();Layout();preview.Layout();Refresh();
}
void StaveCanvas::Sync()
{
    syncing=true;
    double a[]={input.outerRadiusMm*(kind=="diameter"?2:1),double(input.count),input.wallMm,input.outsideAllowanceMm,input.insideAllowanceMm,input.depthMm,input.topTrimMm,input.reachGapMm,input.bottomTrimMm,input.basePlateMm,input.endKerfMm,input.spacerThicknessMm,double(input.spacerEvery)};
    for(int i=0;i<FIELD_COUNT;i++){double v=a[i]/((i==COUNT||i==EVERY)?1:Factor()),old;if(!fields[i].TryGetValue(old)||old!=v)fields[i].SetValue(v);}
    if(joints.GetTextUtf8()!=input.selectedJoints)joints.SetTextUtf8(input.selectedJoints);
    syncing=false;WhenDisplay();
}
void StaveCanvas::Read()
{
    double a[FIELD_COUNT];String problem;
    for(int i=0;i<FIELD_COUNT;i++)if(!fields[i].TryGetValue(a[i])||!std::isfinite(a[i])||IsNull(a[i])){problem="Complete every measurement with a finite number.";break;}
    if(problem.IsEmpty())for(int i:{COUNT,EVERY})if(a[i]!=std::floor(a[i])||a[i]<1||a[i]>512)problem="Counts and cadence must be whole numbers from 1 to 512.";
    if(!problem.IsEmpty()){
        geometry=RouterCalc::StaveGeometry();geometry.error=problem;groups.Clear();save.Disable();export_file.Disable();inspect.Disable();dirty=true;status=problem;preview.SetSource(StaveSvg(input,geometry,unit,selected));Refresh();return;
    }
    for(int i=0;i<FIELD_COUNT;i++)if(i!=COUNT&&i!=EVERY)a[i]*=Factor();
    RouterCalc::StaveInput next;next.outerRadiusMm=a[OUTER]/(kind=="diameter"?2:1);next.count=(int)a[COUNT];next.wallMm=a[WALL];next.outsideAllowanceMm=a[OUT_ALLOW];next.insideAllowanceMm=a[IN_ALLOW];next.depthMm=a[DEPTH];next.topTrimMm=a[TOP_TRIM];next.reachGapMm=a[REACH_GAP];next.bottomTrimMm=a[BOTTOM_TRIM];next.basePlateMm=a[PLATE];next.endKerfMm=a[KERF];next.spacerThicknessMm=a[SPACER];next.spacerEvery=(int)a[EVERY];next.selectedJoints=joints.GetTextUtf8();
    // Both text-change and committed-action events may fire; identical state does no work.
    if(geometry.IsValid()&&RouterCalc::StaveParameters(next)==RouterCalc::StaveParameters(input))return;
    input=next;Recompute();
}
void StaveCanvas::Recompute(bool changed)
{
    geometry=RouterCalc::CalculateStaves(input);groups.Clear();if(changed){dirty=true;status="Unsaved changes";}
    bool valid=geometry.IsValid();save.Enable(valid);export_file.Enable(valid);inspect.Enable(valid);
    if(valid){
        if(selection_count!=input.count){syncing=true;inspect.Clear();for(int i=0;i<input.count;i++)inspect.Add(Format("Inspect stave %d",i+1),i);selected=min(selected,input.count-1);inspect.Select(selected);selection_count=input.count;syncing=false;}
        for(const auto& p:geometry.staves){
            String label=p.leftSpacerMm>0?(p.rightSpacerMm>0?"Strip both sides":"Strip on left"):(p.rightSpacerMm>0?"Strip on right":"Plain stave");
            int found=-1;for(int i=0;i<groups.GetCount();i++)if(groups[i].name==label)found=i;
            if(found<0){auto& q=groups.Add();q.name=label;q.outer=p.outerWidthMm;q.inner=p.innerWidthMm;q.depth=geometry.stockThicknessMm;q.bevel=geometry.bevelDegrees;found=groups.GetCount()-1;}groups[found].count++;
        }
        if(geometry.spacers.GetCount()){auto& q=groups.Add();q.name="Straight strip";q.count=geometry.spacers.GetCount();q.outer=q.inner=input.spacerThicknessMm;q.depth=geometry.spacers[0].radialOuterMm-geometry.spacers[0].radialInnerMm;}
    }else status=geometry.error;
    fields[EVERY].Enable(input.spacerThicknessMm>0);joints.Enable(input.spacerThicknessMm>0);
    preview.SetSource(StaveSvg(input,geometry,unit,selected));Refresh();
}
void StaveCanvas::ChangeUnit(const String& next)
{
    if(next==unit)return;if(!geometry.IsValid()){Error("Complete the measurements before changing units.");return;}unit=next;Sync();Recompute();
}
void StaveCanvas::ChangeKind(const String& next)
{
    if(next==kind)return;if(!geometry.IsValid()){Error("Complete the measurements before changing radius/diameter.");return;}
    kind=next;StyleButton(radius,"Radius",kind=="radius");StyleButton(diameter,"Diameter",kind=="diameter");Sync();Recompute();
}
void StaveCanvas::Error(const String& message,bool modal){status=message;Refresh();if(modal)Exclamation(DeQtf(message));}
bool StaveCanvas::AllowDiscard(){if(!dirty)return true;int a=PromptYesNoCancel("Save changes to the current stave shell?");return a>=0&&(a==0||Save());}
void StaveCanvas::New(bool ask)
{
    if(ask&&!AllowDiscard())return;input=RouterCalc::StaveInput();current=-1;selected=0;syncing=true;name.SetTextUtf8("Untitled shell");notes.Clear();saved.ClearSelection();syncing=false;dirty=false;status="Not saved yet";Sync();Recompute(false);
}
void StaveCanvas::RefreshList()
{
    syncing=true;saved.Clear();ValueArray ds=library["designs"];for(int i=0;i<ds.GetCount();i++)saved.Add((String)ds[i]["name"],i);
    if(current>=0&&current<ds.GetCount())saved.Select(current);else saved.ClearSelection();syncing=false;
}
void StaveCanvas::LoadDesign(int index,bool ask)
{
    if(ask&&!AllowDiscard()){RefreshList();return;}ValueArray ds=library["designs"];if(index<0||index>=ds.GetCount())return;
    Value d=ds[index];String error;RouterCalc::StaveInput next;
    if(!RouterCalc::DecodeStaveParameters(d["parameters"],next,error)){Error(error);return;}
    input=next;current=index;unit=(String)d["unit"];kind=(String)d["kind"];selected=0;
    syncing=true;name.SetTextUtf8((String)d["name"]);notes.SetTextUtf8(d["notes"].Is<String>()?(String)d["notes"]:String());saved.Select(index);syncing=false;
    StyleButton(radius,"Radius",kind=="radius");StyleButton(diameter,"Diameter",kind=="diameter");
    dirty=false;status="Loaded saved shell";Sync();Recompute(false);
}
bool StaveCanvas::Stage(Value& next,int& index,String& error)
{
    if(!geometry.IsValid()){error=geometry.error;return false;}
    String title=TrimBoth(name.GetTextUtf8());if(title.IsEmpty()||title.GetCount()>100){error="Use a design name of 1 to 100 characters.";return false;}
    if(notes.GetTextUtf8().GetCount()>2000){error="Notes are limited to 2,000 bytes.";return false;}
    ValueArray ds=library["designs"];ValueMap d;index=current;
    d.Add("id",index>=0&&index<ds.GetCount()?ds[index]["id"]:Value(AsString(Uuid::Create())));d.Add("name",title);d.Add("parameters",RouterCalc::StaveParameters(input));d.Add("unit",unit);d.Add("kind",kind);d.Add("notes",notes.GetTextUtf8());
    if(index>=0&&index<ds.GetCount())ds.Set(index,d);else{index=ds.GetCount();ds.Add(d);}
    ValueMap root(library);root.Set("designs",ds);next=root;return RouterCalc::ValidateStaveLibrary(next,error);
}
bool StaveCanvas::Save(bool modal)
{
    Value next;int index;String error;
    if(!Stage(next,index,error)||!RouterCalc::WriteJsonFile(path,next,disk_text,disk_exists,error)){Error(error,modal);return false;}
    library=next;current=index;dirty=false;status="Saved to "+GetFileName(path);RefreshList();Refresh();return true;
}
bool StaveCanvas::LoadPath(const String& filename,bool modal)
{
    FileIn f(filename);if(!f.IsOpen()||f.GetSize()>RouterCalc::MAX_LIBRARY_BYTES){Error("Cannot read the library, or it exceeds 2 MiB.",modal);return false;}
    String text=f.Get((int)f.GetSize()),error;Value next;
    try{next=ParseJSON(text);}catch(...){Error("Invalid JSON; the current designs were kept.",modal);return false;}
    if(!RouterCalc::ValidateStaveLibrary(next,error)){Error(error,modal);return false;}
    library=next;path=filename;disk_text=text;disk_exists=true;current=-1;dirty=false;RefreshList();
    ValueArray ds=library["designs"];if(ds.GetCount())LoadDesign(ds.GetCount()-1,false);else New(false);
    status="Opened "+GetFileName(path);Refresh();return true;
}
void StaveCanvas::Open()
{
    if(!AllowDiscard())return;FileSel fs;fs.Type("Stave design library","*.json").AllFilesType();fs.Set(path);
    if(fs.ExecuteOpen("Open stave designs"))LoadPath(~fs);
}
// Small native chooser; the Export button remains one action in the main page.
class StaveChoice : public TopWindow {
    UiButton buttons[5];String heading,detail;int count;
public:
    StaveChoice(const String& title,const String& explanation,const Vector<String>& labels):heading(title),detail(explanation),count(labels.GetCount())
    {
        Title(title);SetRect(0,0,DPI(440),DPI(135+count*40));
        for(int i=0;i<count;i++){StyleButton(buttons[i],labels[i]);Add(buttons[i]);buttons[i].WhenAction=[this,i]{Break(i+1);};}
        WhenClose=[this]{Break(0);};
    }
    void Layout() override {for(int i=0;i<count;i++)buttons[i].SetRect(DPI(22),DPI(112+i*40),DPI(396),DPI(32));}
    void Paint(Draw& w) override {w.DrawRect(GetSize(),Bg());TextAt(w,22,20,heading,21,Ink(),false,true);Wrap(w,22,56,396,detail,12,Muted());}
    void EnableChoice(int i,bool enabled){buttons[i].Enable(enabled);}
};
void StaveCanvas::Settings()
{
    Vector<String> labels;labels<<"Open library…"<<"Export design library…"<<"Duplicate current design"<<"Delete selected design"<<"Close";
    StaveChoice dlg("Saved shells",GetFileName(path)+" — local JSON. Saves retain a .bak copy and reject external edits.",labels);dlg.EnableChoice(2,geometry.IsValid());dlg.EnableChoice(3,current>=0);
    int choice=dlg.Run();
    if(choice==1)Open();
    if(choice==2){Export();}
    if(choice==3){if(!AllowDiscard())return;current=-1;syncing=true;name.SetTextUtf8(name.GetTextUtf8()+" copy");saved.ClearSelection();syncing=false;dirty=true;Save();}
    if(choice==4&&current>=0){
        if(!AllowDiscard()||!PromptYesNo("Delete the selected saved shell? The previous library will remain in .bak."))return;
        ValueArray ds=library["designs"];ds.Remove(current);ValueMap next(library);next.Set("designs",ds);String error;
        if(!RouterCalc::WriteJsonFile(path,next,disk_text,disk_exists,error)){Error(error);return;}
        library=next;current=-1;RefreshList();New(false);status="Design deleted; backup retained";Refresh();
    }
}
void StaveCanvas::Export()
{
    if(!geometry.IsValid()){Error(geometry.error);return;}
    Vector<String> labels;labels<<"Geometry JSON…"<<"Cut list CSV…"<<"Design library JSON…"<<"Cancel";
    StaveChoice dlg("Export shell", "Geometry is for inspection and future tooling. It is not G-code or a robot trajectory.",labels);int choice=dlg.Run();if(choice<1||choice>3)return;
    FileSel fs;String text;Value next;int index=-1;String error;
    if(choice==3){if(!Stage(next,index,error)){Error(error);return;}text=AsJSON(next,true);fs.Type("Stave design library","*.json").DefaultExt("json");fs.Set(path);}
    else if(choice==1){text=AsJSON(RouterCalc::StaveGeometryExport(input,geometry),true);fs.Type("Geometry JSON","*.json").DefaultExt("json");fs.Set(AppendFileName(GetFileFolder(path),"shell.geometry.json"));}
    else{text=RouterCalc::StaveCutListCsv(input,geometry);fs.Type("Cut list CSV","*.csv").DefaultExt("csv");fs.Set(AppendFileName(GetFileFolder(path),"shell.cutlist.csv"));}
    if(!fs.ExecuteSaveAs("Export shell"))return;String destination=~fs,expected;bool exists=FileExists(destination);
    if(NormalizePath(destination)==NormalizePath(path)){
        if(choice!=3){Error("Choose another filename. Geometry and cut lists must not replace the design library.");return;}expected=disk_text;exists=disk_exists;
    }else if(exists)expected=LoadFile(destination);
    if(!RouterCalc::WriteTextFile(destination,text,expected,exists,error)){Error(error);return;}
    if(choice==3){library=next;current=index;path=destination;disk_text=expected;disk_exists=true;dirty=false;RefreshList();}
    status="Exported "+GetFileName(destination);Refresh();
}
} }
