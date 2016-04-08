// LogViewer.cpp : Defines the entry point for the application.
//
#pragma warning (disable:4786)

//#include "vld.h"

#include "MainFrame.h"
#include <commctrl.h>
#include "resource.h"
#include "UserTimeStamp.h"
#include "UserMutex.h"
#include "mousewheelMgr.h"
#include "MyObject.h"
#include "format.h"
#pragma comment(lib, "comctl32.lib")

#ifdef _WINDLL

#define _LinkDLL __declspec(dllexport)
/*
使用方法：
  - 首先你在MyObject.h中用一个类来封装自己的任务。
  - 修改下面的导出函数GetObject，new一个自己设计的类。类的名字并不重要，
    如果你愿意，你甚至直接使用MyObject作为类名，那么这步都省了。

*/
//定义导出函数指针

extern "C"  _LinkDLL Mass*  _cdecl CreateObject(tstring Name,int64 ID=0){
    return new MyObject(Name,ID);
}
  
extern "C"  _LinkDLL void _cdecl  DestroyObject(Mass* p){
	delete p;
	p = NULL;
}

//这个函数给出一个物体最简单的使用说明
extern "C"  _LinkDLL const TCHAR*   GetObjectDoc(){
	static const TCHAR* Text = __TEXT(\
		"\
		1 命令：玩一局\n\
		功能：等待发牌，直到弃牌或有输赢后返回\n\
		输入数据：无\n\
		输出数据：无\n\
		2 命令：刷新\n\
		功能：刷新当前网页\n\
		输入数据：无\n\
		输出数据：无\n\
		3 命令：输出提示\n\
		功能：在结果窗口输出一个提示信息\n\
		输入数据：一个字符串\n\
		输出数据：无\n\
		4 命令：打开网址\n\
		功能：打开指定网址\n\
		输入数据：一个字符串\n\
		输出数据：无\n\
		5 命令：点击标签\n\
		功能：单击指定标签\n\
		输入数据：字符串\n\
		输出数据：无\n\
		6 命令：双击标签\n\
		功能：双击指定标签\n\
		输入数据：字符串\n\
		输出数据：无\n\
		7 命令：点击位置\n\
		功能：点击指定坐标点\n\
		输入数据：两个整数，分别表示x和y值\n\
		输出数据：无\n\
		8 命令：登录\n\
		功能：根据帐号和密码自动登录\n\
		输入数据：两个字符串，分别表示账号和密码\n\
		输出数据：无\n\
		9 命令：寻找标签\n\
		功能：在当前网页中查找是否有指定标签\n\
		输入数据：一个字符串表示标签名字\n\
		输出数据：一个整数，1表示有，0表示无\n\
		10 命令：输入字符串\n\
		功能：向当前网页的焦点输入字符串\n\
		输入数据：一个字符串，表示待输入的内容\n\
		输出数据：无\n\
		11 命令：得到房间号 \n\
		功能：得到当前场景的房间号\n\
		输入数据：无\n\
		输出数据：一个字符串，表示当前房间号\n\
		12 命令：是否有座位\n\
		功能：检测当前游戏房间是否有空的座位\n\
		输入数据：无\n\
		输出数据：一个整数，1表示有，0表示无\n\
		13 命令：得到总分 \n\
		功能：返回当前拥有的游戏总分\n\
		输入数据：无\n\
		输出数据：一个整数，表示游戏总分\n\
		14 命令：得到座位 \n\
		功能：返回当前的座位号[1-9]\n\
		输入数据：无\n\
		输出数据：一个整数，表示座位号\n\
		15 命令：得到盲注 \n\
		功能：返回当前的小盲注值\n\
		输入数据：无\n\
		输出数据：一个整数，表示盲注大小\n\
		16 命令：得到底牌\n\
		功能：返回当前手上的牌\n\
		输入数据：无\n\
		输出数据：两个整数，分表表示手上两张牌，每个整数/100的为花色[0-3分别表示红桃梅花方片黑桃]，整数%100为牌号[2：2-14：A]\n\
		17 命令：得到公牌 \n\
		功能：返回当前的公牌\n\
		输入数据：无\n\
		输出数据：多个整数，分别表示多张牌\n\
		18 命令：得到跟注\n\
		功能：返回当前系统要求的跟注值\n\
		输入数据：无\n\
		输出数据：一个整数，表示跟注大小\n\
		19 命令：暂停\n\
		功能：暂停当前玩牌\n\
		输入数据：无\n\
		输出数据：无\n\
		20 命令：停止\n\
		功能：停止当前玩牌\n\
		输入数据：无\n\
		输出数据：无\n\
		21 命令：运行\n\
		功能：开始当前玩牌\n\
		输入数据：无\n\
		输出数据：无\n\
		22 命令：清除历史\n\
		功能：清除cookies,避免打开网址时自动登录旧帐号\n\
		输入数据：无\n\
		输出数据：无\n\
		"\
		);

	return Text;
}

