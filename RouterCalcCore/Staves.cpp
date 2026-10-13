#include "Staves.h"
#include <cmath>
namespace Upp { namespace RouterCalc {
static constexpr double PI_STAVE=3.1415926535897932384626433832795;
static Pointf RotateStave(double r,double t,double a){return Pointf(r*std::cos(a)-t*std::sin(a),r*std::sin(a)+t*std::cos(a));}
static double RadiusOf(Pointf p){return std::hypot(p.x,p.y);}
StaveGeometry CalculateStaves(const StaveInput& x)
{
    StaveGeometry g;
    for(double v:{x.outerRadiusMm,x.wallMm,x.depthMm,x.outsideAllowanceMm,x.insideAllowanceMm,x.spacerThicknessMm,x.topTrimMm,x.bottomTrimMm,x.basePlateMm,x.reachGapMm,x.endKerfMm})
        if(IsNull(v)||!std::isfinite(v)||v<0){g.error="Lengths and allowances must be finite and non-negative.";return g;}
    if(x.outerRadiusMm<=0){g.error="Finished outside size must be greater than zero.";return g;}
    if(x.depthMm<=0){g.error="Finished depth must be greater than zero.";return g;}
    if(x.wallMm<=0){g.error="Wall thickness must be greater than zero.";return g;}
    if(x.wallMm>=x.outerRadiusMm){g.error="Wall thickness leaves no bore. It must be less than outside R "+Number(x.outerRadiusMm)+" mm.";return g;}
    if(x.count<3||x.count>512){g.error="Use 3 to 512 main staves. Spacer strips are not additional angular divisions.";return g;}
    g.innerRadiusMm=x.outerRadiusMm-x.wallMm;
    double inner_limit=g.innerRadiusMm-x.insideAllowanceMm;
    if(inner_limit<=0){g.error="Inside allowance leaves no central opening.";return g;}
    const double alpha=PI_STAVE/x.count,c=std::cos(alpha),t=std::tan(alpha);
    g.divisionDegrees=360.0/x.count;g.bevelDegrees=180.0/x.count;
    g.outerApothemMm=x.outerRadiusMm+x.outsideAllowanceMm;
    g.innerApothemMm=inner_limit*c;
    g.stockThicknessMm=g.outerApothemMm-g.innerApothemMm;
    g.finishedBottomZMm=x.reachGapMm+x.bottomTrimMm;
    g.finishedTopZMm=g.finishedBottomZMm+x.depthMm;
    g.blankLengthMm=g.finishedTopZMm+x.topTrimMm;
    g.assemblyHeightMm=g.blankLengthMm+x.basePlateMm;
    if(g.innerRadiusMm>=x.outerRadiusMm||!std::isfinite(g.assemblyHeightMm)||!std::isfinite(g.outerApothemMm*2)||!std::isfinite(x.count*(g.blankLengthMm+x.endKerfMm))){g.error="These dimensions exceed the available numeric precision or range.";return g;}
    Vector<int> joints;joints.SetCount(x.count,0);
    if(x.spacerThicknessMm>0){
        if(TrimBoth(x.selectedJoints).IsEmpty()&&(x.spacerEvery<1||x.spacerEvery>x.count)){g.error="Spacer cadence must be from 1 to the main stave count.";return g;}
        if(!TrimBoth(x.selectedJoints).IsEmpty()){
            try{CParser p(x.selectedJoints);for(;;){int j=p.ReadInt();if(j<1||j>x.count||joints[j-1]){g.error="Selected joints must be unique numbers from 1 to the stave count.";return g;}joints[j-1]=1;if(p.IsEof())break;p.PassChar(',');if(p.IsEof()){g.error="Finish the selected-joint list without a trailing comma.";return g;}}}
            catch(CParser::Error){g.error="Use a comma-separated joint list, for example 3,6,9.";return g;}
        }else{
            for(int j=x.spacerEvery-1;j<x.count;j+=x.spacerEvery)joints[j]=1;
            if(x.count%x.spacerEvery)g.warning="The cadence does not divide the stave count; the wrap-around interval is different. The geometry still closes.";
        }
    }
    for(int i=0;i<x.count;i++){
        double left=joints[(i+x.count-1)%x.count]?x.spacerThicknessMm:0;
        double right=joints[i]?x.spacerThicknessMm:0;
        double deduction=(left+right)/(2*c);
        double outer=2*g.outerApothemMm*t-deduction,inner=2*g.innerApothemMm*t-deduction;
        if(inner<=0||!std::isfinite(outer)||!std::isfinite(inner)){g.error="Spacer strips consume a stave's inner face. Use thinner/fewer strips or fewer staves.";g.staves.Clear();g.spacers.Clear();return g;}
        StavePiece& p=g.staves.Add();p.index=i+1;p.leftSpacerMm=left;p.rightSpacerMm=right;
        p.outerWidthMm=outer;p.innerWidthMm=inner;p.centreOffsetMm=(left-right)/(4*c);
        double a=2*alpha*i,A=g.outerApothemMm,B=g.innerApothemMm;
        p.vertices[0]=RotateStave(A,-A*t+left/(2*c),a);
        p.vertices[1]=RotateStave(A,A*t-right/(2*c),a);
        p.vertices[2]=RotateStave(B,B*t-right/(2*c),a);
        p.vertices[3]=RotateStave(B,-B*t+left/(2*c),a);
        for(Pointf v:p.vertices)g.maximumBlankRadiusMm=max(g.maximumBlankRadiusMm,RadiusOf(v));
        if(joints[i]){
            SpacerPiece& q=g.spacers.Add();q.afterStave=i+1;
            double w=x.spacerThicknessMm,angle=a+alpha;
            q.radialInnerMm=B/c-w*t/2;
            // A wide rectangular strip may stand proud of the adjoining outer flats.
            // Never shorten it below the circle that must survive the outside allowance.
            q.radialOuterMm=max(A,A/c-w*t/2);
            q.vertices[0]=RotateStave(q.radialOuterMm,-w/2,angle);
            q.vertices[1]=RotateStave(q.radialOuterMm,w/2,angle);
            q.vertices[2]=RotateStave(q.radialInnerMm,w/2,angle);
            q.vertices[3]=RotateStave(q.radialInnerMm,-w/2,angle);
            for(Pointf v:q.vertices)g.maximumBlankRadiusMm=max(g.maximumBlankRadiusMm,RadiusOf(v));
        }
    }
    return g;
}
Value StaveParameters(const StaveInput& x)
{
    ValueMap v;
    v.Add("outerRadiusMm",x.outerRadiusMm);v.Add("wallMm",x.wallMm);v.Add("outsideAllowanceMm",x.outsideAllowanceMm);v.Add("insideAllowanceMm",x.insideAllowanceMm);
    v.Add("depthMm",x.depthMm);v.Add("topTrimMm",x.topTrimMm);v.Add("bottomTrimMm",x.bottomTrimMm);v.Add("basePlateMm",x.basePlateMm);v.Add("reachGapMm",x.reachGapMm);v.Add("endKerfMm",x.endKerfMm);
    v.Add("spacerThicknessMm",x.spacerThicknessMm);v.Add("count",x.count);v.Add("spacerEvery",x.spacerEvery);v.Add("selectedJoints",x.selectedJoints);return v;
}
bool DecodeStaveParameters(const Value& v,StaveInput& result,String& error)
{
    if(!v.Is<ValueMap>()){error="Missing stave parameters.";return false;}
    StaveInput x;double* fields[]={&x.outerRadiusMm,&x.wallMm,&x.outsideAllowanceMm,&x.insideAllowanceMm,&x.depthMm,&x.topTrimMm,&x.bottomTrimMm,&x.basePlateMm,&x.reachGapMm,&x.endKerfMm,&x.spacerThicknessMm};
    const char* keys[]={"outerRadiusMm","wallMm","outsideAllowanceMm","insideAllowanceMm","depthMm","topTrimMm","bottomTrimMm","basePlateMm","reachGapMm","endKerfMm","spacerThicknessMm"};
    for(int i=0;i<11;i++){Value a=v[keys[i]];if(!IsNumber(a)||a.Is<bool>()||IsNull(a)){error="A stave dimension is not a number.";return false;}*fields[i]=(double)a;}
    for(const char* key:{"count","spacerEvery"}){Value a=v[key];if(!IsNumber(a)||a.Is<bool>()||IsNull(a)||!std::isfinite((double)a)||(double)a!=std::floor((double)a)||(double)a<1||(double)a>512){error="Counts must be whole numbers from 1 to 512.";return false;}}
    x.count=(int)v["count"];x.spacerEvery=(int)v["spacerEvery"];
    if(!v["selectedJoints"].Is<String>()||((String)v["selectedJoints"]).GetCount()>2048){error="Invalid selected-joint list.";return false;}x.selectedJoints=(String)v["selectedJoints"];
    StaveGeometry g=CalculateStaves(x);if(!g.IsValid()){error=g.error;return false;}result=x;error.Clear();return true;
}
Value NewStaveLibrary(){ValueMap v;v.Add("application","upp-router-calculator-staves");v.Add("schemaVersion",1);v.Add("lengthUnit","mm");v.Add("designs",ValueArray());return v;}
bool ValidateStaveLibrary(const Value& v,String& error)
{
    auto fail=[&](const char* s){error=s;return false;};
    if(!v.Is<ValueMap>()||v["application"]!="upp-router-calculator-staves"||v["schemaVersion"]!=1||v["lengthUnit"]!="mm"||!v["designs"].Is<ValueArray>())return fail("Unsupported stave library; expected schema 1 in millimetres.");
    ValueArray ds=v["designs"];if(ds.GetCount()>MAX_DESIGNS)return fail("Too many saved designs.");Index<String> ids;
    for(Value d:ds){
        if(!d.Is<ValueMap>()||!d["id"].Is<String>()||!d["name"].Is<String>()||TrimBoth((String)d["name"]).IsEmpty()||((String)d["name"]).GetCount()>100||((String)d["id"]).IsEmpty()||((String)d["id"]).GetCount()>100)return fail("Invalid design identity or name.");
        if(!IsNull(d["notes"])&&(!d["notes"].Is<String>()||((String)d["notes"]).GetCount()>2000))return fail("Invalid shell notes; use at most 2,000 bytes of text.");
        String id=d["id"];if(ids.Find(id)>=0)return fail("Duplicate design ID.");ids.Add(id);StaveInput x;if(!DecodeStaveParameters(d["parameters"],x,error))return false;
        if((d["unit"]!="mm"&&d["unit"]!="cm")||(d["kind"]!="radius"&&d["kind"]!="diameter"))return fail("Invalid display preferences.");
    }
    error.Clear();return true;
}
} }
