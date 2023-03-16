#include "windowmanager.h"
#include "window.h"


List<Window*>* windows;

window_t WindowManager::CreateWindow()
{
    Window* win = new Window();
    win->Create();
    windows->Add(win);
    return win;
}

bool WindowManager::StartUp()
{
    InitWindowSubsys();
    if (!WindowSubsysIsWork())
        return false;
    windows = new List<Window*>();
    return true;
}
