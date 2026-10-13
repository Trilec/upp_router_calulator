#include "StavePage.h"
#include <cmath>
namespace Upp { namespace RouterApp {
// Outside slider always stores physical OUTSIDE DIAMETER in millimetres.
// Radius/cm switches only change the edit's presentation, not the slider's meaning.
void StaveCanvas::InitSliders()
{
    for(UiSlider* slider:{&outside_slider,&count_slider}) {
        Add(*slider);slider->ExpandTrack().SetTrackInset(0).SetCancelReverts();
    }
    outside_slider.SetRange(1,1000).SetStep(.1);
    count_slider.SetRange(3,64).SetStep(1);
    outside_slider.Tip("Finished outside size. Typed values can extend the range. Wheel modifiers are available in the number field.");
    count_slider.Tip("Main staves, in whole numbers. Typed counts extend the slider up to 512.");
    outside_slider.WhenChanging=outside_slider.WhenAction=[this]{Slide(OUTER);};
    count_slider.WhenChanging=count_slider.WhenAction=[this]{Slide(COUNT);};
}
void StaveCanvas::SyncSliders()
{
    const bool was_syncing=syncing;syncing=true;
    auto sync=[](UiSlider& slider,double value) {
        if(!std::isfinite(value)||IsNull(value)||value<=0)return;
        // Expand only on an out-of-range edit. Never move the goalposts mid-drag.
        if(value<slider.GetMin())slider.SetMin(value);
        if(value>slider.GetMax())slider.SetMax(value);
        if(slider.GetValue()!=value)slider.SetValue(value);
    };
    sync(outside_slider,input.outerRadiusMm*2);sync(count_slider,input.count);
    syncing=was_syncing;
}
void StaveCanvas::Slide(int field)
{
    if(syncing)return;
    UiSlider& slider=field==OUTER?outside_slider:count_slider;
    const double old=field==OUTER?input.outerRadiusMm*2:input.count;
    double value=slider.GetValue();if(field==COUNT)value=std::round(value);
    if(value==old)return;
    const double display=field==OUTER?value/Factor()/(kind=="radius"?2:1):value;
    if(!CanStep(field,display)) {
        fields[field].SetIssue(true);SyncSliders();return;
    }
    syncing=true;fields[field].SetValue(display);syncing=false;
    last_edited=field;Read();
}
} }
