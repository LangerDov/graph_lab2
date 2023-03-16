#pragma warning(disable:4244)

#include "core.h"
#include "globalvars.h"
#include "audio/audio.h"
#include "debugging/log.h"
#include "thread/asyncthread.h"
#include "thread/sync.h"
#include "window/windowmanager.h"
#include "time/time.h"

#include "files/files.h"
#include "files/config.h"

#include "environment/main/mainenvironment.h"
#include "list.h"
#include "thread/threadmgr.h"

struct tickProc
{
	void(*func)(void*);
	void* data;
};

bool mainLoop = true;

Config* config;

List<tickProc*>* TickProcess;
Mutex* TkPMutex;
CAsyncThread* TkPThread;

globalVars* gVars = nullptr;
WindowManager* WinManager = nullptr;
Timer* timer = nullptr;

void(*logprint)(UString) = nullptr;

List<BaseEnvironment*>* LoadedEnv;

BaseEnvironment* ActiveEnv;

CAsyncThread* SlThread;

FILES::File* log_file = new FILES::File();


void TickThread(void* data);
void SleepThread(void* data);


Core::Core()
{
	gVars = new globalVars();
	if (!FILES::PathExist("logs"))
		FILES::CreateDir("logs");
	FILES::RemoveFile("logs/last.log");
	log_file->Open("logs/last.log",write_, open_new);
	
	config = new Config();
	config->Init();

	if (AudioMixer::Init())
		Log(lDebug, "AudioMixer loaded succsesful!");

	WinManager = new WindowManager();
	if (WinManager->StartUp())
		Log(lDebug, "WindowManger loaded succsesful!");
	timer = new Timer();
	if (timer->Init())
		Log(lDebug, "Time system loaded succsesful!");

	LoadedEnv = new List<BaseEnvironment*>();
	gVars->work = true;
	TkPMutex = new Mutex;
	TkPMutex->Init(false);
	TickProcess = new List<tickProc*>;
	TkPThread = new CAsyncThread;
	threadinfo_t* TkPThreadInfo = new threadinfo_t();
	TkPThreadInfo->pFunc = TickThread;
	TkPThread->Start(TkPThreadInfo);
	Log(lDebug, "Tick system loaded");
	SlThread = new CAsyncThread;
	threadinfo_t* slThreadInfo = new threadinfo_t();
	slThreadInfo->pFunc = SleepThread;
	SlThread->Start(slThreadInfo);
	Log(lDebug, u8"Sleep env thread loaded");
	Log(lInfo, u8"Core object loaded!");
	MainEnv* mainenv = new MainEnv;
	mainenv->Create();
	LoadedEnv->Add(mainenv);
	ActiveEnv = mainenv;
}

void TickThread(void* data)
{
	unsigned long long start, end,diff;
	int size = 0;
	tickProc* tKP;
	while (gVars->work)
	{
		start = GetTimeHighRes();
		TkPMutex->Enter();
		size = TickProcess->Size();
		for (size_t i = 0; i < size; i++)
		{
			tKP = TickProcess->El(i);
			tKP->func(tKP->data);
		}

		TkPMutex->Leave();
		end = GetTimeHighRes();
		diff = end-start;
		//Log(lDebug,"%llu %llu %llu", diff,start,end);
		gVars->TPS_time = diff;
		if (diff > 1000000 / gVars->TPS_max)
			continue;
		Sleep(1000000 / gVars->TPS_max - diff);
	}
}

void SleepThread(void* data)
{
	while (gVars->work)
	{
		for (int i = 0; i < LoadedEnv->Size(); i++)
			LoadedEnv->El(i)->SleepLoop();
		Sleep(1);
	}
}

void Core::Run()
{

	while (mainLoop)
	{
		if (ActiveEnv)
			ActiveEnv->MainLoop();
		else
			Sleep(1);
	}
	g_pThreadManager->WaitForThreadsToExit();
	for (size_t i = 0; i < LoadedEnv->Size(); i++)
	{
		LoadedEnv->El(i)->Destroy();
	}
	WinManager->CleanUp();
	AudioMixer::Destroy();
	timer->ShutDown();
}

tickProc_t Core::CreateTickProc(void(*func)(void*), void* data)
{
	tickProc* proc = new tickProc;
	proc->data = data;
	proc->func = func;
	TkPMutex->Enter();
	TickProcess->Add(proc);
	TkPMutex->Leave();
	return proc;
}

bool Core::RemoveTickProc(tickProc_t proc)
{
	int i = TickProcess->Find((tickProc*)proc);
	if (i == -1)
		return false;
	TkPMutex->Enter();
	TickProcess->Remove(i);
	TkPMutex->Leave();
	return true;
}

void Core::ShutDown()
{
	ActiveEnv = nullptr;
	gVars->work = false;
	mainLoop = false;
}
