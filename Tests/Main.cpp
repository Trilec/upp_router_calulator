#include <RouterCalcCore/RouterCalcCore.h>
#include <cmath>
#include <limits>
using namespace Upp;
using namespace Upp::RouterCalc;
int RunStaveTests();
CONSOLE_APP_MAIN
{
    int passed = 0, failed = 0;
    auto test = [&](bool ok, const String& label) { if(ok) ++passed; else { ++failed; Cout() << "FAIL: " << label << '\n'; } };
    auto NearlyEqual = [](double a, double b) { return std::abs(a - b) < 1e-8; };
    Inputs x; Geometry g = Calculate(x);
    test(g.IsValid() && NearlyEqual(g.templateR, 114.5) && NearlyEqual(g.templateR * 2, 229), "75 mm hole / 162 mm guide / 8 mm cutter");
    x.holeRadiusMm = 32; test(NearlyEqual(Calculate(x).templateR, 109), "R32 with 162 guide");
    x.guideDiameterMm = 161; test(NearlyEqual(Calculate(x).templateR, 108.5), "R32 with 161 guide");
    x.holeRadiusMm = 4; g = Calculate(x); test(g.IsValid() && g.pathR == 0 && g.templateR == 80.5, "zero travel boundary");
    x.holeRadiusMm = 3.999; test(!Calculate(x).IsValid(), "hole smaller than cutter");
    x = Inputs(); x.guideDiameterMm = 8; test(!Calculate(x).IsValid(), "guide equal to cutter");
    for(double n : {0.0, -1.0, std::numeric_limits<double>::lowest(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        x = Inputs(); x.holeRadiusMm = n; test(!Calculate(x).IsValid(), "invalid hole input");
        x = Inputs(); x.guideDiameterMm = n; test(!Calculate(x).IsValid(), "invalid guide input");
        x = Inputs(); x.cutterDiameterMm = n; test(!Calculate(x).IsValid(), "invalid cutter input");
    }
    for(int i = 1; i <= 100; ++i) {
        x = Inputs(); x.holeRadiusMm = 4 + i * .37; x.guideDiameterMm = 50 + i * .51;
        g = Calculate(x);
        test(g.IsValid() && NearlyEqual(g.pathR + g.baseR, g.templateR) && NearlyEqual(g.pathR + g.bitR, g.holeR), "tangency identity");
        test(NearlyEqual((g.holeR * 2 + x.guideDiameterMm - x.cutterDiameterMm) / 2, g.templateR), "radius/diameter equivalence");
        double cm = x.holeRadiusMm / 10; test(NearlyEqual(cm * 10, x.holeRadiusMm), "unit round trip");
    }
    test(Number(114.5) == "114.5" && Number(0) == "0" && Number(8) == "8", "human number formatting");
    String error;
    Value library = NewLibrary(); test(ValidateLibrary(library, error), "empty schema-1 library");
    ValueArray ds; ds.Add(NewDesign(Inputs(), "Rim / \xC3\x98 75", "mm", "radius", "8 mm cutting edges"));
    ValueMap m(library); m.Set("designs", ds); library = m;
    if(!ValidateLibrary(library, error)) Cout() << "Library validation: " << error << "\n" << AsJSON(library, true) << "\n";

    Value decoded;
    test(ParseLibrary(AsJSON(library, true), decoded, error) && decoded == library, "JSON round trip");
    if(!error.IsEmpty()) Cout() << "Parse error: " << error << "\n";

    Value sentinel = NewLibrary(); Value original = sentinel;
    test(!ParseLibrary("{ broken json", sentinel, error) && sentinel == original, "corrupt import preserves live data");
    m = ValueMap(library); m.Set("schemaVersion", 999); test(!ValidateLibrary(m, error), "unknown schema rejected");
    m = ValueMap(library); ds.Add(ds[0]); m.Set("designs", ds); test(!ValidateLibrary(m, error), "duplicate IDs rejected");
    m = ValueMap(library); ValueMap d(ds[0]); d.Set("cutterDiameterMm", "8"); ValueArray bad; bad.Add(d); m.Set("designs", bad);
    test(!ValidateLibrary(m, error), "numeric strings rejected");
    test(!ParseLibrary(String('x', MAX_LIBRARY_BYTES + 1), sentinel, error), "oversize import rejected");
    String path = AppendFileName(GetExeFolder(), "router-test-" + AsString(Uuid::Create()) + ".json");
    String expected; bool exists = false;
    test(WriteLibrary(path, library, expected, exists, error), "new adjacent library write");
    test(ParseLibrary(LoadFile(path), decoded, error) && decoded == library, "disk reopen round trip");
    test(WriteLibrary(path, library, expected, exists, error) && LoadFile(path + ".bak") == expected, "save and backup");
    SaveFile(path, "external change");
    test(!WriteLibrary(path, library, expected, exists, error) && LoadFile(path) == "external change", "external edit protection");
    FileDelete(path); FileDelete(path + ".bak");
    Inputs large;large.holeRadiusMm=10001;test(Calculate(large).IsValid(),"no arbitrary ten-metre circle limit");large.holeRadiusMm=std::numeric_limits<double>::max();test(!Calculate(large).IsValid(),"true diameter overflow rejected");
    Cout() << "RouterCalcCore: " << passed << " passed, " << failed << " failed\n";
    failed += RunStaveTests();
    SetExitCode(failed ? 1 : 0);
}
