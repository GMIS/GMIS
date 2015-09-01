// ZSIR.cpp : Defines the entry point for the application.
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

tstring       AfxCurrentDir;
tstring       AfxTempDir;

CMainBrain* GetBrain(){
	return AfxBrain;
}
CMainFrame* GetGUI(){
	return AfxGUI;
}



tstring GetCurrentDir(){
	return AfxCurrentDir;
};

tstring GetTempDir(){
	return AfxTempDir;
};

void Calculate(int dx, int dy, double& a1, double& a2)
{
	const double pi = 3.14159;
	double x = 0.09;
	double y = 0.05;

	x +=dx;
	y +=dy;

	double m_L1 = 0.15;
	double m_L2 = 0.22;
	double  n = (-(x*x +y*y)+m_L1*m_L1+m_L2*m_L2)/(2*m_L1*m_L2);
	assert(n>=-1.0f && n<=1.0f);
	a2 = pi - acos(n);

	a1 = atan(y/x) -atan(m_L2*sin(a2)/(m_L1+m_L2*cos(a2)));

	a1 = a1*180/pi;
	a2 = a2*180/pi;
}

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
	#pragma comment(linker, "/subsystem:console /ENTRY:mainCRTStartup") //不显示控制台窗口
	int main(int argc, TCHAR* argv[])

#endif

{

	HANDLE  MutexOneInstance = ::CreateMutexW( NULL, FALSE,
	  _T("AFX_MAINFRAME_H__C42A272C_3DC1_419E_B79A_76656F53E79C"));

    bool AlreadyRunning = ( ::GetLastError() == ERROR_ALREADY_EXISTS || 
                       ::GetLastError() == ERROR_ACCESS_DENIED);
    // The call fails with ERROR_ACCESS_DENIED if the Mutex was 
    // created in a different users session because of passing
    // NULL for the SECURITY_ATTRIBUTES on Mutex creation);

    if ( AlreadyRunning ){
/*
#ifdef USING_GUI		
		HWND hWnd = NULL;
		EnumWindows(FindOtherInstance, (LPARAM)&hWnd);
		
		if (hWnd)
		{	
			::SendMessage(hWnd, WM_ROCK_TDL_SHOWWINDOW, 0, 0);
			::SetForegroundWindow(hWnd);
		}else{
			::MessageBox(NULL,_T("Warning"),_T("Task has been exist,but not find the win"),MB_OK);
		}	
#endif
		return 0;
*/
	};

    tstring AppName = _T("GMIS(Brain)");
 
#ifdef USING_GUI

	INITCOMMONCONTROLSEX ics;
	ics.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ics.dwICC  = ICC_INTERNET_CLASSES;
    InitCommonControlsEx(&ics);

	if (AfxInitRichEditEx()==FALSE)
	{
		return 0;
	}

	CWinSpace2::RegisterCommonClass(hInstance);
	
	CMouseWheelMgr::Initialize();

	CUserMutex  UIMutex;

	UIMutex.Acquire();
	UIMutex.Release();

	CWinSpace2::InitUIMutex(&UIMutex);
	

	//先初始化本地界面
	CMainFrame MainFrame;
    AfxGUI   = &MainFrame;
	
	RECT rc;
	::SetRect(&rc,200,200,940,740);
	if(!MainFrame.Create(hInstance,AppName.c_str(),WS_CLIPCHILDREN,rc,NULL)){
		int n = ::GetLastError();
		return 0;
	}
	
	HICON hIcon = ::LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MAINFRAME));
    
	int32 n = GetLastError();
	
	assert(hIcon != NULL);
	::SendMessage(MainFrame.GetHwnd(), WM_SETICON, TRUE, (LPARAM)hIcon);
	::SendMessage(MainFrame.GetHwnd(), WM_SETICON, FALSE, (LPARAM)hIcon);
	
	MainFrame.SetIcon(hIcon);
    MainFrame.SetTitle(AppName.c_str());
		
	CenterWindow(MainFrame.GetHwnd(),NULL);
	::ShowWindow(MainFrame.GetHwnd(), SW_SHOW);
    ::UpdateWindow(MainFrame.GetHwnd());
	
	MainFrame.InitSplitterPos(150);

