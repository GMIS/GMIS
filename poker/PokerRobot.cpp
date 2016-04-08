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
ʹ�÷�����
  - ��������MyObject.h����һ��������װ�Լ�������
  - �޸�����ĵ�������GetObject��newһ���Լ���Ƶ��ࡣ������ֲ�����Ҫ��
    �����Ը�⣬������ֱ��ʹ��MyObject��Ϊ��������ô�ⲽ��ʡ�ˡ�

*/
//���嵼������ָ��

extern "C"  _LinkDLL Mass*  _cdecl CreateObject(tstring Name,int64 ID=0){
    return new MyObject(Name,ID);
}
  
extern "C"  _LinkDLL void _cdecl  DestroyObject(Mass* p){
	delete p;
	p = NULL;
}

//�����������һ��������򵥵�ʹ��˵��
extern "C"  _LinkDLL const TCHAR*   GetObjectDoc(){
	static const TCHAR* Text = __TEXT(\
		"\
		1 �����һ��\n\
		���ܣ��ȴ����ƣ�ֱ�����ƻ�����Ӯ�󷵻�\n\
		�������ݣ���\n\
		������ݣ���\n\
		2 ���ˢ��\n\
		���ܣ�ˢ�µ�ǰ��ҳ\n\
		�������ݣ���\n\
		������ݣ���\n\
		3 ��������ʾ\n\
		���ܣ��ڽ���������һ����ʾ��Ϣ\n\
		�������ݣ�һ���ַ���\n\
		������ݣ���\n\
		4 �������ַ\n\
		���ܣ���ָ����ַ\n\
		�������ݣ�һ���ַ���\n\
		������ݣ���\n\
		5 ��������ǩ\n\
		���ܣ�����ָ����ǩ\n\
		�������ݣ��ַ���\n\
		������ݣ���\n\
		6 ���˫����ǩ\n\
		���ܣ�˫��ָ����ǩ\n\
		�������ݣ��ַ���\n\
		������ݣ���\n\
		7 ������λ��\n\
		���ܣ����ָ�������\n\
		�������ݣ������������ֱ��ʾx��yֵ\n\
		������ݣ���\n\
		8 �����¼\n\
		���ܣ������ʺź������Զ���¼\n\
		�������ݣ������ַ������ֱ��ʾ�˺ź�����\n\
		������ݣ���\n\
		9 ���Ѱ�ұ�ǩ\n\
		���ܣ��ڵ�ǰ��ҳ�в����Ƿ���ָ����ǩ\n\
		�������ݣ�һ���ַ�����ʾ��ǩ����\n\
		������ݣ�һ��������1��ʾ�У�0��ʾ��\n\
		10 ��������ַ���\n\
		���ܣ���ǰ��ҳ�Ľ��������ַ���\n\
		�������ݣ�һ���ַ�������ʾ�����������\n\
		������ݣ���\n\
		11 ����õ������ \n\
		���ܣ��õ���ǰ�����ķ����\n\
		�������ݣ���\n\
		������ݣ�һ���ַ�������ʾ��ǰ�����\n\
		12 ����Ƿ�����λ\n\
		���ܣ���⵱ǰ��Ϸ�����Ƿ��пյ���λ\n\
		�������ݣ���\n\
		������ݣ�һ��������1��ʾ�У�0��ʾ��\n\
		13 ����õ��ܷ� \n\
		���ܣ����ص�ǰӵ�е���Ϸ�ܷ�\n\
		�������ݣ���\n\
		������ݣ�һ����������ʾ��Ϸ�ܷ�\n\
		14 ����õ���λ \n\
		���ܣ����ص�ǰ����λ��[1-9]\n\
		�������ݣ���\n\
		������ݣ�һ����������ʾ��λ��\n\
		15 ����õ�äע \n\
		���ܣ����ص�ǰ��Сäעֵ\n\
		�������ݣ���\n\
		������ݣ�һ����������ʾäע��С\n\
		16 ����õ�����\n\
		���ܣ����ص�ǰ���ϵ���\n\
		�������ݣ���\n\
		������ݣ������������ֱ��ʾ���������ƣ�ÿ������/100��Ϊ��ɫ[0-3�ֱ��ʾ����÷����Ƭ����]������%100Ϊ�ƺ�[2��2-14��A]\n\
		17 ����õ����� \n\
		���ܣ����ص�ǰ�Ĺ���\n\
		�������ݣ���\n\
		������ݣ�����������ֱ��ʾ������\n\
		18 ����õ���ע\n\
		���ܣ����ص�ǰϵͳҪ��ĸ�עֵ\n\
		�������ݣ���\n\
		������ݣ�һ����������ʾ��ע��С\n\
		19 �����ͣ\n\
		���ܣ���ͣ��ǰ����\n\
		�������ݣ���\n\
		������ݣ���\n\
		20 ���ֹͣ\n\
		���ܣ�ֹͣ��ǰ����\n\
		�������ݣ���\n\
		������ݣ���\n\
		21 �������\n\
		���ܣ���ʼ��ǰ����\n\
		�������ݣ���\n\
		������ݣ���\n\
		22 ��������ʷ\n\
		���ܣ����cookies,�������ַʱ�Զ���¼���ʺ�\n\
		�������ݣ���\n\
		������ݣ���\n\
		"\
		);

	return Text;
}

extern "C"  _LinkDLL DLL_TYPE  GetDllType(){
//��ͬ�ķ���ֵ����������ͬ��C����ʱ��
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
	static CUserMutex         SpacePoolMutex;  //����Ϊstatic��ȷ������ͷ�
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

	//Param.PushString(_T("���ֳ�"));
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

