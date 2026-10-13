#include "App.h"
namespace Upp { namespace RouterApp {
using RouterCalc::Number;
static int CLLogical(int px){return int(px*100.0/DPI(100)+.5);}
int CircleCanvas::ContentHeight(int) const
{
    return 738 + (notes_open ? 82 : 0) + ((expanded_formula || expanded_safety) ? 164 : 0);
}
void CircleCanvas::Layout()
{
    const int width = CLLogical(GetSize().cx);
    if(width < 700) return;
    left_width=332;right_x=344;right_width=width-right_x;
    notes_extra=notes_open?82:0;diagram_height=PREVIEW_HEIGHT;table_y=484;below_y=566+notes_extra;
    const int y=76,split=right_x+right_width/2;
    form.Panel(6,0,0,width,66);
    form.Panel(0,0,y,left_width,480+notes_extra);
    form.Panel(1,right_x,y,right_width,64,1);
    form.Panel(2,right_x,150,right_width,diagram_height);
    form.Panel(3,right_x,table_y,right_width,174);
    form.Panel(4,0,below_y,left_width,expanded_formula?326:162);
    form.Panel(5,right_x,668,right_width,expanded_safety?224:60,2);
    auto put=[&](Ctrl& c,int p,int x,int yy,int w,int h){form.Place(c,p,x,yy,w,h);};
    const int actions=width-320,picker=min(300,(actions-40)/2);
    form.Label("saved.label",6,14,8,148,16,"SAVED DESIGNS",10);
    form.Label("status",6,170,8,width-184,17,"",10);
    put(saved,6,14,28,picker,28);put(name,6,24+picker,28,actions-picker-34,28);
    put(new_design,6,actions,28,65,28);put(save,6,actions+73,28,65,28);
    put(settings,6,actions+146,28,77,28);put(export_file,6,actions+231,28,75,28);
    put(radius,0,180,y+10,57,25);put(diameter,0,241,y+10,77,25);
    form.Label("title",0,14,y+12,154,21,"Your measurements",14,"#ecede8",false,true);
    form.Label("hole.label",0,14,y+44,304,17,"Finished hole",11,"#ecede8",false,true);
    put(hole,0,14,y+63,304,32);put(suffix[0],0,282,y+70,29,20);
    form.Label("hole.value",0,14,y+101,304,17,"",10);
    put(hole_slider,0,14,y+123,304,17);
    form.Label("guide.label",0,14,y+149,304,17,"Router base / guide diameter",11,"#ecede8",false,true);
    put(guide,0,14,y+168,304,30);put(suffix[1],0,282,y+174,29,20);
    form.Label("guide.help",0,14,y+203,304,17,"Measure at the template contact height.",10);
    put(guide_slider,0,14,y+224,304,17);
    form.Label("cutter.label",0,14,y+250,304,17,"Router cutter diameter",11,"#ecede8",false,true);
    put(cutter,0,14,y+269,304,30);put(suffix[2],0,282,y+275,29,20);
    put(cutter_slider,0,14,y+311,304,17);
    put(cutter_picture,0,14,y+335,304,100);
    put(note_toggle,0,14,y+445,190,24);
    notes.Show(notes_open);put(notes,0,14,y+476,304,74);
    form.Label("result.heading",1,right_x+14,y+8,right_width/2-28,16,"TEMPLATE OPENING · RADIUS",9,"#e6b972");
    form.Label("result.radius",1,right_x+14,y+29,right_width/2-28,29,"",23,"#e6b972",true);
    form.Label("diameter.heading",1,split+14,y+8,right_width/2-28,16,"FULL DIAMETER",9,"#e6b972");
    form.Label("result.diameter",1,split+14,y+29,right_width/2-28,29,"",21,"#ecede8",true);
    form.Label("preview.title",2,right_x+14,161,right_width-260,21,"How the circles fit",14,"#ecede8",false,true);
    put(show_base,2,right_x+right_width-240,159,56,22);put(show_path,2,right_x+right_width-179,159,103,22);put(show_radii,2,right_x+right_width-71,159,57,22);
    form.Label("preview.subtitle",2,right_x+14,186,right_width-28,17,"TOP VIEW · FIXED AT 0° · NOT PRINT SCALE",9);
    put(diagram,2,right_x+10,206,right_width-20,216);
    form.Label("preview.legend",2,right_x+14,427,right_width-28,18,"Template · hole · guide · cutter · centre path",10);
    form.Label("result.detail",2,right_x+14,448,right_width-28,24,"",10,"#9ca8aa",false,false,true);
    form.Label("table.title",3,right_x+14,table_y+10,right_width-174,21,"All dimensions",13,"#ecede8",false,true);
    put(copy,3,right_x+right_width-159,table_y+7,145,25);
    put(dimensions,3,right_x+12,table_y+36,right_width-24,136);
    dimensions.SetColumnWidth(0,DPI(145));
    for(int i=1;i<5;i++)dimensions.SetColumnWidth(i,DPI((right_width-183)/4));
    form.Label("formula.title",4,14,below_y+12,304,21,"Why this works",13,"#ecede8",false,true);
    form.Label("formula",4,14,below_y+41,304,19,"Template R = hole R + guide R − cutter R",11,"#ecede8");
    form.Label("equation",4,14,below_y+66,304,19,"",11,"#e6b972");
    form.Label("explanation",4,14,below_y+92,304,32,"",10,"#9ca8aa",false,false,true);
    put(formula_more,4,14,below_y+132,304,23);
    form.Label("formula.extra",4,14,below_y+179,304,124,"Template Ø = hole Ø + guide Ø − cutter Ø. This sizes the template's inside opening, not the board edge. Cutter width is not shank size. Support central waste separately. Plunge depth does not change this top-view equation for a straight cutter.",11,"#9ca8aa",false,false,true);
    form.Show("formula.extra",expanded_formula);
    form.Label("safety.title",5,right_x+14,678,right_width-216,21,"Before the cut",13,"#e6b972",false,true);
    put(safety_more,5,right_x+right_width-204,675,190,24);
    form.Label("safety",5,right_x+14,704,right_width-28,22,"Use a circular, concentric contact edge. Check the tool; test in scrap.",10);
    form.Label("safety.extra",5,right_x+14,742,right_width-28,127,"162 mm is a provisional guide measurement, not a certified Bosch specification. Flats, taper and an off-centre guide change the result. Unplug before measuring; securely support the template, workpiece and waste. The calculator cannot check the physical tool setup.",11,"#9ca8aa",false,false,true);
    form.Show("safety.extra",expanded_safety);
    UpdateReadouts();
}
void CircleCanvas::UpdateReadouts()
{
    bool valid=geometry.IsValid();form.Text("hole.value","R "+Measure(input.holeRadiusMm)+" · Ø "+Measure(input.holeRadiusMm*2));
    form.Text("cutter.value","Cutter R = "+Measure(input.cutterDiameterMm/2));form.Text("status",status);form.Ink("status",!valid?"#f09987":dirty?"#e6b972":"#80d6bb");
    form.Text("result.radius",valid?"R "+Measure(geometry.templateR):String("—"));form.Text("result.diameter",valid?"Ø "+Measure(geometry.templateR*2):String());
    form.Text("result.detail",valid?String("Centre to the inside template edge."):geometry.error);form.Ink("result.detail",valid?"#9ca8aa":"#f09987");
    form.Text("equation",valid?Number(geometry.templateR/Factor())+" = "+Number(geometry.holeR/Factor())+" + "+Number(geometry.baseR/Factor())+" − "+Number(geometry.bitR/Factor())+" "+unit:String("Correct the highlighted measurement."));
    form.Text("explanation",valid?"Cutter-centre travel: "+Measure(geometry.pathR)+". The cutter reaches the finished hole; the guide reaches the template opening.":String("The last valid preview is retained for orientation. Readouts and export remain unavailable until the current measurements are valid."));
    auto& model=dimensions.Model();if(model.GetRowCount()!=5)return;UiModelUpdate update(model);
    const char* names[]={"Template opening","Finished hole","Router guide","Cutter","Centre path"};double radii[]={geometry.templateR,geometry.holeR,geometry.baseR,geometry.bitR,geometry.pathR};
    for(int r=0;r<5;r++){SetReadoutCell(model,r,0,names[r]);double values[]={radii[r],radii[r]*2,radii[r]/10,radii[r]/5};for(int c=0;c<4;c++)SetReadoutCell(model,r,c+1,valid?Number(values[c]):String("—"));}
}
void CircleCanvas::Paint(Draw& w){w.DrawRect(GetSize(),Bg());}
} }
