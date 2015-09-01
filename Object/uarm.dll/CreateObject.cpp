// CreateObject.cpp : Defines the entry point for the DLL application.
//

#pragma warning(disable: 4786)

#include "CreateObject.h"
#include <windows.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x0400
#include <objbase.h>
#else
#include <objbase.h>
#endif
  
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			break;
    }
    return TRUE;
}


