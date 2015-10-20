﻿// ZSIR.cpp : Defines the entry point for the application.
//
#pragma warning (disable:4786)

#include "GMIS.h"
#include <commctrl.h>
#include "resource.h"
#include "UserTimer.h"
#include "SpaceMutex.h"
#include "ConvertUTF.h"
#include "UserSpacePool.h"
#include "SystemSetting.h"
#include "mousewheelMgr.h"
#include <tchar.h>
#include "Win32Tool.h"
//#include "vld.h"
#include "Ipclass.h"
#include <math.h>

#include <WinBase.h>

CMainBrain*  AfxBrain = NULL;
CMainFrame*  AfxGUI   = NULL;

CMainBrain* GetBrain(){
	return AfxBrain;
}
CMainFrame* GetGUI(){
	return AfxGUI;
}

struct _ACCOUNT{
	tstring Name;
	tstring Password;
	tstring Confirm;
};

_ACCOUNT  TempAccount;

tstring GetCurrentDir(){
	static tstring AfxCurrentDir;
	if(AfxCurrentDir.size())return AfxCurrentDir;

	TCHAR buf[512];
	::GetModuleFileNameW(   
		NULL,         
		buf,       
		512                 
		);   
	AfxCurrentDir = buf;
	int pos = AfxCurrentDir.find_last_of(_T("\\"));
	AfxCurrentDir=AfxCurrentDir.substr(0,pos+1);

	return AfxCurrentDir;
};

tstring GetTempDir(){
	static tstring AfxTempDir;
	if (AfxTempDir.size())return AfxTempDir;

	tstring CurrentDir = GetCurrentDir();
	AfxTempDir = CurrentDir+_T("Temp\\");
	
	_tmkdir(AfxTempDir.c_str());
	return AfxTempDir;
};


#ifdef  USING_GUI

#include "ThinDataTransProtocal.h"

#pragma comment(lib, "comctl32.lib")

BOOL CALLBACK FindOtherInstance(HWND hwnd, LPARAM lParam);
BOOL CALLBACK SetPasswordDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK GetPasswordDlgProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)

#else
	#pragma comment(linker, "/subsystem:console /ENTRY:mainCRTStartup") //控制台窗口
	int main(int argc, TCHAR* argv[])

#endif

{

	HANDLE  MutexOneInstance = ::CreateMutexW( NULL, FALSE,
		_T("AFX_GMIS_BRAIN__C42A272C_3DC1_419E_B79A_76656F53E79C"));

	bool AlreadyRunning = ( ::GetLastError() == ERROR_ALREADY_EXISTS || 
		::GetLastError() == ERROR_ACCESS_DENIED);

	if ( AlreadyRunning ){

		HWND hWnd = NULL;
		EnumWindows(FindOtherInstance, (LPARAM)&hWnd);	
		if (hWnd)
		{	
			::SendMessage(hWnd, WM_ROCK_TDL_SHOWWINDOW, 0, 0);
			::SetForegroundWindow(hWnd);
		}else{
			::MessageBox(NULL,_T("Warning"),_T("Task has been exist"),MB_OK);
		}	
		return 0;

	};

	

	//启动大脑
	try
	{

		tstring AppName =_T("GMIS(Local)");

		//初始化大脑
		static CUserSpacePool   SpacePool; //必须是静态的，确保是最后被析构
		static CUserTimer       Timer;

		int64  t = Timer.TimeStamp();
		tstring w = Timer.GetFullTime(t);
		tstring w1 = Timer.GetHMS(t);

		static CMainBrain MainBrain(&Timer,&SpacePool,AppName);
		AfxBrain = &MainBrain;

		if(!MainBrain.Activation()){
			if (MainBrain.m_ErrorInfo.size()==0)
			{
				MainBrain.m_ErrorInfo = _T("未知原因");
			}
			::MessageBox(NULL,MainBrain.m_ErrorInfo.c_str(),_T("Activation fail"),MB_OK);
			return 0;
		}

#ifndef USING_GUI
		MainBrain.CheckMemory();
		//没有图形界面的消息循环，则自己保证循环
		MainBrain.Do(NULL);
#else 
		//字体，图标资源初始化		
		if(!SS.Init(NULL))return 0;  

		//如果使用本地用户界面，则相当于登录一个本地用户
		INITCOMMONCONTROLSEX ics;
		ics.dwSize = sizeof(INITCOMMONCONTROLSEX);
		ics.dwICC  = ICC_INTERNET_CLASSES;
		InitCommonControlsEx(&ics);

		if (AfxInitRichEditEx()==FALSE)
		{
			MainBrain.Dead();
			if(MutexOneInstance)CloseHandle(MutexOneInstance);
			return 0;
		}

		CWinSpace2::RegisterCommonClass(hInstance);
		CMouseWheelMgr::Initialize();

		CUserMutex  UIMutex;
		CWinSpace2::InitUIMutex(&UIMutex);


		//先初始化本地界面
		CMainFrame MainFrame;
		AfxGUI   = &MainFrame;

		RECT rc;
		::SetRect(&rc,200,200,940,740);
		if(!MainFrame.Create(hInstance,AppName.c_str(),WS_CLIPCHILDREN,rc,NULL)){
			int n = ::GetLastError();
			MainBrain.Dead();
			if(MutexOneInstance)CloseHandle(MutexOneInstance);
			return 0;
		}

		HICON hIcon = ::LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MAINFRAME));
		if(hIcon){
			::SendMessage(MainFrame.GetHwnd(), WM_SETICON, TRUE, (LPARAM)hIcon);
			::SendMessage(MainFrame.GetHwnd(), WM_SETICON, FALSE, (LPARAM)hIcon);
			MainFrame.SetIcon(hIcon);
		}

		MainFrame.SetTitle(AppName.c_str());

		CenterWindow(MainFrame.GetHwnd(),NULL);
		::ShowWindow(MainFrame.GetHwnd(), SW_SHOW);
		::UpdateWindow(MainFrame.GetHwnd());

		MainFrame.InitSplitterPos(150);

		//身份验证
		tstring CrypStr;
		if(GetBrain()->GetUserAccountNum()==0){ 
			//首次使用，要求注册一个

			int i=0;		
			int ret = DialogBoxParam (hInstance,MAKEINTRESOURCE(IDD_SETACCOUNT), 
				MainFrame.GetHwnd(), SetPasswordDlgProc, 0);
			if(ret==0){
				MainBrain.Dead();
				if(MutexOneInstance)CloseHandle(MutexOneInstance);
				return 0;
			}
			CrypStr = TempAccount.Name + TempAccount.Password;

			TempAccount.Password = _T("");
			GetBrain()->RegisterUserAccount(TempAccount.Name,CrypStr,DIALOG_SYSTEM_MAIN);

			ret = MainBrain.Login(0,TempAccount.Name,CrypStr);
			if(!ret){
				MainBrain.Dead();
				if(MutexOneInstance)CloseHandle(MutexOneInstance);
				return 0;
			}
			CrypStr = _T("");
			TempAccount.Password =_T("");

		}else{ 	
			int i=0;
			while(i++<3){

				int ret = ::DialogBoxParam (hInstance,MAKEINTRESOURCE(IDD_PASSWORD), 
					MainFrame.GetHwnd(), GetPasswordDlgProc,0);
				if(ret==0){
					MainBrain.Dead();
					if(MutexOneInstance)CloseHandle(MutexOneInstance);
					return 0;
				}
				CrypStr = TempAccount.Name + TempAccount.Password;
				
			    ret = MainBrain.Login(0,TempAccount.Name,CrypStr);
				if(ret){
					break;
				}
			}
	
			CrypStr = _T("");
			TempAccount.Password =_T("");
			if(i==3){
				MainBrain.Dead();
				if(MutexOneInstance)CloseHandle(MutexOneInstance);
				return 0;
			}
		}

		MainBrain.CheckMemory();

		//消息循环
		MSG msg;
		while (MainBrain.IsAlive()) 
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

	}
	catch (exception& roException)
	{
		tstring s = Format1024(_T("GMIS starting fail: %s "),roException.what());
		::MessageBox(NULL,_T("Sorry!"),s.c_str(),MB_OK);
	}catch (...)
	{
		tstring s = _T("GMIS starting fail: an exception occured");
		::MessageBox(NULL,_T("Sorry!"),s.c_str(),MB_OK);
	}
	
	GetBrain()->Dead();
	if(MutexOneInstance)CloseHandle(MutexOneInstance);
	return 0;
}



