#ifndef _THREADMGR_H
#define _THREADMGR_H
#pragma once

#include "asyncthread.h"

class IThreadManager
{
public:
	virtual void AddToList(threadinfo_t* pInfo) = 0;
	virtual void WaitForThreadsToExit() = 0;
};

extern IThreadManager *g_pThreadManager;

#endif