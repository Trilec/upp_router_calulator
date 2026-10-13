#include "UnitEdit.h"
#include <cmath>
namespace Upp { namespace RouterApp {
Color Coral();
bool UnitEdit::TryGetValue(double& value) const
{
    String text=ToLower(TrimBoth(GetTextUtf8()));double factor=1;
    if(text.EndsWith("mm")||text.EndsWith("cm")){
        if(whole)return false;
        factor=text.EndsWith("cm")?10:1;text=TrimBoth(text.Left(text.GetCount()-2));factor/=(unit=="cm"?10:1);
    }
    if(text.IsEmpty())return false;
    int i=0;if(text[i]=='+'||text[i]=='-')++i;bool digit=false,decimal=false;
    for(;i<text.GetCount()&&text[i]!='e';i++){
        if(IsDigit(text[i]))digit=true;else if(text[i]=='.'&&!decimal)decimal=true;else return false;
    }
    if(!digit)return false;
    if(i<text.GetCount()){
        ++i;if(i<text.GetCount()&&(text[i]=='+'||text[i]=='-'))++i;
        int first=i;while(i<text.GetCount()&&IsDigit(text[i]))++i;
        if(i==first||i!=text.GetCount())return false;
    }
    value=ScanDouble(text)*factor;return std::isfinite(value)&&!IsNull(value);
}
void UnitEdit::SetValue(double value)
{
    String text=IsNull(value)?String():Format("%.12g",value);
    if(GetTextUtf8()!=text)SetTextUtf8(text);
}
void UnitEdit::ConvertUnit(const String& next)
{
    if(next==unit)return;double value;bool complete=TryGetValue(value);
    double factor=(unit=="cm"?10.0:1.0)/(next=="cm"?10.0:1.0);unit=next;
    if(complete&&!whole)SetValue(value*factor);
}
void UnitEdit::SetIssue(bool value){if(issue!=value){issue=value;Refresh();}}
void UnitEdit::Nudge(int ticks,dword flags)
{
    if(IsReadOnly()||!IsEnabled())return;double value;if(!TryGetValue(value))return;
    double step=whole?1:.1;if(flags&K_ALT)step=whole?10:1;if(flags&K_SHIFT)step=whole?100:10;
    double next=value+ticks*step;if(whole)next=std::round(next);
    if(next==value||!std::isfinite(next))return;
    if(WhenStep&&!WhenStep(next)){SetIssue(true);return;}
    SetIssue(false);SetValue(next);WhenAction();
}
bool UnitEdit::Key(dword key,int count)
{
    dword bare=key&~(K_ALT|K_SHIFT|K_CTRL);
    if(bare==K_UP||bare==K_DOWN){Nudge((bare==K_UP?1:-1)*max(1,count),key);return true;}
    // Explicit cm/mm may be typed or pasted; all other syntax is validated as a whole.
    if(!whole&&(key=='c'||key=='C'||key=='m'||key=='M'||key==' '))return UiBaseEdit::Key(key,count);
    return UiFloatEdit::Key(key,count);
}
void UnitEdit::MouseWheel(Point,int delta,dword flags)
{
    wheel_remainder+=delta;int ticks=wheel_remainder/120;wheel_remainder%=120;if(ticks)Nudge(ticks,flags);
}
void UnitEdit::LostFocus()
{
    double value;if(TryGetValue(value))SetValue(value);
    // Do not let UiFloatEdit's raw-number formatter reinterpret explicit suffixes.
    UiBaseEdit::LostFocus();
}
void UnitEdit::Paint(Draw& w)
{
    UiBaseEdit::Paint(w);
    if(issue){
        const auto& style=GetStyle();StyledPalette palette=style.palette;StyledMetrics metrics=style.metrics;
        for(int i=0;i<4;i++)palette.frame[i]=Coral();
        metrics.face_enabled=false;metrics.frame_enabled=true;metrics.frame_width=DPI(2);metrics.shadow.enabled=false;
        UiPaintFaceFrameDash(w,Rect(GetSize()),palette,metrics,ST_NORMAL);
    }
}
void UnitEdit::SetData(const Value& value){if(IsNull(value))Clear();else if(IsNumber(value))SetValue((double)value);else SetTextUtf8(value.ToString());}
} }
