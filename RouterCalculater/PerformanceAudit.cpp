#include "Window.h"
#include <chrono>
#include <cmath>
namespace Upp { namespace RouterApp {
// Opt-in audit on an isolated window. No saved user libraries are read/written.
// Measures real changed editor/slider handlers through OS paint submission;
// not monitor scan-out, mouse hardware latency, or an unchanged-cache shortcut.
struct PerformanceAudit {
    using Clock=std::chrono::steady_clock;
    String report;int failures=0,targets_missed=0,workloads=0;
    static double Ms(Clock::time_point start){return std::chrono::duration<double,std::milli>(Clock::now()-start).count();}
    void Log(const String& s){report<<s;Cout()<<s;}
    double Row(const String& label,Vector<double>& v){
        double sum=0;int over60=0,over30=0;for(double t:v){sum+=t;over60+=t>1000.0/60;over30+=t>1000.0/30;}
        Sort(v);int n=v.GetCount();double p95=v[int(std::ceil(n*.95))-1];
        Log(Format("%s: n=%d mean=%.3f p95=%.3f max=%.3f ms; >16.67=%d >33.33=%d\n",label,n,sum/n,p95,v.Top(),over60,over30));return p95;
    }
    template<class Action,class Valid> void Work(RouterWindow& w,const String& label,DiagramView& view,Action action,Valid valid){
        for(int i=0;i<6;i++){action(i);w.Sync();Ctrl::ProcessEvents();}
        Vector<double> callback,paint,total;int before=view.RenderCount();
        for(int i=0;i<60;i++){
            auto start=Clock::now();action(i+6);double input=Ms(start);
            w.Sync();Ctrl::ProcessEvents();double complete=Ms(start);
            callback.Add(input);paint.Add(complete-input);total.Add(complete);
            if(!valid())++failures;
        }
        Row(label+" callback",callback);Row(label+" paint submission",paint);
        double p95=Row(label+" input-to-paint",total);++workloads;targets_missed+=p95>1000.0/60;
        int renders=view.RenderCount()-before;Log(Format("  geometry raster rebuilds: %d / 60 changed events\n",renders));
        if(renders>60)++failures;
    }
    int Run(RouterWindow& w,const String& folder){
        Log(AppIdentity()+"\nExecutable: "+GetExeFilePath()+"\n");
#ifdef _DEBUG
        Log("Configuration: DEBUG\n");
#else
        Log("Configuration: RELEASE\n");
#endif
        Log(Format("DPI(100)=%d. Target p95 <=16.67 ms; minimum 30 fps budget 33.33 ms.\n",DPI(100)));
        auto& c=w.circle.canvas;auto& s=w.staves.canvas;
        for(bool dark:{true,false})for(int width:{980,1320}){
            SetDark(dark);w.ApplyTheme();w.SetRect(0,0,DPI(width),DPI(900));w.Layout();
            String tag=String(dark?"dark/":"light/")+AsString(width);
            w.tabs.SetActiveTab(0);w.circle.Layout();c.NewDesign(false);c.ChangeUnit("mm");c.ChangeKind("radius");w.Sync();Ctrl::ProcessEvents();
            Work(w,tag+" circle key",c.diagram,[&](int){c.hole.Key(K_UP,1);},[&]{return c.geometry.IsValid();});
            Work(w,tag+" circle wheel",c.diagram,[&](int){c.hole.MouseWheel(Point(10,10),120,0);},[&]{return c.geometry.IsValid();});
            Work(w,tag+" circle slider",c.diagram,[&](int i){c.hole_slider.SetValue(60+i*.25);c.hole_slider.WhenChanging();},[&]{return c.geometry.IsValid();});
            w.tabs.SetActiveTab(1);w.staves.Layout();s.New(false);s.ChangeUnit("mm");s.ChangeKind("diameter");w.Sync();Ctrl::ProcessEvents();
            Work(w,tag+" stave outside wheel",s.preview,[&](int){s.fields[StaveCanvas::OUTER].MouseWheel(Point(10,10),120,0);},[&]{return s.geometry.IsValid();});
            Work(w,tag+" stave wall wheel",s.preview,[&](int){s.fields[StaveCanvas::WALL].MouseWheel(Point(10,10),120,0);},[&]{return s.geometry.IsValid();});
            Work(w,tag+" stave count wheel",s.preview,[&](int){s.fields[StaveCanvas::COUNT].MouseWheel(Point(10,10),120,0);},[&]{return s.geometry.IsValid();});
            s.New(false);w.Sync();Ctrl::ProcessEvents();
            Work(w,tag+" stave outside slider",s.preview,[&](int i){s.outside_slider.SetValue(400+i*.5);s.outside_slider.WhenChanging();},[&]{return s.geometry.IsValid();});
            Work(w,tag+" stave count slider",s.preview,[&](int i){s.count_slider.SetValue(12+i%40);s.count_slider.WhenChanging();},[&]{return s.geometry.IsValid();});
        }
        c.dirty=s.dirty=false;
        Log(Format("Functional failures: %d. 60-fps p95 target missed by %d of %d workloads.\n",failures,targets_missed,workloads));
        Log("Timing misses are reported separately, not disguised as functional passes. This is not a paced presentation/scan-out measurement.\n");
        if(!SaveFile(AppendFileName(folder,"performance-audit.txt"),report))++failures;
        return failures;
    }
};
int RunPerformanceAudit(RouterWindow& w,const String& folder){PerformanceAudit audit;return audit.Run(w,folder);}
} }
