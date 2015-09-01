// PerformanceView.cpp: implementation of the CPerformanceView class.
//
//////////////////////////////////////////////////////////////////////

#include "PerformanceView.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPerformanceView::CPerformanceView()
{
	
	m_ThreadNumView.SetXGap(8);
    m_ThreadNumView.SetTitle(_T("Thread(s)"));
	m_ThreadNumView.InitRang(0,50);
	m_ThreadNumView.AddLine(THREAD_CENTRAL_NERVE,RGB(198,0,0));
	m_ThreadNumView.AddLine(THREAD_NERVE,RGB(0,198,0));

	m_NerveMsgNumView.SetXGap(8);
	m_NerveMsgNumView.InitRang(0,20);
	m_NerveMsgNumView.SetTitle(_T("Msg(s)"));
	m_NerveMsgNumView.AddLine(CENTRALNERVE_MSG_NUM,RGB(198,0,0));
	m_NerveMsgNumView.AddLine(NERVE_MSG_NUM,RGB(0,198,0));

}

CPerformanceView::~CPerformanceView()
{

}

void    CPerformanceView::Layout(bool Redraw /*= true*/){
	RECT rc;
	::GetClientRect(GetHwnd(),&rc);
	if (RectWidth(rc)==0)
	{
		return;
	}

	int32 w = RectWidth(rc)/2;
	RECT rc1 = rc;
	rc1.right = rc1.left+w-1;
	::MoveWindow(m_ThreadNumView.GetHwnd(),rc1.left,rc1.top,RectWidth(rc1),RectHeight(rc1),TRUE);

	rc1 = rc;
    rc1.left+=w+1;
	::MoveWindow(m_NerveMsgNumView.GetHwnd(),rc1.left,rc1.top,RectWidth(rc1),RectHeight(rc1),TRUE);
}

LRESULT CPerformanceView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	
	if(message == WM_CREATE)
		return OnCreate(wParam,lParam);
	else if(message == WM_SIZE){
		Layout();
		return 0;
	}
	else return CWinSpace2 ::Reaction(message,wParam,lParam);
}

LRESULT CPerformanceView::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT rc={0};
	if(!m_ThreadNumView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE,rc,GetHwnd())){
		return -1;
	}
	
	if(!m_NerveMsgNumView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE,rc,GetHwnd())){
		return -1;
	}
	return 0;
}
