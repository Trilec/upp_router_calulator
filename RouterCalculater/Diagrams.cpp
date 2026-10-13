#include "Diagrams.h"
#include <cmath>
#include <cstring>
namespace Upp { namespace RouterApp {
bool IsDark(); Color ThemeColor(const char*); Color Coral(); Color Panel();
Font TextFont(int,bool,bool);
bool DiagramShape::operator==(const DiagramShape& q) const
{
    if(type!=q.type||a!=q.a||b!=q.b||std::strcmp(stroke,q.stroke)||std::strcmp(fill,q.fill)||width!=q.width||radius!=q.radius||alpha!=q.alpha||dashed!=q.dashed)return false;
    if(type==POLYGON)for(int i=0;i<4;i++)if(vertices[i]!=q.vertices[i])return false;
    return true;
}
void DiagramScene::Text(double x,double y,const String& t,const char* ink,int height,bool mono)
{
    auto& p=labels.Add();p.at=Pointf(x,y);p.text=t;p.ink=ink;p.height=height;p.mono=mono;
}
void DiagramScene::Line(double x,double y,double xx,double yy,const char* ink,double width,bool dashed)
{
    auto& p=shapes.Add();p.type=DiagramShape::LINE;p.a=Pointf(x,y);p.b=Pointf(xx,yy);p.stroke=ink;p.width=width;p.dashed=dashed;
}
void DiagramScene::Circle(double x,double y,double r,const char* stroke,const char* fill,double width,double alpha,bool dashed)
{
    if(!std::isfinite(r)||r<=0)return;
    auto& p=shapes.Add();p.type=DiagramShape::ELLIPSE;p.a=Pointf(x,y);p.b=Pointf(r,r);p.stroke=stroke;p.fill=fill;p.width=width;p.alpha=alpha;p.dashed=dashed;
}
void DiagramScene::Box(double x,double y,double width,double height,const char* fill,const char* stroke,double radius)
{
    auto& p=shapes.Add();p.type=DiagramShape::BOX;p.a=Pointf(x,y);p.b=Pointf(width,max(0.0,height));p.fill=fill;p.stroke=stroke;p.radius=radius;
}
void DiagramScene::Polygon(const Pointf* v,const char* fill,const char* stroke,double width)
{
    auto& p=shapes.Add();p.type=DiagramShape::POLYGON;for(int i=0;i<4;i++)p.vertices[i]=v[i];p.fill=fill;p.stroke=stroke;p.width=width;
}
void DiagramView::SetScene(DiagramScene next)
{
    bool same=scene.extent==next.extent&&scene.shapes.GetCount()==next.shapes.GetCount();
    if(same)for(int i=0;i<scene.shapes.GetCount();i++)if(!(scene.shapes[i]==next.shapes[i])){same=false;break;}
    bool text_same=scene.labels.GetCount()==next.labels.GetCount();
    if(text_same)for(int i=0;i<scene.labels.GetCount();i++){
        const auto& a=scene.labels[i];const auto& b=next.labels[i];
        if(a.at!=b.at||a.text!=b.text||a.height!=b.height||a.mono!=b.mono||std::strcmp(a.ink,b.ink)){text_same=false;break;}
    }
    if(same&&text_same)return;
    scene=pick(next);if(!same)++revision;labels_dirty=true;
    Prepare();Refresh();
}
void DiagramView::SetInvalid(bool value){if(value!=invalid){invalid=value;Refresh();}}
void DiagramView::Prepare()
{
    Size sz=GetSize();int theme=IsDark()?1:0;
    if(sz.cx<=0||sz.cy<=0||scene.extent.cx<=0||scene.extent.cy<=0)return;
    bool resized=sz!=rendered_size;
    bool rebuild=resized||rendered_revision!=revision||rendered_theme!=theme;
    if(!rebuild&&!labels_dirty)return;
    double k=min(sz.cx/scene.extent.cx,sz.cy/scene.extent.cy);
    Pointf offset((sz.cx-scene.extent.cx*k)/2,(sz.cy-scene.extent.cy*k)/2);
    // Screen-space cap, independent of real-world workpiece dimensions.
    if(int64(sz.cx)*sz.cy>16*1024*1024){raster=Image();labels.Clear();return;}
    if(rebuild){
        raster_offset=Point(int(std::floor(offset.x)),int(std::floor(offset.y)));
        Size fitted(int(std::ceil(scene.extent.cx*k))+1,int(std::ceil(scene.extent.cy*k))+1);
        ImageBuffer buffer(fitted);BufferPainter p(buffer);
        RGBA clear;clear.r=clear.g=clear.b=clear.a=0;p.Clear(clear);
        p.Translate(offset.x-raster_offset.x,offset.y-raster_offset.y);p.Scale(k);
        for(const auto& s:scene.shapes){
            if(s.type==DiagramShape::LINE)continue; // Native Draw handles straight strokes.
            p.Begin();if(s.dashed)p.Dash("4 5",0);
            switch(s.type){
            case DiagramShape::LINE:p.Move(s.a.x,s.a.y).Line(s.b.x,s.b.y);break;
            case DiagramShape::ELLIPSE:p.Ellipse(s.a.x,s.a.y,s.b.x,s.b.y);break;
            case DiagramShape::BOX:p.RoundedRectangle(s.a.x,s.a.y,s.b.x,s.b.y,s.radius);break;
            case DiagramShape::POLYGON:
                p.Move(s.vertices[0].x,s.vertices[0].y);for(int i=1;i<4;i++)p.Line(s.vertices[i].x,s.vertices[i].y);p.Close();break;
            }
            if(std::strcmp(s.fill,"none")){p.Opacity(s.alpha);p.Fill(ThemeColor(s.fill));p.Opacity(1);}
            if(std::strcmp(s.stroke,"none"))p.Stroke(s.width,ThemeColor(s.stroke));
            p.End();
        }
        raster=Image(buffer);++render_count;
    }
    lines.Clear();
    auto point=[&](Pointf a){return Point(int(std::lround(offset.x+a.x*k)),int(std::lround(offset.y+a.y*k)));};
    for(const auto& s:scene.shapes)if(s.type==DiagramShape::LINE){
        Pointf delta=s.b-s.a;double length=std::hypot(delta.x,delta.y);
        auto add=[&](Pointf a,Pointf b){auto& line=lines.Add();line.a=point(a);line.b=point(b);line.width=max(1,int(std::lround(s.width*k)));line.ink=ThemeColor(s.stroke);};
        if(s.dashed&&length>0){for(double t=0;t<length;t+=9)add(s.a+delta*(t/length),s.a+delta*(min(t+4,length)/length));}
        else add(s.a,s.b);
    }
    labels.Clear();
    for(const auto& source:scene.labels){
        auto& t=labels.Add();t.text=source.text;t.ink=ThemeColor(source.ink);
        // Height already in device pixels; apply no second DPI conversion.
        t.font=TextFont(12,source.mono,false).Height(max(8,int(std::lround(source.height*k))));
        t.at=Point(int(std::lround(offset.x+source.at.x*k)),int(std::lround(offset.y+source.at.y*k))-t.font.GetAscent());
    }
    rendered_size=sz;rendered_revision=revision;rendered_theme=theme;labels_dirty=false;
}
void DiagramView::Layout(){Prepare();}
void DiagramView::Paint(Draw& w)
{
    if(!IsNull(raster))w.DrawImage(raster_offset.x,raster_offset.y,raster);
    for(const auto& line:lines)w.DrawLine(line.a.x,line.a.y,line.b.x,line.b.y,line.width,line.ink);
    for(const auto& t:labels)w.DrawText(t.at.x,t.at.y,t.text,t.font,t.ink);
    if(invalid){
        int height=DPI(26);w.DrawRect(0,0,GetSize().cx,height,Panel());
        w.DrawRect(0,0,GetSize().cx,DPI(2),Coral());
        w.DrawText(DPI(8),DPI(6),"LAST VALID PREVIEW - correct the highlighted input",TextFont(11,false,true),Coral());
    }
}
} }
