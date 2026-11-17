#include <windows.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
// Use the actual widget implementation and Win32 windows; substitute only discovery
// and the host rendering, so the test never touches a real MyFinder or captures its UI.
static HWND fixtureHost;
static std::atomic<bool> goodFrame{true};
static std::atomic<bool> darkGlyphs{false};
BOOL FixturePrintWindow(HWND, HDC dc, UINT) {
    RECT r{0,0,640,24}; FillRect(dc,&r,(HBRUSH)GetStockObject(darkGlyphs ? WHITE_BRUSH : BLACK_BRUSH));
    if (!goodFrame.load()) return FALSE;
    for (int x=500;x<=620;x+=24) {
        RECT glyph{x,6,x+8,18};FillRect(dc,&glyph,(HBRUSH)GetStockObject(darkGlyphs ? BLACK_BRUSH : WHITE_BRUSH));
    }
    return TRUE;
}
HWND FixtureFindWindow(LPCWSTR cls,LPCWSTR title) {
    if (cls && wcscmp(cls,L"MyFinderApp")==0) return fixtureHost;
    return FindWindowW(cls,title);
}
#define PrintWindow FixturePrintWindow
#define FindWindowW FixtureFindWindow
#include "../src/widget.cpp"
#undef FindWindowW
#undef PrintWindow
void Check(bool ok,const char* message) {
    std::printf("%s: %s\n",ok ? "PASS" : "FAIL",message);std::fflush(stdout);
    if (!ok) std::exit(1);
}
void Pump(int ms) {
    auto end=GetTickCount64()+ms;
    while(GetTickCount64()<end) {
        MSG m;while(PeekMessageW(&m,nullptr,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);} Sleep(5);
    }
}
int main() {
    HINSTANCE inst=GetModuleHandleW(nullptr);
    fixtureHost=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"MyFinder test host",WS_POPUP,
                               -5000,-5000,640,24,nullptr,nullptr,inst,nullptr);
    Check(fixtureHost!=nullptr,"fixture host created");ShowWindow(fixtureHost,SW_SHOWNOACTIVATE);
    HWND notify=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"Fixture notification owner",WS_POPUP,
                               -5000,-5000,1,1,nullptr,nullptr,inst,nullptr);
    Check(notify!=nullptr,"separate application owner created");
    widget::SetMyFinder(true);widget::Create(inst,notify,WM_APP+3,false);
    Pump(1700);widget::Reposition();HWND panel=g_wnd;
    Check(panel && IsWindowVisible(panel),"initial valid layout shows strip");
    Check(GetWindow(panel,GW_OWNER)==nullptr,"strip does not inherit host popup lifecycle");
    SetWindowPos(fixtureHost,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    Pump(200);
    bool hostOnTop=false;
    for(HWND next=GetWindow(panel,GW_HWNDPREV);next;next=GetWindow(next,GW_HWNDPREV)) if(next==fixtureHost)hostOnTop=true;
    Check(!hostOnTop,"host raise repairs strip order without one-second timer");
    ShowOwnedPopups(fixtureHost,FALSE);
    Check(IsWindowVisible(panel),"opening host popups does not hide strip");
    HWND menu=CreateWindowExW(WS_EX_TOOLWINDOW,L"STATIC",L"Fixture menu",WS_POPUP,
                             -5000,-5000,100,100,fixtureHost,nullptr,inst,nullptr);
    Check(menu!=nullptr,"fixture menu created");ShowWindow(menu,SW_SHOWNOACTIVATE);
    SetWindowPos(menu,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    widget::Reposition();
    bool foundStrip=false, foundHost=false;
    for(HWND next=GetWindow(menu,GW_HWNDNEXT);next;next=GetWindow(next,GW_HWNDNEXT)) {
        if(next==panel) foundStrip=true;
        if(next==fixtureHost) {foundHost=true;break;}
    }
    Check(foundStrip && foundHost,"strip stays below menu and above host");
    DestroyWindow(menu);
    bool repairedRepeatedly=true;
    for(int i=0;i<12;++i) {
        SetWindowPos(fixtureHost,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
        Pump(60);
        for(HWND p=GetWindow(panel,GW_HWNDPREV);p;p=GetWindow(p,GW_HWNDPREV)) if(p==fixtureHost)repairedRepeatedly=false;
    }
    Check(repairedRepeatedly,"repeated host raises repair order within 60 ms pump");
    SetWindowPos(fixtureHost,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    widget::Reposition();Check((GetWindowLongPtrW(panel,GWL_EXSTYLE)&WS_EX_TOPMOST)!=0,"strip follows topmost host band");
    SetWindowPos(fixtureHost,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    widget::Reposition();Check((GetWindowLongPtrW(panel,GWL_EXSTYLE)&WS_EX_TOPMOST)==0,"strip leaves topmost band with host");
    darkGlyphs=true;Pump(1300);widget::Reposition();Check(g_light,"dark MyFinder glyphs produce dark strip glyphs");
    widget::SetLightTheme(false);Check(g_light,"Windows dark theme cannot override MyFinder glyph color");
    darkGlyphs=false;Pump(1300);widget::Reposition();Check(!g_light,"light MyFinder glyphs produce light strip glyphs");
    goodFrame=false;Pump(1300);widget::Reposition();
    Check(IsWindowVisible(panel),"temporary failed frame keeps last good layout");
    Pump(3800);widget::Reposition();Check(!IsWindowVisible(panel),"persistently stale layout hides strip");
    goodFrame=true;Pump(1500);widget::Reposition();Check(IsWindowVisible(panel),"valid layout recovers");
    ShowWindow(fixtureHost,SW_HIDE);widget::Reposition();Check(!IsWindowVisible(panel),"hidden host hides strip");
    widget::Destroy();Check(!IsWindow(panel),"destroy releases strip window");
    Check(!g_finderOrderHook && !g_finderForegroundHook,"destroy releases z-order event hooks");
    DestroyWindow(notify);DestroyWindow(fixtureHost);std::puts("Visibility lifecycle: all checks passed");
}
