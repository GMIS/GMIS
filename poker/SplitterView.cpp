// SplitterView.cpp: implementation of the CSplitterView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786) 

#include "SplitterView.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSplitterView::CSplitterView(CWinSpace2* Parent,BOOL IsVertical)
:m_Splitter(this,4,20,IsVertical),m_SpaceSelected(NULL),m_Parent(Parent)
{
	PushChild(&m_Splitter);	
}

CSplitterView::~CSplitterView()
{
	m_ChildList.clear(); //avoid delete m_Splitter
}

void CSplitterView::AddView(CWinSpace2* View,DWORD Style){
	m_Splitter.AddView(View,false,Style);
}

void CSplitterView::DelView(int32 ViewID){
	m_Splitter.DeleteView(ViewID);
}

void  CSplitterView::Layout(bool Redraw){
	RECT rc;
	::GetClientRect(GetHwnd(),&rc);
    m_Splitter.Layout(rc);
}

LRESULT CSplitterView::ParentReaction(SpaceRectionMsg* SRM){
	if(m_Parent)m_Parent->ParentReaction(SRM);
	return 0;
};

//´¦ÀíWM_CHILDRECTION
LRESULT CSplitterView::ChildReaction(SpaceRectionMsg* SRM){
	if(m_Parent)m_Parent->ChildReaction(SRM);	
	return 0;
};

LRESULT CSplitterView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
	case WM_ERASEBKGND:
		return TRUE;
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam,lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam,lParam);
	case WM_SIZE:
		return OnSize(wParam,lParam);
	default:
		return CWinSpace2::Reaction(message,wParam,lParam);
	}
	return 0;
};

LRESULT CSplitterView::OnSize(WPARAM wParam, LPARAM lParam){
	Layout();
	return 0;
}

LRESULT CSplitterView::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
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

LRESULT CSplitterView::OnLButtonUp(WPARAM wParam, LPARAM lParam) 
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

LRESULT CSplitterView::OnMouseMove(WPARAM wParam, LPARAM lParam) 
{
	
	POINTS point = MAKEPOINTS(lParam);
	if(m_SpaceSelected){
		if(m_SpaceSelected->m_Alias == ID_SPLITTER){
            m_Splitter.OnMouseMove(point.x,point.y,(SplitterItem*)m_SpaceSelected);
		}
	}		
	return 0;
}