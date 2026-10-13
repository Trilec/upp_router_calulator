#ifndef _RouterCalculater_App_h_
#define _RouterCalculater_App_h_
#include <Ui/Ui.h>
#include <Painter/Painter.h>
#include <plugin/png/png.h>
#include <RouterCalcCore/RouterCalcCore.h>
#include <RouterCalcCore/Staves.h>
#include "Diagrams.h"
#include "NativeForm.h"
#include "UnitEdit.h"

namespace Upp { namespace RouterApp {
// Fixed logical height: window resizing must never make a preview taller.
constexpr int PREVIEW_HEIGHT=324;
void StyleScroll(UiScrollPanel& scroll);
bool IsDark(); void SetDark(bool dark);
Color ThemeColor(const char* dark_hex);
void StylePrimary(UiButton& b, const String& text);
void StyleSlider(UiSlider& s, Color colour);
Color Bg(); Color Panel(); Color Field(); Color Line(); Color Ink(); Color Muted();
Color Amber(); Color Teal(); Color Blue(); Color Coral();
Font TextFont(int px, bool mono = false, bool bold = false);
void StyleButton(UiButton& b, const String& text, bool selected = false);
void StyleEdit(UiBaseEdit& e, int px = 12);
void StyleCheck(UiCheckBox& c, const String& text);
void StyleDropdown(UiDropdown& d);
// A calculator page owns its controls; the non-GUI core owns equations/schema.
// Sizes here are authored in logical pixels, converted once in Layout/Paint.
class CircleCanvas : public Ctrl {
    friend struct PerformanceAudit;
public:
    CircleCanvas(bool load_disk = true);
    virtual void Layout() override;
    virtual void Paint(Draw& w) override;
    int ContentHeight(int width) const;
    void ChangeUnit(const String& next);
    void ApplyTheme();
    bool AllowDiscard();
    bool SaveDesign(bool show_error = true);
    bool ExportLibrary();
    void OpenLibrary();
    void Settings();
    void CopyMeasurements();
    int SelfTest(const String& capture_folder);
    Event<> WhenDisplay;
    Event<> WhenExtent;
    String GetUnit() const { return unit; }
private:
    RouterCalc::Inputs input;
    RouterCalc::Geometry geometry;
    Value library;
    String unit = "mm", kind = "radius", library_path, disk_text, status;
    bool disk_exists = false, dirty = false, syncing = false, notes_open = false;
    int current = -1;
    int applied_theme=-1;
    String styled_kind;
    int left_width = 334, right_x = 352, right_width = 968;
    int diagram_height = 642, table_y = 817, below_y = 1158;
    int notes_extra = 0;
    NativeForm form;
    UiTable dimensions;
    int last_edited=0;
    UnitEdit hole, guide, cutter;
    UiSlider hole_slider, guide_slider, cutter_slider;
    Label suffix[3];
    UiButton radius, diameter, new_design, save, settings, export_file, note_toggle, copy;
    UiButton formula_more, safety_more;
    bool expanded_formula = false, expanded_safety = false;
    UiDropdown saved;
    UiLineEdit name;
    UiMultiEdit notes;
    UiCheckBox show_base, show_path, show_radii;
    DiagramView diagram, cutter_picture;
    double Factor() const { return unit == "cm" ? 10 : 1; }
    String Measure(double mm) const;
    void Recompute(bool changed = true);
    void SyncControls();
    void ReadInputs(bool force=false);
    bool CanStep(int field,double value);
    void UpdateReadouts();
    void SetSlider(UiSlider& s, double value, double low, double high, Color colour);
    void ChangeKind(const String& next);
    void NewDesign(bool ask = true);
    void LoadDesign(int index, bool ask = true);
    void RefreshDesignList();
    bool StageCurrent(Value& candidate, int& index, String& error);
    void ReportError(const String& error, bool modal = true);
    void ApplySettings(const Value& settings);
    bool LoadPath(const String& path, bool modal = true);
};

class CirclePage : public Ctrl {
public:
    CircleCanvas canvas;
    UiScrollPanel scroll;
    CirclePage(bool load_disk = true);
    virtual void Layout() override;
};

} }
#endif
