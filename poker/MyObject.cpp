// MyObject.cpp: implementation of the MyObject class.
//
//////////////////////////////////////////////////////////////////////

#include "MyObject.h"
#include <assert.h>
#include <shellapi.h>
#include <commctrl.h>
#include "mousewheelMgr.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
MyObject::MyObject(tstring Name,int64 ID)
	:CUserObject(&m_Timer,NULL)
{
	m_ID = ID;
    m_Name = Name;
};

MyObject::~MyObject(){

};



bool MyObject::Activation(){
	if(!CUserObject::Activation()){
		return false;
	};

	int64 t1 = AbstractSpace::GetTimer()->TimeStamp();
	int64 t2 = t1+10*1000*1000*5; //等待激活5秒
	while (t1<t2 && m_Frame.GetHwnd()==NULL)
	{
		Sleep(100);
		t1 = AbstractSpace::GetTimer()->TimeStamp();
	}
	if (m_Frame.GetHwnd())
	{
		return true;
	}

	return false;
}

void MyObject::Dead()
{
	if (m_Frame.GetHwnd())
	{
		::PostMessage(m_Frame.GetHwnd(),WM_CLOSE,0,0);
	}
	CUserObject::Dead();
};

bool MyObject::Do(Energy* E)
{
	
	if (E==NULL)//执行GUI线程
	{
		if (m_Frame.GetHwnd())
		{
			return true;
		}

		if (OleInitialize(NULL) != S_OK){
			return false;
		}

		HINSTANCE hDllInstance = (HINSTANCE)m_ID;
		
		HINSTANCE hInstance = CWinSpace2::GetHinstance();

		if (AfxInitRichEditEx()==FALSE)
		{
			return false;
		}
		
//		CMouseWheelMgr::Initialize();

		RECT rc;
		::SetRect(&rc,100,150,700,650);
		if(!m_Frame.Create(hInstance,_T("Poker"),WS_CLIPCHILDREN,rc,NULL)){
			m_Frame.Dead();
			return 0;
		}
		HICON hIcon = ::LoadIcon(hDllInstance,MAKEINTRESOURCE(IDI_MAINFRAME));
		CResultView::ImageItem::hPokerImage = ::LoadImage(hDllInstance,MAKEINTRESOURCE(IDB_POKERBITMAP),IMAGE_BITMAP,0,0,0);

		assert(hIcon != NULL);
		::SendMessage(m_Frame.GetHwnd(), WM_SETICON, TRUE, (LPARAM)hIcon);
		::SendMessage(m_Frame.GetHwnd(), WM_SETICON, FALSE, (LPARAM)hIcon);

		m_Frame.SetIcon(hIcon);
		m_Frame.SetTitle(_T("Poker"));	

		//	CenterWindow(Frame.GetHwnd(),NULL);
		::ShowWindow(m_Frame.GetHwnd(), SW_SHOW);
		::UpdateWindow(m_Frame.GetHwnd());

		m_Frame.InitSplitterPos(150);
            
		m_Frame.Activation();

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

		m_Frame.Dead();
		m_Alive = false;

	}else if(m_Alive){

		bool ret = m_Frame.Do(E);
	    return ret;
	}
	return false;
}