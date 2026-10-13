#ifndef _RouterCalculater_UnitEdit_h_
#define _RouterCalculater_UnitEdit_h_
#include <Ui/Ui.h>
namespace Upp { namespace RouterApp {
// Native Ui numeric editor with workshop-specific units and wheel increments.
// Typed getters are used through UnitEdit, never a UiFloatEdit pointer. Text
// remains editable when incomplete; programmatic setters do not commit events.
class UnitEdit : public UiFloatEdit {
    String unit="mm";bool whole=false,issue=false;int wheel_remainder=0;
    void Nudge(int ticks,dword modifiers);
public:
    Gate<double> WhenStep; // candidate in displayed units; false stops at a physical limit
    void WholeNumbers(bool value=true){whole=value;}
    void DisplayUnit(const String& value){unit=value;}
    void ConvertUnit(const String& value);
    bool TryGetValue(double& value) const;
    double GetValue() const {double v;return TryGetValue(v)?v:double(Null);}
    void SetValue(double value);
    void SetIssue(bool value);
    bool HasIssue() const{return issue;}
    bool Key(dword key,int count) override;
    void MouseWheel(Point p,int zdelta,dword flags) override;
    void LostFocus() override;
    void Paint(Draw& w) override;
    void SetData(const Value& v) override;
    Value GetData() const override {return GetValue();}
};
} }
#endif
