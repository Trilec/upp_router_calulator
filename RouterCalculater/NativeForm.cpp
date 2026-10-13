#include "NativeForm.h"
namespace Upp { namespace RouterApp {
Color ThemeColor(const char*);Color Bg();Color Panel();Color Field();Color Line();Color Ink();Color Muted();Color Teal();
Font TextFont(int,bool,bool);
// The installed Ui nine-slice path stretches subimages during Paint. Prepare
// an exact native skin only when size/theme changes; Paint only blits it.
static Image PanelSkin(Color face,Color frame,Size size)
{
    if(size.cx<=0||size.cy<=0||int64(size.cx)*size.cy>16*1024*1024)return Image();
    int fw=DPI(1);ImageBuffer image(size);
    BufferPainter p(image,MODE_ANTIALIASED);RGBA clear;clear.r=clear.g=clear.b=clear.a=0;p.Clear(clear);
    double inset=fw*.5;p.RoundedRectangle(inset,inset,size.cx-fw,size.cy-fw,DPI(12)).Fill(face).Stroke(fw,frame);
    return Image(image);
}
void StyleNativeLabel(UiLabel& c,int px,const char* colour,bool mono,bool bold,bool wrap)
{
    UiLabel::Style s=UiLabel::StyleDefault();s.font=TextFont(px,mono,bold);
    s.transparent=true;s.nowrap=!wrap;s.align_h=UiAlign::LEFT;s.align_v=UiAlign::TOP;
    s.metrics.content_margin=Rect(0,0,0,0);s.metrics.face_enabled=false;s.metrics.frame_enabled=false;s.metrics.shadow.enabled=false;
    for(int i=0;i<4;i++)s.palette.ink[i]=ThemeColor(colour);
    c.SetCustomStyle(s);
}
void NativeForm::Init(Ctrl& host){for(int i=0;i<PANEL_COUNT;i++){host.Add(panels[i]);panels[i].Hide();StylePanel(i);}}
void NativeForm::StylePanel(int i)
{
    UiPanel::Style s=UiPanel::StyleDefault();s.metrics.content_margin=Rect(0,0,0,0);s.metrics.radius=DPI(12);s.metrics.frame_width=DPI(1);s.metrics.shadow.enabled=false;
    Color face=kinds[i]==1?ThemeColor("#242923"):kinds[i]==2?ThemeColor("#26251f"):RouterApp::Panel();
    Color frame=kinds[i]==1?ThemeColor("#575039"):Line();
    for(int j=0;j<4;j++){s.palette.face[j]=face;s.palette.frame[j]=frame;s.palette.ink[j]=RouterApp::Ink();}
    s.skin.enabled=true;s.skin.base=PanelSkin(face,frame,panels[i].GetSize());s.skin.slice=Rect(0,0,0,0);
    s.metrics.frame_enabled=false; // Frame is in the skin; do not rasterise it again.
    panels[i].SetCustomStyle(s);
}
void NativeForm::StyleCaption(Caption& c){StyleNativeLabel(c.ctrl,c.px,~c.ink,c.mono,c.bold,c.wrap);}
void NativeForm::ApplyTheme(){for(int i=0;i<PANEL_COUNT;i++)StylePanel(i);for(auto& c:captions)StyleCaption(c);}
void NativeForm::Panel(int i,int x,int y,int w,int h,int kind)
{
    bool changed=kinds[i]!=kind||panels[i].GetSize()!=Size(DPI(w),DPI(h));
    bounds[i]=Rect(x,y,x+w,y+h);kinds[i]=kind;
    panels[i].SetRect(DPI(x),DPI(y),DPI(w),DPI(h));if(changed)StylePanel(i);panels[i].Show();
}
void NativeForm::Place(Ctrl& c,int panel,int x,int y,int w,int h)
{
    if(c.GetParent()!=&panels[panel])panels[panel].Add(c);
    c.SetRect(DPI(x-bounds[panel].left),DPI(y-bounds[panel].top),DPI(w),DPI(h));
}
// UiLabel supports explicit lines, not width-dependent word wrapping. Prepare
// line breaks on text/width changes, never during Paint; the native control
// still owns text layout, selection and drawing.
void NativeForm::PrepareCaption(Caption& c)
{
    String shown=c.text;
    if(c.wrap&&c.ctrl.GetSize().cx>0) {
        shown.Clear();const Font font=c.ctrl.GetStyle().font;
        bool first=true;
        for(const String& paragraph:Split(c.text,'\n',false)) {
            if(!first)shown.Cat('\n');first=false;String line;
            for(const String& word:Split(paragraph,' ')) {
                String next=line.IsEmpty()?word:line+" "+word;
                if(!line.IsEmpty()&&GetTextSize(next,font).cx>c.ctrl.GetSize().cx) {
                    shown<<line<<'\n';line=word;
                } else line=next;
            }
            shown<<line;
        }
    }
    shown.Replace("&","&&");c.ctrl.SetText(shown);
}
void NativeForm::Label(const String& id,int panel,int x,int y,int w,int h,const String& text,int px,const char* ink,bool mono,bool bold,bool wrap)
{
    int i=keys.Find(id);if(i<0){i=keys.GetCount();keys.Add(id);captions.Add();}
    auto& c=captions[i];bool style=c.ink!=ink||c.px!=px||c.mono!=mono||c.bold!=bold||c.wrap!=wrap;
    c.panel=panel;c.ink=ink;c.px=px;c.mono=mono;c.bold=bold;c.wrap=wrap;
    if(style)StyleCaption(c);
    const bool resized=c.ctrl.GetSize().cx!=DPI(w);
    Place(c.ctrl,panel,x,y,w,h);
    const bool changed=!text.IsEmpty()&&c.text!=text;if(changed)c.text=text;
    if(changed||resized||style)PrepareCaption(c);
}
void NativeForm::Text(const String& id,const String& text)
{
    int i=keys.Find(id);if(i<0)return;auto& c=captions[i];
    if(c.text==text)return;c.text=text;PrepareCaption(c);
}
void NativeForm::Ink(const String& id,const char* colour){int i=keys.Find(id);if(i>=0&&captions[i].ink!=colour){captions[i].ink=colour;StyleCaption(captions[i]);}}
void NativeForm::Show(const String& id,bool show){int i=keys.Find(id);if(i>=0)captions[i].ctrl.Show(show);}
void StyleNativeTable(UiTable& table)
{
    UiTable::Style s=UiTable::StyleDefault();s.font=TextFont(11,true,false);s.header_font=TextFont(10,false,false);
    s.row_height=DPI(22);s.header_height=DPI(22);s.cell_padding_x=DPI(5);s.cell_padding_y=DPI(3);s.header_padding_x=DPI(5);
    s.show_row_headers=false;s.show_grid=true;s.alternate_rows=false;s.show_sort_indicator=false;
    s.min_column_width=DPI(35);s.max_column_width=DPI(600);s.metrics.frame_enabled=false;s.metrics.radius=0;s.metrics.shadow.enabled=false;
    s.metrics.content_margin=Rect(0,0,0,0);s.table_bg=s.header_bg=s.row_header_bg=s.read_only_bg=Panel();
    s.cell_ink=Ink();s.header_ink=s.muted_ink=Muted();s.grid_color=Line();s.alternate_row_bg=Panel();
    s.hover_bg=s.selection_bg=s.active_bg=Field();s.selection_border=s.active_border=Teal();s.header_hot_bg=Field();
    for(int i=0;i<4;i++){s.palette.face[i]=Panel();s.palette.ink[i]=Ink();s.palette.frame[i]=Line();}
    table.SetCustomStyle(s).EnableInternalMutation(false);
}
void InitReadoutTable(UiTable& table,int rows,const Vector<String>& headers)
{
    auto& model=table.Model();UiModelUpdate update(model);model.SetSize(rows,headers.GetCount());
    for(int c=0;c<headers.GetCount();c++)model.SetHeader(UITABLE_COLUMN_AXIS,c,UiTableHeader(headers[c]));
    for(int r=0;r<rows;r++)for(int c=0;c<headers.GetCount();c++){UiTableCell cell;cell.editable=false;model.SetCell(r,c,cell);}
}
bool SetReadoutCell(UiTableModel& model,int row,int col,const String& text)
{
    if(model.GetCellValue(row,col)==text)return false;
    UiTableCell cell;cell.value=text;cell.editable=false;cell.align=col?ALIGN_RIGHT:ALIGN_LEFT;model.SetCell(row,col,cell);return true;
}
} }