BOOL CALLBACK FindOtherInstance(HWND hwnd, LPARAM lParam)
{
	
	int nLen = ::GetWindowTextLength(hwnd);
	TCHAR* buf = new TCHAR[nLen+1];
	
	::GetWindowText(hwnd, buf, nLen + 1);
	
	_tcslwr(buf);
    
	if (_tcspbrk(buf,_T("GMIS")) != NULL)
	{
		HWND* pWnd = (HWND*)lParam;
		*pWnd = hwnd;
		delete buf;
		return FALSE;
	}
	delete buf;
	
	return TRUE;
}

BOOL CALLBACK SetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message)
	{
	case WM_INITDIALOG:
		{
			CenterWindow(hDlg,GetParent(hDlg));
			::SetFocus(GetDlgItem(hDlg,IDC_SETNAME));
			return TRUE;
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) 
		{

			HWND hName = GetDlgItem(hDlg,IDC_SETNAME);
			HWND hPassword = GetDlgItem(hDlg,IDC_SETPASSWORD);
			HWND hConfirm  = GetDlgItem(hDlg,IDC_SETCONFIRM);

			TCHAR buf[100];
			::GetWindowText(hName,buf,99);
			TempAccount.Name = buf;
			::GetWindowText(hPassword,buf,99);
			TempAccount.Password = buf;
			::GetWindowText(hConfirm,buf,99);
			TempAccount.Confirm = buf;
		
			if(TempAccount.Name.size()>0 && TempAccount.Confirm.size()>0 && TempAccount.Confirm == TempAccount.Password){
				::SetWindowText(hName,_T(""));
				::SetWindowText(hPassword,_T(""));		
				::SetWindowText(hConfirm,_T(""));
				EndDialog(hDlg, 1);
			}
			return TRUE;
		}else if(LOWORD(wParam) == IDCANCEL){
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
    return FALSE;
};

BOOL CALLBACK GetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	
	switch (message)
	{
	case WM_INITDIALOG:
		{
			CenterWindow(hDlg,GetParent(hDlg));	   
			return TRUE;
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) 
		{
			HWND hName = GetDlgItem(hDlg,IDC_SETNAME);
			HWND hPassword = GetDlgItem(hDlg,IDC_EDITPASSWORD);
			TCHAR buf[100];
			::GetWindowText(hName,buf,99);
			TempAccount.Name = buf;
			::GetWindowText(hPassword,buf,99);
			TempAccount.Password = buf;

			if(TempAccount.Name.size()>0 && TempAccount.Password.size()>0 ){
				::SetWindowText(hName,_T(""));
				::SetWindowText(hPassword,_T(""));
				EndDialog(hDlg, 1);
			}
			return TRUE;
		}else if(LOWORD(wParam) == IDCANCEL){
			EndDialog(hDlg, 0);
			return TRUE;
		}
	}
    return FALSE;
};