#endif

	//得到文件夹位置
	TCHAR buf[512];
	::GetModuleFileNameW(   
		hInstance,         
		buf,       
		512                 
		);   
	
	//奇怪，直接使用 s+="\\" 居然不行,也不能m_CurrentDir = s +"\\"
	AfxCurrentDir = buf;
	int pos = AfxCurrentDir.find_last_of(_T("\\"));
	AfxCurrentDir=AfxCurrentDir.substr(0,pos+1);

	AfxTempDir = AfxCurrentDir+_T("Temp\\");
	_tmkdir(AfxTempDir.c_str());

	
	if(!SS.Init(NULL))return 0;  //必须在主窗口生成之前初始化

	//身份验证
	
	CBrainMemory Memory;
    
	Memory.Open();
	
	AnsiString Cryptograph = Memory.GetSystemItem(SYS_ACCOUNT);
	AnsiString IP          = Memory.GetSystemItem(SYS_WORLDIP);
	bool passok = false;
	

	//初始化大脑
	static CUserSpacePool     SpacePool; //必须是静态的，确保是最后被析构

	static CUserTimer         Timer;
	
    //Model init part	
	static CUserMutex       ModelListMutex; 
	static CUserMutex		CentralNerveMutex;
	CLockPipe		CentralNerve(&CentralNerveMutex,_T("CentralNerve"),1);

	CUserMutex              ModelDataMutex;	
	Model::CLockedModelData ModelData(&ModelDataMutex);
	
	//System init part
	CUserMutex      SystemListMutex;
	CSpaceMutex     ClientSitMutex;	
	CUserMutex      NerveMutex;
	CLockPipe		Nerve(&NerveMutex,_T("Nerve"),2);

	CUserMutex                         SystemDataMutex;
    System::CLockedSystemData          SystemData(&SystemDataMutex);

	System::CSystemInitData InitData;
	InitData.m_Timer = &Timer;
	InitData.m_Pool  = &SpacePool;
	InitData.m_CentralNerve   = &CentralNerve;
	InitData.m_ModelData      = &ModelData;
	InitData.m_ModelListMutex = &ModelListMutex;
	InitData.m_Name           = _T("Frame");
    InitData.m_nCPU           = 2;
	InitData.m_ClientSitMutex = &ClientSitMutex;
	InitData.m_Nerve          = &Nerve;
	InitData.m_SystemData     = &SystemData;
	InitData.m_SystemListMutex= &SystemListMutex;


	CMainBrain MainBrain(&InitData);
	AfxBrain = &MainBrain;

	MainBrain.SetName(AppName);

//  test timestamp
//	int64 t = AbstractSpace::CreateTimeStamp();
//	tstring st = AbstractSpace::GetTimer()->GetFullTime(t);


	_ACCOUNT& Account = *MainBrain.GetAccount(); 
	Account.CrypText = _T("");
	Account.ip = atoint64(IP.c_str());

	//	char szDataOut[255] ;
	// 	char szHex[512];		
	if(Cryptograph.size()==0){ //如果是第一次则要求生成账号，否则输入账号
		int i=0;		
		int ret = DialogBoxParam (hInstance,MAKEINTRESOURCE(IDD_SETACCOUNT), 
			MainFrame.GetHwnd(), SetPasswordDlgProc, 0);
		if(ret>0){
			
			try{
				
                tstring s = Account.Name + Account.Password;
				AnsiString CrypStr = WStoUTF8(s);
			    AnsiString ipstr;
				char buf[100];
				sprintf(buf,"%I64ld",Account.ip);
				ipstr = buf;

				Memory.SetSystemItem(SYS_ACCOUNT,CrypStr);
				Memory.SetSystemItem(SYS_WORLDIP,ipstr);
				
				//AfxAccount.Password = "";
				Account.CrypText = s;
				passok = true;
			}catch(exception& roException){
				tstring s = Format1024(_T("Software Exception: %s "),roException.what());
				::MessageBox(NULL,_T("Warning"),s.c_str(),MB_OK);
			}
			catch (...) {
				::MessageBox(NULL,_T("Warning"),_T("Software Exception: Unkown error"),MB_OK);
            }
			
		}
		
	}else{ //账号第一次被存储在MemoryDB的SYSTEM项目下
		tstring CryptoStr = UTF8toWS(Cryptograph);
		int i=0;
		while(i++<3){

			int ret = ::DialogBoxParam (hInstance,MAKEINTRESOURCE(IDD_PASSWORD), 
				MainFrame.GetHwnd(), GetPasswordDlgProc,0);
			if(ret==0)break;
			try
            {                
				tstring NewCryptograph = Account.Name + Account.Password;
				
				if(CryptoStr == NewCryptograph){	
					Account.CrypText = CryptoStr;

					AnsiString ipstr;

					char buf[100];
					sprintf(buf,"%I64ld",Account.ip);
					ipstr = buf;

					Memory.SetSystemItem(SYS_WORLDIP,ipstr);
					passok = true;
					break;
				}
            }catch(exception& roException){
#ifdef _UNICODE
				AnsiString s = roException.what();
				tstring  ws = UTF8toWS(s);
				::MessageBox(NULL,_T("Warning"),ws.c_str(),MB_OK);
#else
				::MessageBox(NULL,_T("Warning"),roException.what(),MB_OK);
#endif
			}catch (...) {
				MainFrame.m_Status.SetTip(_T("Error: input password again"));
			}
			
		}		
	};

	Account.Password = _T("");
	//	Account.Name = AppName;
	//	Account.CrypText = AppName;

	AnsiString DialogInfo = CBrainMemory::GetSystemItem(SYS_DIALOG);

	Memory.Close();

	if (!passok)
	{
		return 0;
	}

	//DialogInfo=""; //仅用于测试时忽略

	if (DialogInfo.size())
	{
		ePipeline Pipe;
		uint32 pos=0;
		bool ret = Pipe.FromString(DialogInfo,pos);
		if (ret)
		{
			try
			{
				bool ret = MainBrain.FromEnergy(&Pipe);
				if(!ret){
					::MessageBox(NULL,_T("逻辑重入未能成功"),_T("Warning"),MB_OK);		
				};
			}
			catch (...)
			{
				::MessageBox(NULL,_T("逻辑重入发生异常，可能数据损坏"),_T("Warning"),MB_OK);	
			}
		}
	}


	if(!MainBrain.Activation()){
		if (MainBrain.m_ErrorInfo.size()==0)
		{
			MainBrain.m_ErrorInfo = _T("未知原因");
		}
		::MessageBox(NULL,MainBrain.m_ErrorInfo.c_str(),_T("Activation fail"),MB_OK);	
	    PostQuitMessage(0);
	}



