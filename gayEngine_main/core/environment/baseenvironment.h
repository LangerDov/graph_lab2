#pragma once

#include "str.h"

class BaseEnvironment
{
public:
	virtual bool Create() = 0;
	virtual bool Destroy() = 0;
	virtual bool AcceptData(int id,BaseEnvironment* sender, void* data) = 0;
	virtual bool RequestControl() = 0;
	virtual UString GetName() = 0;
	virtual void MainLoop() = 0;
	virtual void SleepLoop() = 0;
private:

};
