#ifndef _RouterCalcCore_RouterCalcCore_h_
#define _RouterCalcCore_RouterCalcCore_h_
#include <Core/Core.h>

namespace Upp { namespace RouterCalc {
// All canonical lengths are millimetres. No GUI dependencies or hidden rounding.
struct Inputs {
    double holeRadiusMm = 37.5;
    double guideDiameterMm = 162;
    double cutterDiameterMm = 8;
};
struct Geometry {
    double holeR = 0, baseR = 0, bitR = 0, pathR = 0, templateR = 0;
    String error;
    bool IsValid() const { return error.IsEmpty(); }
};
Geometry Calculate(const Inputs& input);
String Number(double value);
String Timestamp();
Value NewLibrary();
Value NewDesign(const Inputs& input, const String& name, const String& unit,
                const String& kind, const String& notes = String());
Inputs DesignInputs(const Value& design);
bool ValidateLibrary(const Value& library, String& error);
bool ParseLibrary(const String& text, Value& library, String& error);
// Validate and stage a complete file, retain .bak, then atomically replace it.
// Refuse to overwrite a library changed externally since its last load/save.
bool WriteLibrary(const String& path, const Value& library, String& expected_text,
                  bool& expected_exists, String& error);
constexpr int MAX_LIBRARY_BYTES = 2 * 1024 * 1024;
constexpr int MAX_DESIGNS = 1000;
} }
#endif
