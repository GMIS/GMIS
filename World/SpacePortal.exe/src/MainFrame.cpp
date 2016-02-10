// MainFrame.cpp: implementation of the CMainFrame class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "MainFrame.h"
#include "Win32Tool.h"
#include "resource.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainFrame::CMainFrame()
{
	m_Created = FALSE;
	m_SecondHide = -1;
	m_Second = m_SecondHide;

  
	m_State = SPACE_SHOWWINBORDER;

}

CMainFrame::~CMainFrame()
{

}

void CMainFrame::AddRTInfo(const TCHAR* Text){
	m_LogView.AddInfo(Text);
}
void CMainFrame::SetStatus(const TCHAR* Text){
	m_Status.SetTip(Text);
}
void CMainFrame::Layout(bool Redraw /* = true */){
	CWSFrameView::Layout(Redraw);
	if(m_Created){
		RECT rc = m_rcClient;
		rc.bottom -= 20;
		if(rc.bottom<rc.top)rc.bottom=rc.top;
		::MoveWindow(m_LogView.GetHwnd(),rc.left,rc.top,RectWidth(rc),RectHeight(rc),TRUE);
		rc = rc;
		rc.top   = rc.bottom-1;
		rc.bottom=m_rcClient.bottom;
		::MoveWindow(m_Status.GetHwnd(),rc.left,rc.top,RectWidth(rc),RectHeight(rc),TRUE);    
	}
}

LRESULT CMainFrame::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
	case WM_TIMER:
		return OnTimer(wParam,lParam);
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_CREATE:
		return OnCreate(wParam,lParam);
	default:
		return CWSFrameView::Reaction(message,wParam,lParam);
	}
}
LRESULT CMainFrame::OnCreate( WPARAM wParam, LPARAM lParam) 
{
	DWORD style = GetWindowLong(m_hWnd,GWL_STYLE);
	style |=WS_CLIPCHILDREN;
	SetWindowLong(m_hWnd,GWL_STYLE,style);

	RECT rc;
	::SetRect(&rc,0,0,0,0);
	if(!m_LogView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE,rc,GetHwnd())){
		return -1;
	}

	if(!m_Status.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE,rc,GetHwnd())){
		return -1;
	}

	m_Status.SetProgressAlpha(127);

	AddRTInfo(_T("Enter SpacePortal ... "));
	m_Created = TRUE;
	return 0;
}

LRESULT CMainFrame::OnTimer(WPARAM wParam, LPARAM lParam) 
{
	long nIDEvent = wParam;
	if(nIDEvent == TIMER_HIDEWIN){
		m_Second--;
		tstring s = Format1024(_T("%d second..."),m_Second);
		m_Status.SetTip(s.c_str());
		if(m_Second==0){
			m_Second = m_SecondHide;
			m_Status.SetTip(_T(""));
			::KillTimer(GetHwnd(),TIMER_HIDEWIN);
			::ShowWindow(GetHwnd(),SW_HIDE);
		}
	}
	else return CWSFrameView::OnTimer(wParam,lParam);
	return 0;
}
