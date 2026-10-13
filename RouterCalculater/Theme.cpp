#include "App.h"
#include <cstring>
namespace Upp { namespace RouterApp {
static bool dark_theme=true;
bool IsDark(){return dark_theme;}
void SetDark(bool dark){dark_theme=dark;}
static const char* Mapped(const char* hex)
{
    if(IsDark())return hex;
    static const char* pairs[][2]={
        {"#131719","#f2f4f4"},{"#1b2023","#ffffff"},{"#111719","#f8fafa"},{"#333b3e","#cbd4d6"},{"#ecede8","#202b2e"},{"#9ca8aa","#53666b"},
        {"#e6b972","#8c5c14"},{"#80d6bb","#19745b"},{"#91b8db","#316f9f"},{"#f09987","#ab4935"},{"#323b3e","#e3eaec"},
        {"#191f22","#f6f9f9"},{"#354148","#dce4e6"},{"#68583d","#c6b899"},{"#292a25","#eee9dc"},{"#24423a","#b7d9ca"},
        {"#99a6ae","#52676f"},{"#758c97","#75858b"},{"#ffd1c4","#743526"},{"#d8deda","#263c37"},{"#1c2428","#eef3f4"},
        {"#38444a","#c4d1d5"},{"#a5b3b8","#536b75"},{"#7e909a","#5c6b72"},{"#543c36","#f1d0c5"},{"#819398","#526b73"},
        {"#9cadb5","#4d6570"},{"#71858f","#536772"},{"#171d20","#f5f8f8"},{"#a0adb1","#506970"},{"#7c898e","#87999f"},
        {"#b1bdc2","#586c75"},{"#242923","#f5efdf"},{"#575039","#cbbd96"},{"#26251f","#faf4e5"},{"#504837","#d2c49f"}
    };
    for(const auto& p:pairs)if(std::strcmp(hex,p[0])==0)return p[1];return hex;
}
Color ThemeColor(const char* hex){int c=ScanInt(Mapped(hex)+1,nullptr,16);return Color((c>>16)&255,(c>>8)&255,c&255);}
Color Bg(){return ThemeColor("#131719");}Color Panel(){return ThemeColor("#1b2023");}Color Field(){return ThemeColor("#111719");}Color Line(){return ThemeColor("#333b3e");}
Color Ink(){return ThemeColor("#ecede8");}Color Muted(){return ThemeColor("#9ca8aa");}Color Amber(){return ThemeColor("#e6b972");}Color Teal(){return ThemeColor("#80d6bb");}Color Blue(){return ThemeColor("#91b8db");}Color Coral(){return ThemeColor("#f09987");}
void StylePrimary(UiButton& b,const String& text){StyleButton(b,text);b.SetBaseColors(Amber(),Amber(),IsDark()?Color(36,30,19):White(),5,8);}
void StyleSlider(UiSlider& s,Color colour)
{
    UiSlider::Style st=UiSlider::StyleDefault();st.track_size=Size(DPI(120),DPI(4));st.thumb_size=Size(DPI(12),DPI(12));
    st.track_metrics.radius=DPI(2);st.track_metrics.frame_width=0;st.thumb_metrics.radius=DPI(6);st.thumb_metrics.frame_width=0;
    st.track_metrics.shadow.enabled=st.thumb_metrics.shadow.enabled=false;
    for(int i=0;i<4;i++){st.track_palette.face[i]=Line();st.track_palette.ink[i]=colour;st.track_palette.frame[i]=colour;st.thumb_palette.face[i]=colour;st.thumb_palette.frame[i]=colour;}
    s.SetCustomStyle(st).SetStep(.1);
}
void StyleScroll(UiScrollPanel& scroll)
{
    UiScrollPanel::Style s=UiScrollPanel::StyleDefault();s.metrics.frame_enabled=false;s.metrics.radius=0;s.metrics.content_margin=Rect(0,0,0,0);s.metrics.shadow.enabled=false;
    scroll.SetCustomStyle(s).SetBaseColors(Bg(),Bg(),Ink(),0,0);
}
} }
