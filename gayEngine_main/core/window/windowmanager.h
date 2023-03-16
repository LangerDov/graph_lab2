#pragma once
#include "windowdefs.h"
#include "list.h"

class WindowManager
{
friend class Core;
public:
	window_t CreateWindow();
	bool DestroyWindow(window_t);
	window_t GetWindowHWIN(void* HWIN);
private:
	bool StartUp();
	bool CleanUp();
	List<window_t> GetAllWindows();
};


extern WindowManager* WinManager;