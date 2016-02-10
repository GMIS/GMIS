// MainFrame.cpp: implementation of the CMainFrame class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786 4244)


#include "MainFrame.h"
#include <iostream>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainFrame::CMainFrame()
:
 m_EditStrBuf(m_AppLog),
 m_EditStrBufW(m_AppLog)
 //m_Log("Executer.Log"),
{
	m_Created = FALSE;
}

CMainFrame::~CMainFrame()
{
	cout.rdbuf( m_OldBuf );
	wcout.rdbuf(m_OldBufW );
	DeleteObject(m_BKBrush);	
   
}

LRESULT CMainFrame::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
	case WM_ERASEBKGND:
		return TRUE;
	case WM_CREATE:
		return OnCreate(wParam,lParam);
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	default:
		return CWSFrameView::Reaction(message,wParam,lParam);
	}
	return 0;
}

LRESULT CMainFrame::OnCreate(WPARAM wParam, LPARAM lParam){
	m_hEdit = CreateWindow( 
					_T("EDIT"),     // predefined class 
					NULL,       // no window title 
					WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
					ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,//|ES_READONLY, 
					0, 0, 0, 0,  // set size in WM_SIZE message 
					m_hWnd,        // parent window 
					(HMENU) 100, // edit control ID 
					(HINSTANCE) GetWindowLong(m_hWnd, GWL_HINSTANCE), 
					NULL);                // pointer not needed 
	if(m_hEdit==NULL)return -1;
				
	m_BKBrush = ::CreateSolidBrush(RGB(0,0,0));
				
	m_AppLog.SetEditCtrl(m_hEdit);
				
	m_OldBuf = cout.rdbuf( &m_EditStrBuf );
	// and also the unicode version
	m_OldBufW = wcout.rdbuf( &m_EditStrBufW );
	return 0;
}
void CMainFrame::Layout(bool Redraw /* = true */){

	//if(!m_Created)return;		
	CWSFrameView::Layout(FALSE);

	RECT rcClient = m_rcClient;
	::MoveWindow(m_hEdit,rcClient.left,rcClient.top,RectWidth(rcClient),RectHeight(rcClient),TRUE);
    rcClient.left +=4;
	::SendMessage(m_hEdit,EM_SETRECT,0,(LPARAM)&rcClient);
};

