#pragma once
#include "str.h"


extern UString GetOSName();

extern locale_t GetOSLocale();

extern UString GetCPUName();

enum cpu_arch
{
	x86,
	amd64,
	armv8_64,
	armv9_64,
};

extern cpu_arch GetCPUArch();

extern size_t GetFreeRAM();
extern size_t GetRAMSize();

extern size_t GetUsedRAM();

extern UString GetAppDir();
