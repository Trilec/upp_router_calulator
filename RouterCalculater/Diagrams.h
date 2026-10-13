#ifndef _RouterCalculater_Diagrams_h_
#define _RouterCalculater_Diagrams_h_
#include <CtrlLib/CtrlLib.h>
#include <Painter/Painter.h>
#include <RouterCalcCore/Staves.h>
namespace Upp { namespace RouterApp {
// GUI-thread, disposable presentation. Project canonical millimetres into a
// bounded design space. Curves use Painter; labels use native Draw text.
struct DiagramShape : Moveable<DiagramShape> {
    enum { LINE, ELLIPSE, BOX, POLYGON } type=LINE;
    Pointf a=Pointf(0,0), b=Pointf(0,0), vertices[4];
    const char* stroke="none"; const char* fill="none";
    double width=1, radius=0, alpha=1;
    bool dashed=false;
    bool operator==(const DiagramShape& q) const;
};
struct DiagramLabel : Moveable<DiagramLabel> {
    Pointf at=Pointf(0,0); String text; const char* ink="#9ca8aa";
    int height=12; bool mono=false;
};
struct DiagramScene {
    Sizef extent=Sizef(840,450);
    Vector<DiagramShape> shapes; Vector<DiagramLabel> labels;
    void Text(double x,double y,const String& text,const char* ink,int height=12,bool mono=false);
    void Line(double x,double y,double xx,double yy,const char* ink,double width=1,bool dashed=false);
    void Circle(double x,double y,double radius,const char* stroke,const char* fill="none",double width=1,double alpha=1,bool dashed=false);
    void Box(double x,double y,double width,double height,const char* fill,const char* stroke="none",double radius=0);
    void Polygon(const Pointf* vertices,const char* fill,const char* stroke,double width=1);
};
class DiagramView : public Ctrl {
    DiagramScene scene;
    Image raster;
    Point raster_offset=Point(0,0);
    struct Label : Moveable<Label> {Point at;String text;Font font;Color ink;};
    Vector<Label> labels;
    struct Line : Moveable<Line> {Point a,b;Color ink;int width;};
    Vector<Line> lines;
    Size rendered_size=Size(0,0);
    int revision=0, rendered_revision=-1, rendered_theme=-1, render_count=0;
    bool invalid=false, labels_dirty=true;
    void Prepare();
public:
    void SetScene(DiagramScene next);
    void SetInvalid(bool value);
    bool IsRetainingInvalid() const {return invalid;}
    int RenderCount() const {return render_count;}
    int PrimitiveCount() const {return scene.shapes.GetCount();}
    const Image& Raster() const {return raster;}
    void Layout() override;
    void Paint(Draw& w) override;
};
DiagramScene CircleDiagram(const RouterCalc::Geometry& g,const String& unit,bool base,bool path,bool radii);
DiagramScene CutterDiagram(double diameter,const String& unit);
DiagramScene StaveDiagram(const RouterCalc::StaveInput& x,const RouterCalc::StaveGeometry& g,const String& unit,int selected);
} }
#endif
