#ifndef _CORE_H
#define _CORE_H

#include "environment/baseenvironment.h"
#include "str.h"

typedef void* tickProc_t;


class Core
{
public:
	Core();
	void Run();
	static tickProc_t CreateTickProc(void(*func)(void*), void* data);
	static bool RemoveTickProc(tickProc_t);
	static void ShutDown();
	static BaseEnvironment* LoadEnv(UString path);
	static bool UnloadEnv(BaseEnvironment*);
	static bool TransferControl(BaseEnvironment* from, BaseEnvironment to);
	static bool RequestControl(BaseEnvironment* sender);
private:

};

#endif