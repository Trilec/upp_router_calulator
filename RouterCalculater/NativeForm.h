#ifndef _RouterCalculater_NativeForm_h_
#define _RouterCalculater_NativeForm_h_
#include <Ui/Ui.h>
namespace Upp { namespace RouterApp {
// Small owner of native presentation controls, not a drawing engine. Declare it
// before borrowed inputs/tables so those controls are destroyed before panels.
class NativeForm {
    struct Caption { UiLabel ctrl; String text, ink; int px=12,panel=0; bool mono=false,bold=false,wrap=false; };
    enum { PANEL_COUNT = 7 };
    UiPanel panels[PANEL_COUNT]; Rect bounds[PANEL_COUNT]; int kinds[PANEL_COUNT]={};
    Index<String> keys; Array<Caption> captions;
    void StylePanel(int i);
    void StyleCaption(Caption& c);
    void PrepareCaption(Caption& c);
public:
    void Init(Ctrl& host);
    void ApplyTheme();
    void Panel(int i,int x,int y,int w,int h,int kind=0);
    void Place(Ctrl& c,int panel,int x,int y,int w,int h);
    void Label(const String& id,int panel,int x,int y,int w,int h,const String& text,int px=12,const char* ink="#9ca8aa",bool mono=false,bool bold=false,bool wrap=false);
    void Text(const String& id,const String& value);
    void Ink(const String& id,const char* colour);
    void Show(const String& id,bool show);
};
void StyleNativeLabel(UiLabel& c,int px,const char* colour,bool mono=false,bool bold=false,bool wrap=false);
void StyleNativeTable(UiTable& table);
void InitReadoutTable(UiTable& table,int rows,const Vector<String>& headers);
bool SetReadoutCell(UiTableModel& model,int row,int col,const String& text);
} }
#endif
