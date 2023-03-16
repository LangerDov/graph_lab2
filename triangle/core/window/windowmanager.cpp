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

bool WindowManager::DestroyWindow(window_t win)
{
    size_t i = windows->Find((Window*)win);
    if (i > -1)
    {
        windows->El(i)->Destroy();
        return false;
    }
    
    return true;
}

window_t WindowManager::GetWindowHWIN(void* HWIN)
{
    for (size_t i = 0; i < windows->Size(); i++)
    {
        if (windows->El(i)->GetHWIN() == HWIN)
            return windows->El(i);
    }
    return nullptr;
}


bool WindowManager::StartUp()
{
    InitWindowSubsys();
    if (!WindowSubsysIsWork())
        return false;
    windows = new List<Window*>();
    return true;
}

bool WindowManager::CleanUp()
{
    for (size_t i = 0; i < windows->Size(); i++)
    {
        windows->El(i)->Destroy();
    }
    return true;
}
