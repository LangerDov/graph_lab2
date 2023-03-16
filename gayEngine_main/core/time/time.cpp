
#include "time.h"

#ifdef _WIN32
#include "time.h"
#include <math.h>
#include <Windows.h>

LARGE_INTEGER Frequency;

timer_t Timer::CreateTimer(callback_t<void>)
{
    return timer_t();
}

bool Timer::DestroyTimer(timer_t)
{
    return false;
}

const char* Timer::GetTimeFormatString(const char* format)
{
    return nullptr;
}

unsigned long long Timer::GetSysTime()
{
    return 0;
}
double PCFreq = 0.0;
bool Timer::Init()
{
    QueryPerformanceFrequency(&Frequency);
    return true;
}

bool Timer::ShutDown()
{
    return true;
}

inline size_t GetTimeHighRes()
{
    LARGE_INTEGER time;
    QueryPerformanceFrequency(&Frequency);
    QueryPerformanceCounter(&time);
    return (time.QuadPart/Frequency.QuadPart)*1000000;
}
#else

timer_t Timer::CreateTimer(callback_t<void>) {
    return nullptr;
}

bool Timer::DestroyTimer(timer_t) {
    return false;
}

const char *Timer::GetTimeFormatString(const char *format) {
    return nullptr;
}

unsigned long long Timer::GetSysTime() {
    return 0;
}

bool Timer::Init() {
    return false;
}

bool Timer::ShutDown() {
    return false;
}

 unsigned long long GetTimeHighRes()
{
    return -1;
}
#endif