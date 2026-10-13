#include "App.h"
namespace Upp { namespace RouterApp {
Font TextFont(int px,bool mono,bool bold)
{
    static const int mono_face=Font::FindFaceNameIndex("Consolas");
    static const int sans_face=Font::FindFaceNameIndex("Segoe UI");
    int face=mono?mono_face:sans_face;
    Font f(face>=0?face:Font::GetStdFont().GetFace(),DPI(px));
    return bold?f.Bold():f;
}
void StyleButton(UiButton& b,const String& text,bool selected)
{
    UiButton::Style s=UiButton::StyleDefault();
    s.font=TextFont(12);s.metrics.radius=DPI(7);s.metrics.frame_width=DPI(1);
    s.metrics.content_margin=Rect(DPI(8),DPI(3),DPI(8),DPI(3));
    s.metrics.shadow.enabled=false;s.metrics.focus_color=Teal();
    b.SetCustomStyle(s).SetText(text);
    b.SetBaseColors(selected?ThemeColor("#323b3e"):Panel(),selected?Amber():Line(),selected?Ink():Muted(),8,12);
}
void StyleEdit(UiBaseEdit& e,int px)
{
    UiBaseEdit::Style s=UiBaseEdit::StyleDefault();
    s.font=TextFont(px,px>18);s.metrics.radius=DPI(7);s.metrics.shadow.enabled=false;
    s.metrics.content_margin=Rect(DPI(px>18?11:9),DPI(3),DPI(px>18?38:9),DPI(3));
    s.caret_color=Teal();s.selection_color=IsDark()?Color(51,90,80):Color(187,220,210);s.selection_ink=Ink();s.placeholder_ink=Muted();
    e.SetCustomStyle(s).SetBaseColors(Field(),Line(),Ink(),0,0).SetFocusColor(Teal());
}
void StyleCheck(UiCheckBox& c,const String& text)
{
    UiCheckBox::Style s=UiCheckBox::StyleDefault();
    s.font=TextFont(11);s.indicator_size=DPI(13);s.indicator_gap=DPI(5);
    s.metrics.face_enabled=false;s.metrics.frame_enabled=false;s.metrics.shadow.enabled=false;
    s.indicator_metrics.radius=DPI(2);s.indicator_metrics.shadow.enabled=false;
    for(int i=0;i<4;i++){s.palette.ink[i]=Muted();s.indicator_palette.ink[i]=Bg();s.indicator_palette.frame[i]=Blue();s.indicator_palette.face[i]=Blue();}
    c.SetCustomStyle(s).SetText(text);
}
void StyleDropdown(UiDropdown& d)
{
    UiDropdown::Style s=UiDropdown::StyleDefault();
    for(int i=0;i<4;i++)s.palette.icon[i]=Ink();
    s.font=TextFont(12);s.metrics.radius=DPI(7);s.metrics.shadow.enabled=false;
    s.metrics.content_margin=Rect(DPI(9),DPI(4),DPI(9),DPI(4));
    s.popup_item_style.font=TextFont(12);s.popup_background_color=Panel();s.popup_frame_color=Line();
    for(int i=0;i<4;i++){s.popup_item_style.palette.ink[i]=Ink();s.popup_item_style.palette.face[i]=Panel();}
    d.SetCustomStyle(s).SetBaseColors(Field(),Line(),Ink(),5,8);
    d.ShowDragHandle(false).ShowSelectionBadge(false).SetPopupItemHeight(DPI(32));
}
} }
