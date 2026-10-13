#include "StavePage.h"
namespace Upp { namespace RouterApp {
using RouterCalc::Number;
static int SLogical(int px) { return int(px * 100.0 / DPI(100) + .5); }
static void SPlace(Ctrl& c, int x, int y, int w, int h) { c.SetRect(DPI(x), DPI(y), DPI(w), DPI(h)); }

void StaveCanvas::Layout()
{
    const int width = SLogical(GetSize().cx);
    if(width < 700) return;
    left_width = 332; right_x = left_width + 12; right_width = width - right_x;
    const int main_y = 76, preview_y = 150, preview_h = 324, list_y = 484;
    const int pad = 14, col = (left_width - 2 * pad - 16) / 3;
    form.Panel(4, 0, 0, width, 66);
    form.Panel(0, 0, main_y, left_width, 486);
    form.Panel(1, right_x, main_y, right_width, 64, 1);
    form.Panel(2, right_x, preview_y, right_width, preview_h);
    form.Panel(3, right_x, list_y, right_width, 244);
    form.Panel(5, 0, 572, left_width, expanded_formula ? 318 : 156);
    auto put = [&](Ctrl& c, int p, int x, int y, int w, int h) { form.Place(c, p, x, y, w, h); };

    // The same order as the circle page: choose/name a design, then edit it.
    const int actions = width - 320, picker = min(300, (actions - 40) / 2);
    form.Label("saved.label", 4, 14, 8, 148, 16, "SAVED DESIGNS", 10);
    form.Label("status", 4, 170, 8, width - 184, 17, "", 10);
    put(saved, 4, 14, 28, picker, 28);
    put(name, 4, 24 + picker, 28, actions - picker - 34, 28);
    put(create, 4, actions, 28, 65, 28);
    put(save, 4, actions + 73, 28, 65, 28);
    put(settings, 4, actions + 146, 28, 77, 28);
    put(export_file, 4, actions + 231, 28, 75, 28);

    form.Label("title", 0, pad, main_y + 12, 132, 20, "Your shell", 14, "#ecede8", false, true);
    put(radius, 0, 180, main_y + 10, 57, 25);
    put(diameter, 0, 241, main_y + 10, 77, 25);
    form.Label("outside.label", 0, pad, main_y + 44, 202, 17, "", 11, "#ecede8", false, true);
    form.Label("field.count", 0, 230, main_y + 44, 88, 17, "Main staves", 11, "#ecede8", false, true);
    put(fields[OUTER], 0, pad, main_y + 63, 202, 30);
    put(fields[COUNT], 0, 230, main_y + 63, 88, 30);
    put(outside_slider, 0, pad, main_y + 98, 202, 17);
    put(count_slider, 0, 230, main_y + 98, 88, 17);
    auto field = [&](int id, int column, int y, const char* label) {
        const int x = pad + column * (col + 8);
        form.Label(Format("field.%d", id), 0, x, y, col, 17, label, 10);
        put(fields[id], 0, x, y + 18, col, 28);
    };
    field(WALL, 0, main_y + 126, "Wall (radial)");
    field(OUT_ALLOW, 1, main_y + 126, "Outside pad");
    field(IN_ALLOW, 2, main_y + 126, "Inside pad");
    form.Label("unit", 0, pad, main_y + 181, left_width - 2 * pad, 17, "", 10);

    form.Label("depth.title", 0, pad, main_y + 210, left_width - 2 * pad, 18, "DEPTH & MOUNTING", 10, "#e6b972");
    field(DEPTH, 0, main_y + 234, "Finished depth");
    field(TOP_TRIM, 1, main_y + 234, "Top trim");
    field(BOTTOM_TRIM, 2, main_y + 234, "Bottom trim");
    field(REACH_GAP, 0, main_y + 291, "Router end gap");
    field(PLATE, 1, main_y + 291, "Plate thickness");
    field(KERF, 2, main_y + 291, "End-cut kerf");

    form.Label("strips.title", 0, pad, main_y + 350, left_width - 2 * pad, 18, "OPTIONAL STRAIGHT STRIPS", 10, "#f09987");
    field(SPACER, 0, main_y + 374, "Strip thickness");
    field(EVERY, 1, main_y + 374, "Every Nth joint");
    form.Label("joints.label", 0, pad + 2 * (col + 8), main_y + 374, col, 17, "Or joint list", 10);
    put(joints, 0, pad + 2 * (col + 8), main_y + 392, col, 28);
    put(notes, 0, pad, main_y + 433, left_width - 2 * pad, 28);
    form.Label("help", 0, pad, main_y + 467, left_width - 2 * pad, 16, "", 9);

    const int third = (right_width - 28) / 3;
    form.Label("bevel.title", 1, right_x + 14, main_y + 9, third - 6, 15, "BEVEL FROM SQUARE", 9, "#e6b972");
    form.Label("stock.title", 1, right_x + 14 + third, main_y + 9, third - 6, 15, "ROUGH STOCK THICKNESS", 9, "#e6b972");
    form.Label("length.title", 1, right_x + 14 + 2 * third, main_y + 9, third - 6, 15, "CUT LENGTH / PIECE", 9, "#e6b972");
    form.Label("bevel", 1, right_x + 14, main_y + 29, third - 6, 29, "", 24, "#e6b972", true);
    form.Label("stock", 1, right_x + 14 + third, main_y + 29, third - 6, 29, "", 19, "#ecede8", true);
    form.Label("length", 1, right_x + 14 + 2 * third, main_y + 29, third - 6, 29, "", 19, "#ecede8", true);
    form.Label("error", 1, right_x + 14, main_y + 28, right_width - 28, 32, "", 11, "#f09987", false, false, true);

    form.Label("preview.title", 2, right_x + 14, preview_y + 11, right_width - 202, 21, "How the segments fit", 14, "#ecede8", false, true);
    put(inspect, 2, right_x + right_width - 181, preview_y + 9, 167, 25);
    form.Label("inside", 2, right_x + 14, preview_y + 36, right_width - 28, 17, "", 10, "#80d6bb");
    put(preview, 2, right_x + 10, preview_y + 56, right_width - 20, preview_h - 108);
    form.Label("legend", 2, right_x + 14, preview_y + preview_h - 47, right_width - 28, 17, "Gold: finished outside   Green: bore   Coral: strips", 10);
    form.Label("warning", 2, right_x + 14, preview_y + preview_h - 26, right_width - 28, 24, "", 10, "#9ca8aa", false, false, true);

    form.Label("list.title", 3, right_x + 14, list_y + 10, right_width - 28, 21, "Cut list & centre references", 13, "#ecede8", false, true);
    put(cut_table, 3, right_x + 12, list_y + 36, right_width - 24, 136);
    cut_table.SetColumnWidth(0, DPI(122)); cut_table.SetColumnWidth(1, DPI(36));
    for(int c = 2; c < 6; ++c) cut_table.SetColumnWidth(c, DPI((right_width - 196) / 4));
    form.Label("centres", 3, right_x + 14, list_y + 178, right_width - 28, 18, "", 10);
    form.Label("totals", 3, right_x + 14, list_y + 198, right_width - 28, 18, "", 10, "#ecede8");
    form.Label("offset", 3, right_x + 14, list_y + 220, right_width - 28, 18, "", 9);

    form.Label("why.title", 5, pad, 584, left_width - 2 * pad, 21, "Why this works", 13, "#ecede8", false, true);
    form.Label("why.formula", 5, pad, 612, left_width - 2 * pad, 19, "Bevel = 180° / main staves", 11, "#e6b972");
    form.Label("why.inside", 5, pad, 637, left_width - 2 * pad, 19, "Inside R = outside R − radial wall", 11);
    form.Label("why.summary", 5, pad, 662, left_width - 2 * pad, 30, "Padding reserves wood for milling. The plate stays outside the stave cut length.", 10, "#9ca8aa", false, false, true);
    put(formula_more, 5, pad, 699, left_width - 2 * pad, 23);
    form.Label("why.details", 5, pad, 738, left_width - 2 * pad, 140,
        "α = 180° / N. Outer flat: A = R + outside pad. Inner flat: B = (R − wall − inside pad) × cos α. Stock thickness = A − B.\n\nStraight strips reduce adjoining stave widths, not the division angle. Cut length = depth + both trims + router end gap. Dry-fit before glue; the numbers cannot check your saw.",
        10, "#9ca8aa", false, false, true);
    form.Show("why.details", expanded_formula);
    UpdateReadouts();
}

void StaveCanvas::UpdateReadouts()
{
    const bool valid = geometry.IsValid() && !geometry.staves.IsEmpty();
    form.Text("outside.label", kind == "diameter" ? "Finished outside diameter" : "Finished outside radius");
    form.Text("unit", "Lengths in " + unit + "; wall and padding are radial.");
    form.Text("help", "Wheel: 0.1 · Alt: 1 · Shift: 10. Counts stay whole.");
    form.Text("status", status);
    form.Ink("status", !valid ? "#f09987" : dirty ? "#e6b972" : "#80d6bb");
    form.Text("bevel", valid ? Number(geometry.bevelDegrees) + "°" : String());
    form.Text("stock", valid ? M(geometry.stockThicknessMm) : String());
    form.Text("length", valid ? M(geometry.blankLengthMm) : String());
    form.Text("error", valid ? String() : geometry.error); form.Show("error", !valid);
    form.Text("inside", valid ? "Finished inside: Ø " + M(geometry.innerRadiusMm * 2) + " · R " + M(geometry.innerRadiusMm) : String("Last valid preview retained; correct the highlighted input."));
    form.Text("warning", geometry.warning.IsEmpty() ? String("Straight shell. End gap is sacrificial stock; not a machine path.") : geometry.warning);
    form.Ink("warning", geometry.warning.IsEmpty() ? "#9ca8aa" : "#e6b972");
    auto& model = cut_table.Model(); UiModelUpdate batch(model);
    const int rows = valid ? groups.GetCount() : 0;
    if(model.GetRowCount() != rows) model.SetRowCount(rows);
    const String headers[] = {"Part group", "Qty", "Outer " + unit, "Inner " + unit, "Stock " + unit, "Bevel"};
    for(int c = 0; c < 6; ++c)
        if(model.GetHeader(UITABLE_COLUMN_AXIS, c).text != headers[c]) model.SetHeader(UITABLE_COLUMN_AXIS, c, UiTableHeader(headers[c]));
    for(int r = 0; r < rows; ++r) {
        const auto& q = groups[r];
        String values[] = {q.name, AsString(q.count), Number(q.outer / Factor()), Number(q.inner / Factor()), Number(q.depth / Factor()), Number(q.bevel) + "°"};
        for(int c = 0; c < 6; ++c) SetReadoutCell(model, r, c, values[c]);
    }
    if(!valid) { for(const char* id : {"centres", "offset", "totals"}) form.Text(id, String()); return; }
    const auto& p = geometry.staves[min(selected, geometry.staves.GetCount() - 1)];
    form.Text("centres", "Centre → outer flat " + M(geometry.outerApothemMm) + " · inner flat " + M(geometry.innerApothemMm));
    form.Text("totals", "Stock lengths: main " + M(input.count * (geometry.blankLengthMm + input.endKerfMm)) + " · strips " + M(geometry.spacers.GetCount() * (geometry.blankLengthMm + input.endKerfMm)));
    form.Text("offset", Format("Stave %d: midpoint offset ", p.index) + M(p.centreOffsetMm) + ". Totals include end kerf; allow extra for ripping/squaring.");
}
void StaveCanvas::Paint(Draw& w) { w.DrawRect(GetSize(), Bg()); }
StavePage::StavePage(bool load_disk) : canvas(load_disk)
{
    StyleScroll(scroll); scroll.SetScrollMode(UIPANELSCROLL_VERTICAL);
    Add(scroll.SizePos()); scroll.Content().Add(canvas);
    canvas.WhenExtent = [this] { Layout(); };
}
void StavePage::Layout()
{
    const int width = max(850, SLogical(GetSize().cx) - 18);
    SPlace(canvas, 0, 0, width, canvas.ContentHeight()); scroll.Layout();
}
void StavePage::ApplyTheme() { StyleScroll(scroll); canvas.ApplyTheme(); }
} }
