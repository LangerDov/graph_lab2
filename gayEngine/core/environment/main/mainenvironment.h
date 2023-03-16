#pragma once
#include "environment/baseenvironment.h"
#include "thread/sync.h"

#include "graphics/components/all.h"


class MainEnv: public BaseEnvironment
{
public:
	bool Create();
	bool Destroy();
	UString GetName();
	bool AcceptData(int id, BaseEnvironment* sender, void* data);
	bool RequestControl();
	void SleepLoop();
	void MainLoop();
private:
	WindowForm mywin;
	Triangle tr;
};