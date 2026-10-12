#ifndef _RouterCalculater_StavePage_h_
#define _RouterCalculater_StavePage_h_
#include "App.h"
namespace Upp { namespace RouterApp {
String StaveSvg(const RouterCalc::StaveInput& x,const RouterCalc::StaveGeometry& g,const String& unit,int selected);
struct StaveCutGroup : Moveable<StaveCutGroup> {String name;int count=0;double outer=0,inner=0,depth=0,bevel=0;};
class StaveCanvas : public Ctrl {
public:
    StaveCanvas(bool load_disk=true);
    void Layout() override;
    void Paint(Draw& w) override;
    int ContentHeight() const {return 990;}
    void ApplyTheme();
    void ChangeUnit(const String& unit);
    String GetUnit() const {return unit;}
    bool AllowDiscard();
    bool Save(bool modal=true);
    void Open();
    int SelfTest(const String& folder);
    Event<> WhenDisplay;
private:
    enum {OUTER,COUNT,WALL,OUT_ALLOW,IN_ALLOW,DEPTH,TOP_TRIM,REACH_GAP,BOTTOM_TRIM,PLATE,KERF,SPACER,EVERY,FIELD_COUNT};
    UiFloatEdit fields[FIELD_COUNT];
    UiLineEdit joints,name,notes;
    UiDropdown saved,inspect;
    UiButton radius,diameter,create,save,settings,export_file;
    SvgView preview;
    RouterCalc::StaveInput input;
    RouterCalc::StaveGeometry geometry;
    Vector<StaveCutGroup> groups;
    Value library;
    String unit="mm",kind="diameter",path,disk_text,status="Not saved yet";
    bool disk_exists=false,dirty=false,syncing=false;
    int current=-1,selection_count=0,selected=0,applied_theme=-1,left_width=320,right_x=336,right_width=966;
    Image left_back,result_back,preview_back,list_back;
    double Factor() const {return unit=="cm"?10:1;}
    String M(double value) const;
    void Sync();
    void Read();
    void Recompute(bool changed=true);
    void ChangeKind(const String& next);
    void New(bool ask=true);
    bool LoadPath(const String& filename,bool modal=true);
    void LoadDesign(int index,bool ask=true);
    void RefreshList();
    bool Stage(Value& next,int& index,String& error);
    void Settings();
    void Export();
    void Error(const String& message,bool modal=true);
};
class StavePage : public Ctrl {
public:
    StaveCanvas canvas;
    UiScrollPanel scroll;
    StavePage(bool load_disk=true);
    void Layout() override;
    void ApplyTheme();
};
} }
#endif
