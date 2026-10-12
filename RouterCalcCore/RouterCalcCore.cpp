#include "RouterCalcCore.h"
#include "Staves.h"
#include <cmath>
#include <cstdio>
#ifdef PLATFORM_WIN32
#include <windows.h>
#include <string>
#endif

namespace Upp { namespace RouterCalc {
Geometry Calculate(const Inputs& x)
{
    Geometry g;
    const double a[] = {x.holeRadiusMm, x.guideDiameterMm, x.cutterDiameterMm};
    for(double n : a) {
        if(!std::isfinite(n) || IsNull(n)) { g.error = "Enter a finite number in all three measurement fields."; return g; }
        if(n <= 0) { g.error = "All measurements must be greater than zero."; return g; }
        if(n > 10000) { g.error = "Input radii and diameters must not exceed 10,000 mm. Check the units."; return g; }
    }
    if(x.guideDiameterMm <= x.cutterDiameterMm) { g.error = "The circular guide must be larger than the cutter."; return g; }
    if(x.holeRadiusMm < x.cutterDiameterMm / 2) { g.error = "The finished hole cannot be smaller than the cutter."; return g; }
    g.holeR = x.holeRadiusMm;
    g.baseR = x.guideDiameterMm / 2;
    g.bitR = x.cutterDiameterMm / 2;
    g.pathR = g.holeR - g.bitR;
    g.templateR = g.pathR + g.baseR;
    return g;
}
String Number(double v)
{
    if(!std::isfinite(v) || IsNull(v)) return "\xE2\x80\x94";
    if(std::abs(v) < 1e-9) v = 0;
    String s = Format("%.5f", v);
    while(s.EndsWith("0")) s.Trim(s.GetCount() - 1);
    if(s.EndsWith(".")) s.Trim(s.GetCount() - 1);
    return s;
}
String Timestamp()
{
    Time t = GetSysTime(); // ISO local time, deliberately no false UTC suffix.
    return Format("%04d-%02d-%02d", t.year, t.month, t.day) + "T" +
           Format("%02d:%02d:%02d", t.hour, t.minute, t.second);
}
Value NewLibrary()
{
    ValueMap s;
    s.Add("unit", "mm"); s.Add("kind", "radius");
    s.Add("defaultGuideDiameterMm", 162.0); s.Add("defaultCutterDiameterMm", 8.0);
    s.Add("showBase", true); s.Add("showPath", true); s.Add("showRadii", true);
    ValueMap v;
    v.Add("application", "upp-router-calculator"); v.Add("schemaVersion", 1);
    v.Add("lengthUnit", "mm"); v.Add("settings", s); v.Add("designs", ValueArray());
    return v;
}
Value NewDesign(const Inputs& x, const String& name, const String& unit,
                const String& kind, const String& notes)
{
    ValueMap display; display.Add("unit", unit); display.Add("kind", kind);
    ValueMap d;
    d.Add("id", AsString(Uuid::Create())); d.Add("name", name);
    d.Add("calculator", "router-circle"); d.Add("holeRadiusMm", x.holeRadiusMm);
    d.Add("guideDiameterMm", x.guideDiameterMm); d.Add("cutterDiameterMm", x.cutterDiameterMm);
    d.Add("display", display); d.Add("notes", notes);
    String now = Timestamp(); d.Add("createdAt", now); d.Add("updatedAt", now);
    return d;
}
Inputs DesignInputs(const Value& d)
{
    Inputs x;
    x.holeRadiusMm = (double)d["holeRadiusMm"];
    x.guideDiameterMm = (double)d["guideDiameterMm"];
    x.cutterDiameterMm = (double)d["cutterDiameterMm"];
    return x;
}
static bool Text(const Value& v, int maxlen, bool empty = false)
{
    return v.Is<String>() && ((String)v).GetCount() <= maxlen && (empty || !TrimBoth((String)v).IsEmpty());
}
static bool Length(const Value& v)
{
    return IsNumber(v) && !v.Is<bool>() && !IsNull(v) && std::isfinite((double)v) && (double)v > 0 && (double)v <= 10000;
}
static bool Choice(const Value& v, const char* a, const char* b)
{
    return v.Is<String>() && (v == a || v == b);
}
static bool DateText(const Value& v)
{
    if(!Text(v, 40)) return false;
    String s = v;
    if(s.GetCount() < 19 || s[4] != '-' || s[7] != '-' || s[10] != 'T' || s[13] != ':' || s[16] != ':') return false;
    for(int i : {0,1,2,3,5,6,8,9,11,12,14,15,17,18}) if(s[i] < '0' || s[i] > '9') return false;
    Date d(StrInt(s.Left(4)), StrInt(s.Mid(5,2)), StrInt(s.Mid(8,2)));
    return d.IsValid() && StrInt(s.Mid(11,2)) < 24 && StrInt(s.Mid(14,2)) < 60 && StrInt(s.Mid(17,2)) < 60;
}
bool ValidateLibrary(const Value& v, String& error)
{
    error.Clear();
    auto fail = [&](const char* message) { error = message; return false; };
    if(!v.Is<ValueMap>() || v["application"] != "upp-router-calculator" ||
       v["schemaVersion"] != 1 || v["lengthUnit"] != "mm") return fail("Unsupported library: expected UPP Router Calculator schema 1 in millimetres.");
    Value s = v["settings"];
    if(!s.Is<ValueMap>() || !Choice(s["unit"], "mm", "cm") || !Choice(s["kind"], "radius", "diameter") ||
       !Length(s["defaultGuideDiameterMm"]) || !Length(s["defaultCutterDiameterMm"]) ||
       (double)s["defaultGuideDiameterMm"] <= (double)s["defaultCutterDiameterMm"])
        return fail("Invalid default units, input type or cutter/guide dimensions.");
    for(const char* key : {"showBase", "showPath", "showRadii"}) if(!s[key].Is<bool>()) return fail("Invalid diagram visibility setting.");
    if(!v["designs"].Is<ValueArray>()) return fail("The library is missing its designs array.");
    ValueArray designs = v["designs"];
    if(designs.GetCount() > MAX_DESIGNS) return fail("A library may contain at most 1,000 designs.");
    Index<String> ids;
    for(int i = 0; i < designs.GetCount(); ++i) {
        Value d = designs[i];
        if(!d.Is<ValueMap>() || d["calculator"] != "router-circle" || !Text(d["id"], 100) || !Text(d["name"], 100))
            return fail("Invalid design identity, name or calculator.");
        String id = d["id"];
        if(ids.Find(id) >= 0) return fail("Duplicate design IDs are not allowed.");
        ids.Add(id);
        if(!Length(d["holeRadiusMm"]) || !Length(d["guideDiameterMm"]) || !Length(d["cutterDiameterMm"]))
            return fail("A saved design contains an invalid measurement.");
        Geometry g = Calculate(DesignInputs(d));
        if(!g.IsValid()) { error = Format("Design %d: ", i + 1) + g.error; return false; }
        Value display = d["display"];
        if(!display.Is<ValueMap>() || !Choice(display["unit"], "mm", "cm") || !Choice(display["kind"], "radius", "diameter"))
            return fail("Invalid saved display preferences.");
        if(!Text(d["notes"], 2000, true) || !DateText(d["createdAt"]) || !DateText(d["updatedAt"]))
            return fail("Invalid design notes or ISO timestamps.");
    }
    return true;
}
bool ParseLibrary(const String& text, Value& library, String& error)
{
    if(text.GetCount() > MAX_LIBRARY_BYTES) { error = "Library files are limited to 2 MiB."; return false; }
    try {
        Value candidate = ParseJSON(text);
        if(!ValidateLibrary(candidate, error)) return false;
        library = candidate;
        return true;
    }
    catch(...) { error = "The file does not contain valid library JSON."; return false; }
}
#ifdef PLATFORM_WIN32
static std::wstring WidePath(const String& s)
{
    int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ~s, s.GetCount(), nullptr, 0);
    std::wstring w(n, L'\0');
    if(n) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ~s, s.GetCount(), &w[0], n);
    return w;
}
#endif
bool WriteLibrary(const String& path, const Value& library, String& expected_text,
                  bool& expected_exists, String& error)
{
    if(!ValidateLibrary(library, error)) return false;
    return WriteJsonFile(path, library, expected_text, expected_exists, error);
}
bool WriteJsonFile(const String& path,const Value& value,String& expected_text,bool& expected_exists,String& error)
{
    return WriteTextFile(path, AsJSON(value, true), expected_text, expected_exists, error);
}
bool WriteTextFile(const String& path,const String& text,String& expected_text,bool& expected_exists,String& error)
{
    if(text.GetCount() > MAX_LIBRARY_BYTES) { error = "This library exceeds the 2 MiB limit."; return false; }
    if(FileExists(path) != expected_exists || (expected_exists && LoadFile(path) != expected_text)) {
        error = "The library changed on disk. Reopen it, or export to a different file; nothing was overwritten."; return false;
    }
    String temporary = path + "." + AsString(Uuid::Create()) + ".tmp";
    if(!SaveFile(temporary, text)) { error = "Cannot write beside this file. Choose a writable location with Export."; return false; }
    if(expected_exists && !SaveFile(path + ".bak", expected_text)) {
        FileDelete(temporary); error = "Cannot preserve the previous library backup. Save was cancelled."; return false;
    }
#ifdef PLATFORM_WIN32
    bool ok = MoveFileExW(WidePath(temporary).c_str(), WidePath(path).c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    bool ok = std::rename(~temporary, ~path) == 0;
#endif
    if(!ok) { FileDelete(temporary); error = "Cannot replace the library file. The previous file and backup were preserved."; return false; }
    expected_text = text; expected_exists = true; error.Clear(); return true;
}
} }
