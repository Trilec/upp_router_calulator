#ifndef _RouterCalcCore_Staves_h_
#define _RouterCalcCore_Staves_h_
#include "RouterCalcCore.h"
namespace Upp { namespace RouterCalc {
struct StaveInput {
    double outerRadiusMm=175, wallMm=10, outsideAllowanceMm=2, insideAllowanceMm=1;
    double depthMm=200, topTrimMm=2, bottomTrimMm=2, basePlateMm=18, reachGapMm=15, endKerfMm=3;
    double spacerThicknessMm=0;
    int count=20, spacerEvery=1;
    String selectedJoints; // optional comma-separated 1-based joints; overrides cadence
};
struct StavePiece : Moveable<StavePiece> {
    int index=0;
    double leftSpacerMm=0, rightSpacerMm=0, outerWidthMm=0, innerWidthMm=0, centreOffsetMm=0;
    Pointf vertices[4];
};
struct SpacerPiece : Moveable<SpacerPiece> {
    int afterStave=0;
    double radialInnerMm=0, radialOuterMm=0;
    Pointf vertices[4];
};
struct StaveGeometry {
    double innerRadiusMm=0, outerApothemMm=0, innerApothemMm=0, stockThicknessMm=0;
    double bevelDegrees=0, divisionDegrees=0, maximumBlankRadiusMm=0;
    double blankLengthMm=0, finishedBottomZMm=0, finishedTopZMm=0, assemblyHeightMm=0;
    String error, warning;
    Vector<StavePiece> staves;
    Vector<SpacerPiece> spacers;
    bool IsValid() const {return error.IsEmpty();}
};
StaveGeometry CalculateStaves(const StaveInput& input);
Value StaveParameters(const StaveInput& input);
bool DecodeStaveParameters(const Value& value, StaveInput& input, String& error);
Value StaveGeometryExport(const StaveInput& input, const StaveGeometry& geometry);
String StaveCutListCsv(const StaveInput& input, const StaveGeometry& geometry);
Value NewStaveLibrary();
bool ValidateStaveLibrary(const Value& value, String& error);
// Validated callers share the same conflict-checked, staged file writer.
bool WriteJsonFile(const String& path, const Value& value, String& expected_text, bool& expected_exists, String& error);
bool WriteTextFile(const String& path, const String& text, String& expected_text, bool& expected_exists, String& error);
} }
#endif
