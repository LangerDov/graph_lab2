#pragma once
#include "callback.h"
typedef void* timer_t;

class Timer
{
friend class Core;
public:
	timer_t CreateTimer(callback_t<void>);
	bool DestroyTimer(timer_t);
	const char* GetTimeFormatString(const char* format);
	// time in miliseconds in utc
	unsigned long long GetSysTime();
private:
	bool Init();
	bool ShutDown();
};

// time in microseconds, based on CPU timer
extern size_t GetTimeHighRes();

extern Timer* timer;