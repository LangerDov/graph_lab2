#include "threadmgr.h"
#include "list.h"
#include "time.h"

class CThreadManager : public IThreadManager
{
public:
	CThreadManager() {}
	~CThreadManager() {}

	virtual void AddToList(threadinfo_t* pInfo)
	{
		threads.Add(pInfo);
	}
	virtual void WaitForThreadsToExit()
	{
		while (true)
		{
			bool bExit = true;
			for (int i = 0; i < threads.Size(); i++)
			{
				if (!threads[i]->bStopped)
				{
					bExit = false;
					break;
				}
			}

			if (bExit)
				break;
		}
		for (int i = 0; i < threads.Size(); i++)
		{
			delete threads[i];
		}
	}

	List<threadinfo_t*> threads;
};

static CThreadManager g_threadmanager;
IThreadManager* g_pThreadManager = &g_threadmanager;