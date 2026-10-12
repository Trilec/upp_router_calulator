#include "Staves.h"
namespace Upp { namespace RouterCalc {
static Value XYLoop(const Pointf* v)
{
    ValueArray loop;for(int i=0;i<4;i++){ValueArray p;p.Add(v[i].x);p.Add(v[i].y);loop.Add(p);}return loop;
}
Value StaveGeometryExport(const StaveInput& x,const StaveGeometry& g)
{
    if(!g.IsValid())return Value();
    ValueMap root;root.Add("application","upp-router-calculator-stave-geometry");root.Add("schemaVersion",1);root.Add("lengthUnit","mm");root.Add("angleUnit","degrees");root.Add("machineReady",false);
    root.Add("model","straight-cylindrical-shell-with-parallel-spacer-strips");root.Add("parameters",StaveParameters(x));
    root.Add("coordinateSystem","right-handed; Z is shell axis; +Z goes from mounting plate into blank; X points through stave 1 centre direction; joints numbered counterclockwise looking toward -Z");
    root.Add("origin","shell axis at mounting plate upper face; base plate lies at negative Z");
    root.Add("warning","Geometry and cut list only. NOT G-code or a safe robot trajectory. Tool, holder, arm, fixtures, reach, collision checks, feeds and workholding remain unmodelled. Do not execute as machine instructions.");
    ValueMap axial;axial.Add("blankBottomZMm",0);axial.Add("blankTopZMm",g.blankLengthMm);axial.Add("finishedBottomZMm",g.finishedBottomZMm);axial.Add("finishedTopZMm",g.finishedTopZMm);axial.Add("inaccessibleTopZMm",x.reachGapMm);axial.Add("plateBottomZMm",-x.basePlateMm);axial.Add("plateDiameterMm",Value());axial.Add("reachableAxialLengthMm",g.blankLengthMm-x.reachGapMm);axial.Add("extraBlankLengthMm",g.blankLengthMm-x.depthMm);root.Add("axial",axial);
    ValueMap radial;radial.Add("finishedOutsideRadiusMm",x.outerRadiusMm);radial.Add("finishedInsideRadiusMm",g.innerRadiusMm);radial.Add("outerFaceCentreDistanceMm",g.outerApothemMm);radial.Add("innerFaceCentreDistanceMm",g.innerApothemMm);radial.Add("maximumBlankRadiusMm",g.maximumBlankRadiusMm);radial.Add("mainStockThicknessMm",g.stockThicknessMm);root.Add("radial",radial);
    ValueArray pieces,order;
    for(const StavePiece& p:g.staves){
        String id=Format("S%03d",p.index);ValueMap v;v.Add("id",id);v.Add("type","main-stave");v.Add("position",p.index);v.Add("xyVerticesCCW",XYLoop(p.vertices));
        v.Add("z0Mm",0);v.Add("z1Mm",g.blankLengthMm);v.Add("outerWidthMm",p.outerWidthMm);v.Add("innerWidthMm",p.innerWidthMm);v.Add("stockThicknessMm",g.stockThicknessMm);v.Add("bevelFromSquareDegrees",g.bevelDegrees);v.Add("centreOffsetAlongLocalTangentMm",p.centreOffsetMm);v.Add("leftStripMm",p.leftSpacerMm);v.Add("rightStripMm",p.rightSpacerMm);pieces.Add(v);order.Add(id);
        for(const SpacerPiece& q:g.spacers)if(q.afterStave==p.index){
            id=Format("P%03d",q.afterStave);ValueMap strip;strip.Add("id",id);strip.Add("type","parallel-spacer");strip.Add("afterStave",q.afterStave);strip.Add("xyVerticesCCW",XYLoop(q.vertices));strip.Add("z0Mm",0);strip.Add("z1Mm",g.blankLengthMm);strip.Add("widthMm",x.spacerThicknessMm);strip.Add("radialDepthMm",q.radialOuterMm-q.radialInnerMm);strip.Add("bevelFromSquareDegrees",0);pieces.Add(strip);order.Add(id);
        }
    }
    root.Add("pieces",pieces);root.Add("assemblyOrderCCW",order);root.Add("endCutKerfMm",x.endKerfMm);
    root.Add("stockEstimateNote","Length estimate uses one separating end cut per piece, after the stock end is squared. It excludes ripping waste, initial squaring, defects and offcuts. Kerf does not reduce glued-piece dimensions.");
    root.Add("mainLengthWithEndKerfMm",g.staves.GetCount()*(g.blankLengthMm+x.endKerfMm));root.Add("stripLengthWithEndKerfMm",g.spacers.GetCount()*(g.blankLengthMm+x.endKerfMm));return root;
}
String StaveCutListCsv(const StaveInput& x,const StaveGeometry& g)
{
    if(!g.IsValid())return String();
    String out="part,type,position_or_after_stave,quantity,length_mm,outer_width_mm,inner_width_mm,stock_depth_mm,bevel_from_square_deg,left_strip_mm,right_strip_mm,tangent_centre_offset_mm,end_cut_kerf_mm\r\n";
    for(const StavePiece& p:g.staves){
        out<<Format("S%03d",p.index)<<",main-stave,"<<p.index<<",1,"<<Number(g.blankLengthMm)<<","<<Number(p.outerWidthMm)<<","<<Number(p.innerWidthMm)<<","<<Number(g.stockThicknessMm)<<","<<Number(g.bevelDegrees)<<","<<Number(p.leftSpacerMm)<<","<<Number(p.rightSpacerMm)<<","<<Number(p.centreOffsetMm)<<","<<Number(x.endKerfMm)<<"\r\n";
        for(const SpacerPiece& q:g.spacers)if(q.afterStave==p.index)out<<Format("P%03d",q.afterStave)<<",parallel-spacer,"<<q.afterStave<<",1,"<<Number(g.blankLengthMm)<<","<<Number(x.spacerThicknessMm)<<","<<Number(x.spacerThicknessMm)<<","<<Number(q.radialOuterMm-q.radialInnerMm)<<",0,0,0,0,"<<Number(x.endKerfMm)<<"\r\n";
    }
    return out;
}
} }
