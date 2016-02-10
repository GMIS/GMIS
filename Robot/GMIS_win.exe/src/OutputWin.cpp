// OuputWin.cpp: implementation of the COutputWin class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4786)
#pragma warning (disable: 4244)

#include "OutputWin.h"
//#include <wtypes.h>
#include "resource.h"
#include "Conversation.h"
#include "SystemSetting.h"
#include <commdlg.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COutputWin::COutputWin()
:m_Created(false),
 m_Splitter(this,4,20)
{

	m_crWinBorder = RGB(0,0,0);

	m_SpaceSelected = NULL;
    PushChild(&m_Splitter);	

	m_FindView.m_Parent = this;

	m_DialogView.m_Alias = BNT_DIALOG;
	m_OutputView.m_Alias = BNT_OUTPUT;
	m_DebugView.m_Alias = BNT_DEBUG;
	m_LogicView.m_Alias = BNT_LOGIC;
	m_ObjectView.m_Alias = BNT_OBJECT;
	m_FindView.m_Alias = BNT_FIND;

	m_MemoryView.m_Alias = BNT_MEMORY;

};

COutputWin::~COutputWin(){
	m_ChildList[0] = NULL; //avoid delete m_Splitter

}

void COutputWin::HideAllView(){
	SendChildMessage(GetHwnd(),CMD_DELALLVIEW,0,0);	
};


void COutputWin::ShowView(int64 ViewID,BOOL bShow){
	SendChildMessage(GetHwnd(),CMD_SHOW_VIEW,ViewID,bShow);	
}

BOOL COutputWin::HasView(){
	int n = m_Splitter.m_ChildList.size();
	return n;
}

void COutputWin::Layout(bool Redraw /*= true*/){
	RECT rcClient;
	::GetClientRect(GetHwnd(),&rcClient);
    m_Splitter.Layout(rcClient);
}

LRESULT COutputWin::ParentReaction(SpaceRectionMsg* SRM){
	SendParentMessage(SRM->Msg,SRM->wParam,SRM->lParam,SRM->WinSpace);
	return 0;
}


LRESULT COutputWin::ChildReaction(SpaceRectionMsg* SRM){
	switch(SRM->Msg)
	{
	case CMD_SHOW_VIEW:
		{
			int64 ViewID = SRM->wParam;
			BOOL  bShow  = SRM->lParam;
			if (!bShow)
			{
			
				m_Splitter.DeleteView(ViewID);
				return 0;
			};
			
			switch(ViewID){
			case BNT_DIALOG:	
				m_Splitter.AddView(&m_DialogView,true);
				break;
			case BNT_OUTPUT:
				m_Splitter.AddView(&m_OutputView,true);
				break;
			case BNT_DEBUG:
				m_Splitter.AddView(&m_DebugView,true);
				break;
			case BNT_MEMORY:
				m_Splitter.AddView(&m_MemoryView,true);
				break;	
			case BNT_LOGIC:
				m_Splitter.AddView(&m_LogicView,true);
				break;
			case BNT_OBJECT:
				m_Splitter.AddView(&m_ObjectView,true);
				break;
			case BNT_FIND:
				m_Splitter.AddView(&m_FindView,true);
				break;
			}
			Layout(true);
		}
		break;
	case CMD_DELALLVIEW:	
		m_Splitter.Hide();
		m_Splitter.Clear();
		break;	
	}
	return 0;
}

LRESULT COutputWin::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
	   case WM_LBUTTONDOWN:
		   return OnLButtonDown(wParam,lParam);
	   case WM_LBUTTONUP:
		   return OnLButtonUp(wParam,lParam);
	   case WM_MOUSEMOVE:
		   return OnMouseMove(wParam,lParam);
	   case  WM_SIZE:
		   return OnSize(wParam,lParam);
	   case WM_CREATE:
		   return OnCreate(wParam,lParam);
	   default:
		   return CWinSpace2::Reaction(message,wParam,lParam);
	}
}

LRESULT COutputWin::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT rcEdit;
	SetRect(&rcEdit,0,0,0,0);
	// Create the edit control

	if(m_OutputView.Create(GetHinstance(),_T("Output"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rcEdit,GetHwnd(),101) == -1)return -1;

    if(m_FindView.Create(GetHinstance(),_T("FindView"),WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|ES_AUTOHSCROLL|ES_AUTOVSCROLL,rcEdit,GetHwnd(),103) == -1)return -1;
	
	if(m_LogicView.Create(GetHinstance(),_T("LogicView"),WS_CHILD|WS_VISIBLE,rcEdit,GetHwnd(),104) == -1)return -1;
	
	if(m_MemoryView.Create(GetHinstance(),_T("MemoryView"),WS_CHILD|WS_VISIBLE,rcEdit,GetHwnd(),105) == -1)return -1;

	if(m_DebugView.Create(GetHinstance(),_T("DebugView"),WS_CHILD|WS_VISIBLE,rcEdit,GetHwnd(),106) == -1)return -1;

    if(m_DialogView.Create(GetHinstance(),_T("DialogView"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rcEdit,GetHwnd(),108) == -1)return -1;

    if(m_ObjectView.Create(GetHinstance(),_T("ObjectView"),WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rcEdit,GetHwnd(),109) == -1)return -1;
  
	m_Created = true;
	return 0;	
}
LRESULT COutputWin::OnSize(WPARAM wParam, LPARAM lParam){
    long nWidth  = LOWORD(lParam);  // width of client area 
	long nHeight = HIWORD(lParam);  
	if(nWidth==0 || nHeight ==0 )return 0;
	if(m_Created){
		Layout();
	}
	return 0;
}


LRESULT COutputWin::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
{
    POINTS p = MAKEPOINTS(lParam);
	
	POINT point;
	point.x = p.x;
	point.y = p.y;	
	m_SpaceSelected = Hit(point);
	if(m_SpaceSelected){
		m_SpaceSelected->m_State |= SPACE_SELECTED;
		if(m_SpaceSelected->m_Alias == ID_SPLITTER){	
			m_Splitter.OnLButtonDown((SplitterItem*)m_SpaceSelected);
		}
	}
	return 0;
}

LRESULT COutputWin::OnLButtonUp(WPARAM wParam, LPARAM lParam) 
{
	
	if(m_SpaceSelected){
		if(m_SpaceSelected->m_Alias == ID_SPLITTER) {
			m_Splitter.OnLButtonUp((SplitterItem*)m_SpaceSelected);
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
            m_SpaceSelected = NULL; 	
		}
	}
	return 0;
}

LRESULT COutputWin::OnMouseMove(WPARAM wParam, LPARAM lParam) 
{
	
	POINTS point = MAKEPOINTS(lParam);
	if(m_SpaceSelected){
		if(m_SpaceSelected->m_Alias == ID_SPLITTER){
            m_Splitter.OnMouseMove(point.x,point.y,(SplitterItem*)m_SpaceSelected);
		}
	}		
	return 0;
}

