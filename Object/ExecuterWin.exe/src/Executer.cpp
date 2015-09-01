// Executer.cpp : Defines the entry point for the application.
//
#pragma warning(disable: 4786)

#include "ExecuterFrame.h"
#include <commctrl.h>
#include "resource.h"
#include "UserTimer.h"
#include "UserMutex.h"
#include "EditLog.h"
#include "editlog_stream.h"
#include "Resource.h"
#include "SpaceMsgList.h"
#include "MouseWheelMgr.h"

#define MAX_LOADSTRING 100

using namespace ABSTRACT;

#pragma comment(lib, "comctl32.lib")

// Global Variables:
HINSTANCE hInst;								// current instance

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	int64 ExecuterID = 0;

	ePipeline CmdPipe;
	char* token;
	char* str = (char*)lpCmdLine;
	char* flag = " ";
	token = strtok(str,flag);
	while(token)
	{
		AnsiString s = token;
		CmdPipe.PushString(s);
		token = strtok(NULL,flag);
	}

	if (CmdPipe.Size()==1)
	{
		tstring s = CmdPipe.PopString();

		ExecuterID = _ttoi64(s.c_str());
	    assert(ExecuterID);
	}


	INITCOMMONCONTROLSEX ics;
	ics.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ics.dwICC  = ICC_INTERNET_CLASSES;
    InitCommonControlsEx(&ics);
	

	CWinSpace2::RegisterCommonClass(hInstance);
	
	CUserMutex  UIMutex;
	CWinSpace2::InitUIMutex(&UIMutex);
	
	CMouseWheelMgr::Initialize();

	CUserMutex  SpacePoolMutex;
	CABSpacePool SpacePool(&SpacePoolMutex);
	
	CUserTimer  Timer;

	//实例一个Model
	tstring AppName = Format1024(_T("GMIS(Exe:%I64ld)"),ExecuterID);

    //Model init part	
	CUserMutex      ModelListMutex;
	CUserMutex		CentralNerveMutex;
	CLockPipe		CentralNerve(&CentralNerveMutex,_T("CentralNerve"),1);
	
	CUserMutex              ModelDataMutex;	
	Model::CLockedModelData ModelData(&ModelDataMutex);
	
	
	Model::CModelInitData InitData;
	InitData.m_Timer = &Timer;
	InitData.m_Pool  = &SpacePool;
	InitData.m_CentralNerve   = &CentralNerve;
	InitData.m_ModelData      = &ModelData;
	InitData.m_ModelListMutex = &ModelListMutex;
	InitData.m_Name           = AppName.c_str();
    InitData.m_nCPU           = 2;
	
	CMainFrame MainFrame(ExecuterID,&InitData);

    RECT rc;
	::SetRect(&rc,0,0,300,200);
	if(!MainFrame.Create(hInstance,_T("Executer"),WS_CLIPCHILDREN,rc,NULL))return 0;
		
	HICON hIcon = ::LoadIcon(hInstance,MAKEINTRESOURCE(IDR_MAINFRAME));
    
	assert(hIcon != NULL);
	::SendMessage(MainFrame.GetHwnd(), WM_SETICON, TRUE, (LPARAM)hIcon);
	::SendMessage(MainFrame.GetHwnd(), WM_SETICON, FALSE, (LPARAM)hIcon);

	MainFrame.SetIcon(hIcon);
    MainFrame.SetTitle(AppName.c_str());

	//CenterWindow(MainFrame.GetHwnd(),NULL);
	//::ShowWindow(MainFrame.GetHwnd(), SW_SHOW);
    //::UpdateWindow(MainFrame.GetHwnd());


	if(!MainFrame.Activation()){
		return 0;
	}

	MainFrame.OutputLog(LOG_TIP,_T("Activation oK"));

//测试
/*	
	tstring FileAddress =_T("d:\\project\\bin\\vc10\\debug\\PokerRobot\\PokerRobot.dll");
	CExObject* Ob = new CExObject(&MainFrame,0,0,FileAddress);
	ePipeline ExePipe;
	//Ob->Do(&ExePipe);
	//Ob->Dead();
	Ob->WaitForStopRun(50);
//    delete Ob;
*/	

	tstring ip = _T("127.0.0.1");
	CMsg Msg(MSG_CONNECT_TO,DEFAULT_DIALOG,0);
	ePipeline& Letter = Msg.GetLetter();
	Letter.PushInt(SYSTEM_SOURCE);
	Letter.PushString(ip);
	Letter.PushInt(SPACE_PORT);
	Letter.PushInt(5);  //等待5秒
	
	MainFrame.PushCentralNerveMsg(Msg);
	
	MSG msg;
	while (1) 
	{				
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)){ 
			if(GetMessage(&msg, NULL, 0, 0)<1)break;
			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			
		}else{
			SLEEP_MILLI(1);
		}
	}
    MainFrame.Dead();
	return 0;
}


