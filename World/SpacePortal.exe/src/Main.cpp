// SpacePortal.cpp : Defines the entry point for the application.
//

#pragma warning (disable:4786)

#include "resource.h"
#include "SpacePortal.h"
#include "Win32Tool.h"
#include "UserTimer.h"
#include "UserMutex.h"
#include "SpaceMutex.h"
#include "UserSpacePool.h"
#include <tchar.h>
#include <commctrl.h>

//#include "vld.h"
#pragma comment(lib, "comctl32.lib")

using namespace ABSTRACT;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	

    HANDLE  MutexOneInstance = ::CreateMutexW( NULL, FALSE,
	  _T("SPACEPORTAL_CREATED"));

    bool AlreadyRunning = ( ::GetLastError() == ERROR_ALREADY_EXISTS || 
                       ::GetLastError() == ERROR_ACCESS_DENIED);
    // The call fails with ERROR_ACCESS_DENIED if the Mutex was 
    // created in a different users session because of passing
    // NULL for the SECURITY_ATTRIBUTES on Mutex creation);

    if ( AlreadyRunning )return 0;
	
	ePipeline Param;
	char* token;
	char* str = (char*)lpCmdLine;
	char* flag = " ";
	token = strtok(str,flag);
	while(token)
	{
		AnsiString s = token;
		Param.PushString(s);
		token = strtok(NULL,flag);
	}

	INITCOMMONCONTROLSEX ics;
	ics.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ics.dwICC  = ICC_INTERNET_CLASSES;
    InitCommonControlsEx(&ics);
	     
	CWinSpace2::RegisterCommonClass(hInstance);
  	
	CUserMutex  UIMutex;
	CWinSpace2::InitUIMutex(&UIMutex);

	//Initialize
	static CUserSpacePool     SpacePool; //Must be static to guarantee it be destructed in the last
	static CUserTimer         Timer;
	
	People  Host; 
	GetHost(&Host);//initialize，due to the static variable  can not control the destructor order, in order to avoid Host destruct ahead of the WorldDB , Host to local variables

	ROOM_SPACE RootRoom;
	GetRootRoom(&RootRoom); //initialize


    //Model init part	
	CUserMutex      ModelListMutex;
	CUserMutex		CentralNerveMutex;
	CLockPipe		CentralNerve(&CentralNerveMutex,_T("CentralNerve"),1);
	
	CUserMutex              ModelDataMutex;	
	Model::CLockedModelData ModelData(&ModelDataMutex);
	
	//System init part
	CUserMutex      SystemListMutex;
	CSpaceMutex     ClientSitMutex;	
	CUserMutex      NerveMutex;
	CLockPipe		Nerve(&NerveMutex,_T("Nerve"),2);
	
	CUserMutex                         SystemDataMutex;
	System::CLockedSystemData SystemData(&SystemDataMutex);
	
	System::CSystemInitData InitData;
	InitData.m_Timer = &Timer;
	InitData.m_Pool  = &SpacePool;
	InitData.m_CentralNerve   = &CentralNerve;
	InitData.m_ModelData      = &ModelData;
	InitData.m_ModelListMutex = &ModelListMutex;
	InitData.m_Name           = _T("SpacePortal");
    InitData.m_nCPU           = 2;
	InitData.m_ClientSitMutex = &ClientSitMutex;
	InitData.m_Nerve          = &Nerve;
	InitData.m_SystemData     = &SystemData;
	InitData.m_SystemListMutex= &SystemListMutex;

	CSpacePortal MainFrame(&InitData);

	RECT rc;
	::SetRect(&rc,250,150,740,540);
	if(!MainFrame.Create(hInstance,_T("World"),WS_CLIPCHILDREN,rc,NULL)){
		int n = ::GetLastError();
		return 0;
	}


	MainFrame.m_CmdLine << Param;

	HICON hIcon = ::LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MYWORLD));
    
	assert(hIcon != NULL);
	::SendMessage(MainFrame.GetHwnd(), WM_SETICON, TRUE, (LPARAM)hIcon);
	::SendMessage(MainFrame.GetHwnd(), WM_SETICON, FALSE, (LPARAM)hIcon);

	MainFrame.SetIcon(hIcon);

	tstring Dir = GetCurrentDir();
    Dir += _T("\\World.db");


    MainFrame.SetTitle(Dir.c_str());

	//CenterWindow(MainFrame.GetHwnd(),NULL);
	::ShowWindow(MainFrame.GetHwnd(), SW_SHOW);
    ::UpdateWindow(MainFrame.GetHwnd());

	if(!MainFrame.Activation()){
	    PostQuitMessage(0);
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	Host.GoOut(); //initiative leave to avoid occuring error after the database has been shut down
	
    MainFrame.Dead();

	if(MutexOneInstance)CloseHandle(MutexOneInstance);

	return 0;
}



