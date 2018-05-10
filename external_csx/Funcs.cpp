#include "stdafx.h"
#include "Funcs.h"

DWORD* MakePointer(void* pBase, DWORD Offset) {
	return (DWORD*)((DWORD)pBase + Offset);
}

bool CompareSignature(BYTE* pData, BYTE* pSignature, char* szMask) {
	for (; *szMask != NULL; szMask++, pSignature++, pData++) {
		if (*szMask == 'x' && *pSignature != *pData) {
			return false;
		}
	}
	return true;
}

void* SignatureScan(BYTE* pMemory, int iMemorySize, char* szSignature, char* szMask) {
	int endIndex = iMemorySize - strlen(szMask) + 1;
	if (endIndex < 0)
		return NULL;

	for (BYTE* pEnd = (BYTE*)MakePointer(pMemory, endIndex); pMemory < pEnd; pMemory++) {
		// Cast signature to PBYTE to avoid char/BYTE sign mismatch issues
		if (CompareSignature(pMemory, (PBYTE)szSignature, szMask)) {
			return pMemory;
		}
	}

	return NULL;
}

void* SignatureScan(HANDLE hProcess, HMODULE hModule, char* szSignature, char* szMask) {
	DWORD memorySize = GetModuleSize(hProcess, hModule);
	if (memorySize == NULL)
		return NULL;

	// Read entire memory space of module into buffer 
	// This is done to reduce ReadProcessMemory calls, which are expensive 
	// module handle is cast to void* as a HMODULE is a module's base address
	PBYTE pDataBuf = new BYTE[memorySize];
	SIZE_T bytesRead = 0;
	if (ReadProcessMemory(hProcess, (void*)hModule, pDataBuf, memorySize, &bytesRead) == 0) {
		return NULL;
	}
	
	// Find pattern in local buffer
	void* result = SignatureScan(pDataBuf, (int)bytesRead, szSignature, szMask);
	if (result == NULL) {
		delete[] pDataBuf;
		return NULL;
	}

	// Get pattern offset for local buffer
	int localPatternOffset = (int)MakePointer(result, -1 * (DWORD)pDataBuf);
	delete[] pDataBuf;

	// Add offset to module base to get external pattern pointer
	return MakePointer((void*)hModule, localPatternOffset);
}

DWORD GetModuleSize(HANDLE hProcess, HMODULE hModule) {
	MODULEINFO modInfo;
	if (GetModuleInformation(hProcess, hModule, &modInfo, sizeof(modInfo)) == 0)
		return NULL;
	return modInfo.SizeOfImage;
}

EModuleResult GetProcessHandleFromFileName(char* szName, HANDLE& _out_module) {
	_out_module = NULL;

	// Get array of process IDs for all processes running 
	DWORD processID[MAX_PROCESSES], numProcesses = 0;
	if (!EnumProcesses(processID, sizeof(processID), &numProcesses)) {
		return MODULE_ERROR;
	}

	// NumProcess contains size written, convert to number of process IDs
	numProcesses /= sizeof(DWORD);
	
	// Find and return handle to process
	char nameBuf[256];
	for (unsigned int i = 0; i < numProcesses - 1; i++) {
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processID[i]);
		if (!hProcess)
			continue;
	
		int nameLen = GetProcessImageFileNameA(hProcess, nameBuf, 256);
		if (!nameLen)
			continue;
		nameBuf[nameLen] = 0;
		
		// The name returned contains path, get pointer to base filename 
		char* fileName = 0;
		for (int j = nameLen - 1; j > 0; j--) {
			if (nameBuf[j] == '\\') {
				fileName = nameBuf + j + 1;
				break;
			}
		}

		if (fileName && !strcmp(fileName, szName)) {
			_out_module = hProcess;
			return MODULE_SUCCESS;
		} 
		
		CloseHandle(hProcess);
	}

	return MODULE_NOT_FOUND;
}  

EModuleResult GetExternalModule(HANDLE hProcess, char* szName, HMODULE& _out_module) {
	HMODULE handles[MAX_MODULES];
	DWORD nummodules = 0;
	_out_module = NULL;

	// Get list of module handles
	EnumProcessModules(hProcess, handles, sizeof(handles), &nummodules);
	if (nummodules <= 0) 
		return EModuleResult::MODULE_ERROR;

	// numModules contains size written, convert into number of modules
	nummodules /= sizeof(HMODULE);

	// Find and return module handle for provided name
	char nameBuf[256];
	for (unsigned int i = 0; i < nummodules; i++) {

		if (!GetModuleBaseNameA(hProcess, handles[i], nameBuf, 256))
			continue;

		if (!strcmp(szName, nameBuf)) {
			_out_module = handles[i];
			return EModuleResult::MODULE_SUCCESS;
		}

	}

	return EModuleResult::MODULE_NOT_FOUND;
}