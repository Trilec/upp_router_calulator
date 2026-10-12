#ifndef _RouterCalculater_Window_h_
#define _RouterCalculater_Window_h_
#include "App.h"
#include "StavePage.h"
namespace Upp { namespace RouterApp {
// Add future calculators here as owned pages, not as modes inside another page.
class RouterWindow : public TopWindow {
    CirclePage circle;
    StavePage staves;
    UiTab tabs;
    UiButton mm, cm, theme;
    SvgView mark;
    String shown_unit, preferences_path, preferences_text;
    bool preferences_exist=false, persist_preferences=true;
    void ApplyTheme();
    void UpdateUnits();
    void ChangeUnit(const String& unit);
    bool SavePreferences();
public:
    RouterWindow(bool load_disk=true);
    void Layout() override;
    void Paint(Draw& w) override;
    bool Key(dword key,int count) override;
    int SelfTest(const String& folder);
};
} }
#endif