#ifdef USING_GUI
	//界面循环
	MSG msg;
	while (1 && MainBrain.IsAlive()) 
	{		

		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)){ 
			if(GetMessage(&msg, NULL, 0, 0)<1)break;
			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			
		}else{
			SLEEP_MILLI(1);
		}
		
	}
    MainBrain.Dead();

#else 
    MainBrain.Do(NULL);
	MainBrain.Dead();

#endif

#ifdef SERVER_APP
	if(MutexOneInstance)CloseHandle(MutexOneInstance);
#endif
	return 0;
}



BOOL CALLBACK FindOtherInstance(HWND hwnd, LPARAM lParam)
{
	
	int nLen = ::GetWindowTextLength(hwnd);
	TCHAR* buf = new TCHAR[nLen+1];
	
	::GetWindowText(hwnd, buf, nLen + 1);
	
	_tcslwr(buf);
    
	if (_tcspbrk(buf,_T("rock")) != NULL)
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
			_ACCOUNT* ac = GetBrain()->GetAccount(); 
			HWND hIP   = GetDlgItem(hDlg,IDC_SETPASSWORD);
			::SendMessage(hIP,IPM_SETADDRESS,0,MAKEIPADDRESS(127,0,0,1));
			::SetFocus(GetDlgItem(hDlg,IDC_SETNAME));
			return TRUE;
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) 
		{
			_ACCOUNT* ac = GetBrain()->GetAccount(); 
			HWND hName = GetDlgItem(hDlg,IDC_SETNAME);
			HWND hPassword = GetDlgItem(hDlg,IDC_SETPASSWORD);
			HWND hConfirm  = GetDlgItem(hDlg,IDC_SETCONFIRM);
			HWND hIP       = GetDlgItem(hDlg,IDC_OUTER_IP);
			TCHAR buf[100];
			::GetWindowText(hName,buf,99);
			ac->Name = buf;
			::GetWindowText(hPassword,buf,99);
			ac->Password = buf;
			::GetWindowText(hConfirm,buf,99);
			ac->Confirm = buf;
			DWORD dwAddress;
			::SendMessage(hIP,IPM_GETADDRESS,0,(LPARAM)&dwAddress);
			IP ip;
			ip.seg1 = FIRST_IPADDRESS(dwAddress);
		    ip.seg2 = SECOND_IPADDRESS(dwAddress);
			ip.seg3 = THIRD_IPADDRESS(dwAddress);
			ip.seg4 = FOURTH_IPADDRESS(dwAddress);
			
			ac->ip = ip.GetUint32();

			if(ac->Name.size()>0 && ac->Confirm.size()>0 && ac->Confirm == ac->Password){
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
			_ACCOUNT* ac =GetBrain()->GetAccount(); 
	        IP ip((uint32)ac->ip);
			HWND hIP   = GetDlgItem(hDlg,IDC_EDITPASSWORD);
			::SendMessage(hIP,IPM_SETADDRESS,0,MAKEIPADDRESS(ip.seg1,ip.seg2,ip.seg3,ip.seg4));
			::SetFocus(GetDlgItem(hDlg,IDC_SETNAME));
			return TRUE;
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK ) 
		{
			_ACCOUNT* ac = GetBrain()->GetAccount(); 
			HWND hName = GetDlgItem(hDlg,IDC_SETNAME);
			HWND hPassword = GetDlgItem(hDlg,IDC_EDITPASSWORD);
			HWND hIP       = GetDlgItem(hDlg,IDC_OUTER_IP);
			TCHAR buf[100];
			::GetWindowText(hName,buf,99);
			ac->Name = buf;
			::GetWindowText(hPassword,buf,99);
			ac->Password = buf;
			DWORD dwAddress;
			::SendMessage(hIP,IPM_GETADDRESS,0,(LPARAM)&dwAddress);

			IP ip;
			ip.seg1 = FIRST_IPADDRESS(dwAddress);
			ip.seg2 = SECOND_IPADDRESS(dwAddress);
			ip.seg3 = THIRD_IPADDRESS(dwAddress);
			ip.seg4 = FOURTH_IPADDRESS(dwAddress);
			
			ac->ip = ip.GetUint32();

			if(ac->Name.size()>0 && ac->Password.size()>0 ){
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
