#include <RouterCalcCore/Staves.h>
#include <cmath>
#include <limits>
#include <chrono>
using namespace Upp;
using namespace Upp::RouterCalc;
namespace {
constexpr double PI=3.14159265358979323846;
bool Near(double a,double b){return std::abs(a-b)<=1e-7;}
double TestDistance(Pointf a,Pointf b){return std::hypot(a.x-b.x,a.y-b.y);}
bool Contains(const Pointf* v,Pointf p)
{
    for(int i=0;i<4;i++){
        Pointf a=v[i],b=v[(i+1)%4];
        if((b.x-a.x)*(p.y-a.y)-(b.y-a.y)*(p.x-a.x)<-1e-6)return false;
    }
    return true;
}
// JSON uses decimal text; validate dimensions to a much finer precision than
// the cut list rather than requiring the identical binary floating-point bits.
bool JsonEquivalent(const Value& a,const Value& b,double& maximum_error)
{
    if(a.Is<ValueMap>()&&b.Is<ValueMap>()){
        ValueMap x(a),y(b);if(x.GetCount()!=y.GetCount())return false;
        for(int i=0;i<x.GetCount();i++)if(x.GetKey(i)!=y.GetKey(i)||!JsonEquivalent(x.GetValue(i),y.GetValue(i),maximum_error))return false;
        return true;
    }
    if(a.Is<ValueArray>()&&b.Is<ValueArray>()){
        ValueArray x(a),y(b);if(x.GetCount()!=y.GetCount())return false;
        for(int i=0;i<x.GetCount();i++)if(!JsonEquivalent(x[i],y[i],maximum_error))return false;
        return true;
    }
    if(IsNumber(a)&&IsNumber(b)&&!a.Is<bool>()&&!b.Is<bool>()&&!IsNull(a)&&!IsNull(b)){
        double error=std::abs((double)a-(double)b);maximum_error=max(maximum_error,error);return std::isfinite(error)&&error<=1e-9;
    }
    return a==b;
}
bool InStock(const StaveGeometry& g,Pointf p)
{
    for(const auto& s:g.staves)if(Contains(s.vertices,p))return true;
    for(const auto& s:g.spacers)if(Contains(s.vertices,p))return true;
    return false;
}
}
int RunStaveTests()
{
    int passed=0,failed=0;
    auto check=[&](bool ok,const String& label){if(ok)++passed;else{++failed;Cout()<<"FAIL: "<<label<<"\n";}};
    StaveInput x;StaveGeometry g=CalculateStaves(x);
    check(g.IsValid()&&g.staves.GetCount()==20&&Near(g.bevelDegrees,9),"default staves");
    check(Near(g.divisionDegrees,18)&&Near(g.innerRadiusMm,165),"division and finished bore");
    check(Near(g.outerApothemMm,177)&&Near(g.innerApothemMm,164*std::cos(PI/20)),"circumscribed outer / inscribed inner allowance");
    check(Near(g.blankLengthMm,219)&&Near(g.finishedBottomZMm,17)&&Near(g.finishedTopZMm,217)&&Near(g.assemblyHeightMm,237),"axial allowances");
    // Independent polygon containment check: the entire requested annulus, plus
    // allowances, must be present in the glue-up. This is not merely reusing formulas.
    for(int n:{3,6,12,18,20,48})for(double radius:{50.0,175.0,500.0})for(double strip:{0.0,1.0,2.0})for(int every:{1,3}){
        x=StaveInput();x.count=n;x.outerRadiusMm=radius;x.spacerThicknessMm=strip;x.spacerEvery=every;
        g=CalculateStaves(x);String label=Format("N=%d R=%g strip=%g every=%d",n,radius,strip,every);
        check(g.IsValid(),"valid geometry "+label);if(!g.IsValid())continue;
        bool covered=true,closed=true,positive=true;
        for(int sample=0;sample<360;sample++){
            double a=2*PI*(sample+.137)/360;
            for(double r:{g.innerRadiusMm-x.insideAllowanceMm,(g.innerRadiusMm+x.outerRadiusMm)/2,x.outerRadiusMm+x.outsideAllowanceMm})
                covered=covered&&InStock(g,Pointf(r*std::cos(a),r*std::sin(a)));
        }
        for(int i=0;i<n;i++){
            const auto& p=g.staves[i];const auto& q=g.staves[(i+1)%n];
            positive=positive&&p.innerWidthMm>0&&p.outerWidthMm>p.innerWidthMm;
            if(p.rightSpacerMm==0)closed=closed&&TestDistance(p.vertices[1],q.vertices[0])<1e-7&&TestDistance(p.vertices[2],q.vertices[3])<1e-7;
        }
        for(const auto& q:g.spacers){
            const auto& p=g.staves[q.afterStave-1];const auto& next=g.staves[q.afterStave%n];
            closed=closed&&TestDistance(p.vertices[2],q.vertices[3])<1e-7&&TestDistance(next.vertices[3],q.vertices[2])<1e-7;
            closed=closed&&Near(TestDistance(q.vertices[0],q.vertices[1]),strip)&&Near(TestDistance(q.vertices[2],q.vertices[3]),strip);
        }
        check(covered,"milling annulus contained "+label);check(closed,"glue joints close "+label);check(positive,"positive cut faces "+label);
        check(Near(g.bevelDegrees*n,180),"bevel closure "+label);
    }
    x=StaveInput();x.count=20;x.spacerThicknessMm=2;x.spacerEvery=3;g=CalculateStaves(x);
    check(g.spacers.GetCount()==6&&!g.warning.IsEmpty(),"cadence remainder explained");
    x.selectedJoints="1,7,20";g=CalculateStaves(x);check(g.IsValid()&&g.spacers.GetCount()==3,"selected joint override");
    check(g.staves[0].leftSpacerMm==2&&g.staves[0].rightSpacerMm==2&&g.staves[6].rightSpacerMm==2,"asymmetric joint identities");
    check(g.staves[1].centreOffsetMm>0&&g.staves[19].centreOffsetMm<0,"one-sided strip offsets have correct sign");
    for(const char* bad:{"0","21","1,1","3,","one","1;3"}){x.selectedJoints=bad;check(!CalculateStaves(x).IsValid(),String("invalid joint list ")+bad);}
    x=StaveInput();x.spacerThicknessMm=1000;check(!CalculateStaves(x).IsValid(),"strip consumes inner face");
    x=StaveInput();x.insideAllowanceMm=165;check(!CalculateStaves(x).IsValid(),"inside allowance consumes bore");
    for(int n:{0,1,2,513}){x=StaveInput();x.count=n;check(!CalculateStaves(x).IsValid(),"stave count range");}
    for(double bad:{-1.0,10001.0,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}){
        x=StaveInput();x.depthMm=bad;check(!CalculateStaves(x).IsValid(),"invalid axial input");
        x=StaveInput();x.outerRadiusMm=bad;check(!CalculateStaves(x).IsValid(),"invalid radial input");
    }
    x=StaveInput();x.topTrimMm=x.bottomTrimMm=x.reachGapMm=x.basePlateMm=x.endKerfMm=0;g=CalculateStaves(x);
    check(g.IsValid()&&Near(g.blankLengthMm,x.depthMm)&&Near(g.finishedBottomZMm,0),"zero axial allowances");
    x=StaveInput();x.spacerThicknessMm=2;x.selectedJoints="2,8,20";g=CalculateStaves(x);
    Value exported=StaveGeometryExport(x,g);ValueArray pieces=exported["pieces"],order=exported["assemblyOrderCCW"];
    check(pieces.GetCount()==23&&order.GetCount()==23,"numbered geometry includes each strip");
    check(exported["machineReady"]==false&&exported["lengthUnit"]=="mm","export not machine instructions");
    check(Near((double)exported["axial"]["plateBottomZMm"],-18)&&Near((double)exported["axial"]["finishedBottomZMm"],17),"export axial origin");
    double json_error=0;
    check(JsonEquivalent(ParseJSON(AsJSON(exported,true)),exported,json_error),"geometry JSON structure and 1e-9 mm numeric round trip");
    Cout()<<Format("Geometry JSON maximum numeric round-trip error: %.12g mm\n",json_error);
    String csv=StaveCutListCsv(x,g);check(Split(csv,'\n').GetCount()==24&&csv.Find("P020,parallel-spacer")>=0,"CSV includes headers and all parts");
    String error;StaveInput decoded;
    check(DecodeStaveParameters(StaveParameters(x),decoded,error)&&StaveParameters(x)==StaveParameters(decoded),"parameter round trip");
    ValueMap bad_parameters(StaveParameters(x));bad_parameters.Set("count",20.5);check(!DecodeStaveParameters(bad_parameters,decoded,error),"JSON fractional count rejected");
    bad_parameters=ValueMap(StaveParameters(x));bad_parameters.Set("wallMm","10");check(!DecodeStaveParameters(bad_parameters,decoded,error),"JSON numeric string rejected");
    ValueMap library(NewStaveLibrary()),design;ValueArray designs;
    design.Add("id","fixture-01");design.Add("name","A round ambition");design.Add("parameters",StaveParameters(x));design.Add("unit","mm");design.Add("kind","diameter");design.Add("notes","");designs.Add(design);library.Set("designs",designs);
    check(ValidateStaveLibrary(library,error),"valid stave design library");
    String path=AppendFileName(GetExeFolder(),"stave-test-"+AsString(Uuid::Create())+".json"),expected;bool exists=false;
    check(WriteJsonFile(path,library,expected,exists,error)&&ValidateStaveLibrary(ParseJSON(LoadFile(path)),error),"stave library disk round trip");
    designs.Add(design);library.Set("designs",designs);check(!ValidateStaveLibrary(library,error),"duplicate shell ID rejected");
    FileDelete(path);FileDelete(path+".bak");
    auto start=std::chrono::steady_clock::now();double sum=0;x=StaveInput();x.count=48;x.spacerThicknessMm=2;
    for(int i=0;i<10000;i++)sum+=CalculateStaves(x).stockThicknessMm;
    double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    check(sum>0,"benchmark evaluates geometry");
    Cout()<<"StaveCore: "<<passed<<" passed, "<<failed<<" failed\n";
    Cout()<<"10000 48-stave calculations: "<<Number(ms)<<" ms\n";return failed;
}
