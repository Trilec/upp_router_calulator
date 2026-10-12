#include "Window.h"
using namespace Upp;
using namespace Upp::RouterApp;
GUI_APP_MAIN
{
    SetDefaultCharset(CHARSET_UTF8);
    const Vector<String>& args=CommandLine();
    bool testing=args.GetCount()>0&&args[0]=="--self-test";
    RouterWindow window(!testing);
    if(testing){
        String folder=args.GetCount()>1?args[1]:AppendFileName(GetExeFolder(),"ui-evidence");
        RealizeDirectory(folder);window.Open();Ctrl::ProcessEvents();
        int failures=window.SelfTest(folder);window.Close();SetExitCode(failures?1:0);return;
    }
    window.Run();
}
