#include "amanith_globals.h"
#if defined(AM_OS_WIN)

#include <Windows.h>
HINSTANCE dllInstance = 0;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		dllInstance = hinstDLL;
	}
	return TRUE;
}

#endif