extern "C"  _LinkDLL DLL_TYPE  GetDllType(){
//不同的返回值帮助决定不同的C运行时库
#ifdef _DEBUG

#if _MSC_VER<1300
	return DLL_VC6D;
#elif _MSC_VER<1400
	return DLL_VC7D;
#elif _MSC_VER<1500
	return DLL_VC8D;
#elif _MSC_VER<1600
	return DLL_VC9D;
#elif _MSC_VER<1700
	return DLL_VC10D;
#endif

#else

#if _MSC_VER<1300
	return DLL_VC6;
#elif _MSC_VER<1400
	return DLL_VC7;
#elif _MSC_VER<1500
	return DLL_VC8;
#elif _MSC_VER<1600
	return DLL_VC9;
#elif _MSC_VER<1700
	return DLL_VC10;
#endif

#endif
} 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x0400
#include <objbase.h>
#endif


HANDLE hDllModule = NULL;
HANDLE GetDLLHandle(){
	return hDllModule;	
};
BOOL APIENTRY DllMain( HANDLE hModule, 
	DWORD  ul_reason_for_call, 
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hDllModule = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		break;
	}
	return TRUE;
}


#else

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	static CUserTimeStamp     Timer;  
	static CUserMutex         SpacePoolMutex;  //声明为static，确保最后被释放
	static CAbstractSpacePool SpacePool(&SpacePoolMutex);


	INITCOMMONCONTROLSEX ics;
	ics.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ics.dwICC  = ICC_INTERNET_CLASSES;
    InitCommonControlsEx(&ics);


	if (OleInitialize(NULL) != S_OK){
		return 0;
	}

	if (AfxInitRichEditEx()==FALSE)
	{
		return 0;
	}

	CWinSpace2::RegisterCommonClass(hInstance);
	
	CUserMutex  UIMutex;
	CWinSpace2::InitUIMutex(&UIMutex);
	

	CMouseWheelMgr::Initialize();
		
	CMainFrame Frame(&Timer,&SpacePool);

	ePipeline Param;
	Param.PushString(_T("http://apps.renren.com/boyaa_texas/index.php?origin=103"));
	

	char* token;
	char* str = lpCmdLine;
	char* flag = " ";
	token = strtok(str,flag);
	while(token)
	{
		AnsiString s = token;
		Param.PushString(s);
		token = strtok(NULL,flag);
	}

#ifdef _DEBUG
    Param.PushString(_T("hjjehpch_d1554@163.com"));
	Param.PushString(_T("af7r4fh76e0"));

	//Param.PushString(_T("新手场"));
	//Param.PushString(_T("110"));
#endif

	Frame.m_UserAccount<<Param;

    RECT rc;
	::SetRect(&rc,100,150,700,650);
	if(!Frame.Create(hInstance,_T("Poker"),WS_CLIPCHILDREN,rc,NULL))return 0;
	HICON hIcon = ::LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MAINFRAME));
	CResultView::ImageItem::hPokerImage = ::LoadImage(hInstance,MAKEINTRESOURCE(IDB_POKERBITMAP),IMAGE_BITMAP,0,0,0);
   
	assert(hIcon != NULL);
	::SendMessage(Frame.GetHwnd(), WM_SETICON, TRUE, (LPARAM)hIcon);
	::SendMessage(Frame.GetHwnd(), WM_SETICON, FALSE, (LPARAM)hIcon);
	
	Frame.SetIcon(hIcon);

#ifdef LOCAL_USE
	Frame.SetTitle(_T("Poker"));	
#else
    Frame.SetTitle(_T("Poker"));	
#endif	

	//	CenterWindow(Frame.GetHwnd(),NULL);
	::ShowWindow(Frame.GetHwnd(), SW_SHOW);
    ::UpdateWindow(Frame.GetHwnd());
	
	Frame.InitSplitterPos(150);

//	Frame.Activation();

	MSG msg;
	// Main message loop:
	while(GetMessage(&msg, NULL, 0, 0)) 
	{
		//if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}		
	}
	
	OleUninitialize();

	Frame.Dead();
	return 0;
}

#endif

