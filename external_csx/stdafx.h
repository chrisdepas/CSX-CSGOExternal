#pragma once

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <WinDef.h>
#include <WinBase.h>
#include <math.h>

// Required for EnumProcesses
#if PSAPI_VERSION == 1
	#pragma comment(lib,"Psapi.lib")
#endif
#include <Psapi.h>
