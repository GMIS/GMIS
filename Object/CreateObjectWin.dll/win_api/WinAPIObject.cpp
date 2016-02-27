// WinAPIObject.cpp: implementation of the WinAPIObject class.
//
//////////////////////////////////////////////////////////////////////

#include "WinAPIObject.h"


#if defined _COMPILE_WIMAPI_OBJECT

#include <assert.h>
#include <shellapi.h>

const TCHAR* CWinAPIObject::UserManual = __TEXT(\
		"\
		1 cmd：open\n\
		Function：打开一个程序或链接\n\
		Input：字符串，程序名或链接地址\n\
		Output：null\n\
		"\
		);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool CWinAPIObject::Do(Energy* E)
{
	assert(E->EnergyType() == TYPE_PIPELINE);
	ePipeline* Pipe = (ePipeline*)E;

	if (Pipe->Size()==0)
	{
		return false;
	}

	
    ENERGY_TYPE Type = Pipe->GetDataType(0);
	if (Type != TYPE_STRING)
	{
		return false;
	}
	tstring s = Pipe->PopString();
    
    HINSTANCE hHandle = ShellExecute(NULL,_T("open"), s.c_str(), NULL, NULL, SW_SHOWNORMAL);
	DWORD ret = (DWORD)hHandle;

    if(ret>32)return true;

	tstring Error;
	switch(ret){
	case 0:
		Error =_T("The operating system is out of memory or resources.");
		break;
	case ERROR_FILE_NOT_FOUND:
		Error = _T("The specified file was not found.");
		break;
	case ERROR_PATH_NOT_FOUND:
		Error = _T("The specified path was not found.");
		break;
	case ERROR_BAD_FORMAT:
		Error = _T("The .exe file is invalid (non-Win32 .exe or error in .exe image).");
		break;
	case SE_ERR_ACCESSDENIED:
		Error = _T("The operating system denied access to the specified file.");
		break;
	case SE_ERR_ASSOCINCOMPLETE:
		Error = _T("The file name association is incomplete or invalid.");
		break;
	case SE_ERR_DDEBUSY:
		Error = _T("The DDE transaction could not be completed because other DDE transactions were being processed.");
		break;
	case SE_ERR_DDEFAIL:
		Error = _T("The DDE transaction failed.");
		break;
	case SE_ERR_DDETIMEOUT:
		Error = _T("The DDE transaction could not be completed because the request timed out.");
		break;
	case SE_ERR_DLLNOTFOUND:
		Error = _T("The specified DLL was not found.");
		break;
	case SE_ERR_NOASSOC:
		Error = _T("There is no application associated with the given file name extension. This error will also be returned if you attempt to print a file that is not printable.");
		break;
	case SE_ERR_OOM:
		Error = _T("There was not enough memory to complete the operation.");
		break;
	case SE_ERR_SHARE:
		Error = _T("A sharing violation occurred.");
		break;
	}
	Pipe->SetLabel(Error.c_str());
//	Pipe->SetID(RETURN_ERROR);
	return false;
}
#endif //_COMPILE_WIMAPI_OBJECT