// Executer.cpp : Defines the entry point for the application.
//
#pragma warning(disable: 4786)

#include "MainFrame.h"
#include "Executer.h"
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

#define USING_GUI 

using namespace ABSTRACT;

#pragma comment(lib, "comctl32.lib")


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{


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

	if (CmdPipe.Size()==0)
	{
		CmdPipe.PushString(_T("executer"));
		CmdPipe.PushString(_T("101"));
	}

	assert(CmdPipe.Size()==2);
	if (CmdPipe.Size()!=2)return 0;

	
	tstring CryptText = CmdPipe.PopString();

    tstring s = CmdPipe.PopString();
	int32 DllType = _ttoi(s.c_str());
	assert(DllType);
	


	INITCOMMONCONTROLSEX ics;
	ics.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ics.dwICC  = ICC_INTERNET_CLASSES;
    InitCommonControlsEx(&ics);
	

	CWinSpace2::RegisterCommonClass(hInstance);
	
	static CUserMutex  UIMutex;
	CWinSpace2::InitUIMutex(&UIMutex);
	
	CMouseWheelMgr::Initialize();

	static CUserSpacePool SpacePool;	
	static CUserTimer  Timer;

	//实例一个Model
	tstring AppName = Format1024(_T("Executer(Type:%d)"),DllType);

	CExecuter  Executer(DllType,CryptText,&Timer,&SpacePool);

	if(!Executer.Activate()){
		return 0;
	}

	Executer.OutputLog(LOG_TIP,_T("Activation oK"));

	tstring ip = _T("127.0.0.1");
	CMsg Msg(MSG_CONNECT_TO,DEFAULT_DIALOG,0);
	ePipeline& Letter = Msg.GetLetter();
	Letter.PushInt(SYSTEM_SOURCE);
	Letter.PushString(ip);
	Letter.PushInt(SPACE_PORT);
	Letter.PushInt(5);  //等待5秒

	Executer.PushCentralNerveMsg(Msg,false,true);

#ifndef USING_GUI

	//没有图形界面的消息循环，则自己保证循环
	while(Executer.IsAlive()){
		Sleep(200);
	}
#else 
	CMainFrame MainFrame;

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
	::ShowWindow(MainFrame.GetHwnd(), SW_SHOW);
    ::UpdateWindow(MainFrame.GetHwnd());


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
	
	MSG msg;
	while (Executer.IsAlive()) 
	{				
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)){ 
			if(GetMessage(&msg, NULL, 0, 0)<1)break;
			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			
		}else{
			SLEEP_MILLI(1);
		}
	}
#endif

	Executer.Dead();
	return 0;
}


