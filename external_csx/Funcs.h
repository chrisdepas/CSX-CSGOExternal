#ifndef __FUNCS_H__
#define __FUNCS_H__
//
//	External Sigscan and Module/Process functions
//  Vector / Angle funcs
//	Chris De Pasquale
//

#define _USE_MATH_DEFINES // define M_PI 
#include <math.h>
#include <Windows.h>

// Number of processes to enumerate in GetProcessHandleFromFileName
#define MAX_PROCESSES 1024

// Number of process modules to enumerate in GetExternalModule
#define MAX_MODULES 256

// 180 / PI
#define TO_DEGREE (float)(180 / M_PI)

struct vec3 {
	float x, y, z;

	vec3() {}
	vec3(float _x, float _y, float _z) {
		x = _x; y = _y; z = _z;
	}

	vec3 vec3::operator+(vec3 v) {
		return vec3(x + v.x, y + v.y, z + v.z);
	}
	vec3 vec3::operator-(vec3 v) {
		return vec3(x - v.x, y - v.y, z - v.z);
	}
	void vec3::operator+=(vec3 v) {
		x += v.x; y += v.y; z += v.z;
	}
};

// Makepointer
DWORD* MakePointer(void* pBase, DWORD Offset);

/* Searches an external process module's memory space for a pattern of bytes
Args
  hProcess   - Handle of external process containing module to search
  hModule    - Handle of module to search for signature 
  pSignature - String of bytes to be found within hModule (e.g. "\xAA\xBB\xCC")
  szMask     - String of x/? characters (e.g. "x?x"), with an 'x' indicating 
    that the signature byte at the same position should be checked, and a '?' 
	(or any non-x char) indicating that the corresponding byte should be skipped. 

Return Value
  Pointer to first memory address found which matches signature and mask

Example
  E.g. a process & module with memory space containing these 6 bytes:
	\x00\x11\x22\xAA\x33\x44
  Then SignatureScan(hProcess, hModule, "\x22\x00\x33", "x?x") returns pointer
  (hModule + 2), as this has 0x22 at index 0, 0x33 at index 2) */
void* SignatureScan(HANDLE hProcess, HMODULE hModule, char* szSignature, char* szMask);

/* Searches internally accessible memory space for a pattern of bytes
Args
  pMemory     - pointer to memory accessible 
  iMemorySize - Length of search space in bytes
  pSignature, szMask as explained in external signature scan above

Return Value
  Pointer to first memory address found which matches signature and mask  */
void* SignatureScan(BYTE* pMemory, int iMemorySize, char* szSignature, char* szMask);

/* Compares a given signature and mask against given data pointer, returns TRUE
   if a match is found. See SignatureScan for Signature/Mask behaviour */
bool CompareSignature(BYTE* pData, BYTE* pSignature, char* pMask);

// Get size in bytes of an external process' module
DWORD GetModuleSize(HANDLE hProcess, HMODULE hModule);

enum EModuleResult {
	MODULE_ERROR,
	MODULE_NOT_FOUND,
	MODULE_SUCCESS
};

// Returns a module to a process by filename in arg _out_module, which must be 
// closed with CloseHandle(). returns EModuleResult indicating result
EModuleResult GetProcessHandleFromFileName(char* Name, HANDLE& _out_module);

// Get a module handle to an external process in _out_module, which doesn't need 
// to be closed. returns EModuleResult indicating result
EModuleResult GetExternalModule(HANDLE hProcess, char* szName, HMODULE& _out_module);

#endif