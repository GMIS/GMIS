// WSAutoShowBar.cpp : implementation file
//

#include "WSAutoShowBar.h"

namespace VISUALSPACE{

/////////////////////////////////////////////////////////////////////////////
// CWSAutoShowBar

CWSAutoShowBar::CWSAutoShowBar()
{
	m_MaxItemWidth = 40;
}

CWSAutoShowBar::~CWSAutoShowBar()
{
}

void CWSAutoShowBar::AddItem(ButtonItem* Item,CWSPopView* PopWin,bool Redraw/*=false*/)
{
	assert(m_hWnd);
	assert(Item !=NULL);
	bool IsVDraw = false;
	if(m_DockSide == DOCK_LEFT || m_DockSide == DOCK_RIGHT)IsVDraw = true;
	
	if (PopWin!=NULL)
	{
		PopWin->m_Alias = Item->m_Alias;
		
		PopWin->SetTitle(Item->m_Name.c_str());
		
		PopWin->m_Parent = this;
	}


    Item->m_AttachSpace = PopWin;
	Item->m_bVerDrawText = IsVDraw;
	PushChild(Item);	
	if(Redraw){
        Layout(true);
	}
}

ButtonItem* CWSAutoShowBar::GetItem(int64 ID){
	for(uint32 i=0; i<m_ChildList.size(); i++){
		ButtonItem* Item = (ButtonItem*)m_ChildList[i];
		if(Item->m_Alias == ID)return Item;
	}
	return NULL;
}

void CWSAutoShowBar::Layout(bool Redraw ){

	if(m_ChildList.size()==0)return;
	
	RECT rcClient;
	GetClientRect(m_hWnd,&rcClient);

	SetArea(rcClient);

	if(m_bVertical){

		int32 len = RectHeight(rcClient)-8;
		len = len/m_ChildList.size();
	  
		int32 d = len>m_MaxItemWidth?(len-m_MaxItemWidth)/2:2;
		
		deque<CVSpace2*>::iterator It = m_ChildList.begin();
		RECT rc = rcClient;
		if(RectWidth(rcClient)>TOOL_DEFAULT_HEIGHT) rc.right = rc.left +TOOL_DEFAULT_HEIGHT;
		rc.top = 4;
		while(It<m_ChildList.end()){
			rc.bottom = rc.top +len;
			CVSpace2* Space = *It;
			
			Space->SetArea(rc);
			Space->DeflateArea(1,d,1,d);
			rc.top = rc.bottom;
			It++;
		}
	}
	else{
		int32 len = RectWidth(rcClient)-8;
		len = len/m_ChildList.size();
		
		int32 d = len>m_MaxItemWidth?(len-m_MaxItemWidth)/2:2;

		deque<CVSpace2*>::iterator It =m_ChildList.begin();
		RECT rc = rcClient;
		if(RectHeight(rcClient)>TOOL_DEFAULT_HEIGHT) rc.bottom = rc.top +TOOL_DEFAULT_HEIGHT;
		rc.left = 4;
		while(It<m_ChildList.end()){
			rc.right = rc.left +len;
			CVSpace2* Space = *It;
			
			Space->SetArea(rc);
			Space->DeflateArea(1,d,1,d);
			rc.left = rc.right;
			It++;
		}
	}

    if(Redraw)Invalidate();
} 
void CWSAutoShowBar::FocuseChanged(CVSpace2* OldSpace,POINT& point){
	static bool DelayHide = true;
	if(OldSpace){
		OldSpace->m_State &= ~SPACE_FOCUSED;
		Invalidate();
		
		ButtonItem* ToolItem = (ButtonItem*)OldSpace;
		CWSPopView*  PopWin   = (CWSPopView*) ToolItem->m_AttachSpace;

		if(m_SpaceFocused){
			if(!(OldSpace->m_State&SPACE_SELECTED)&&PopWin){
				PopWin->Hide();
			}
		}
		else{ //需要检测鼠标不在弹出的窗口后才能取消focuse			
			
			POINT pt;
			::GetCursorPos(&pt);	

			ButtonItem*  ToolItem = (ButtonItem*)OldSpace;
		    CWSPopView* PopWin   = (CWSPopView*)ToolItem->m_AttachSpace;
            if(PopWin == NULL || PopWin->GetHwnd() == NULL){
				return;
			} 
            if(OldSpace->m_State & SPACE_SELECTED)return;
			if(PopWin->IsPop()){
				if(DelayHide ||PopWin->IsDragBorder()){
					DelayHide = false;
					m_SpaceFocused = OldSpace;
					m_SpaceFocused->m_State|= SPACE_FOCUSED;
					::KillTimer(m_hWnd,TIMER_MOUSELEAVE);
					::SetTimer(m_hWnd,TIMER_MOUSELEAVE,400,NULL);
					return;
				}
				RECT rcWin;
				GetWindowRect(PopWin->GetHwnd(),&rcWin);
				if(::PtInRect(&rcWin,pt)){
					if(m_SpaceFocused)m_SpaceFocused->m_State &= ~SPACE_FOCUSED;
					m_SpaceFocused = OldSpace;
					m_SpaceFocused->m_State |= SPACE_FOCUSED;
					::KillTimer(m_hWnd,TIMER_MOUSELEAVE);
					::SetTimer(m_hWnd,TIMER_MOUSELEAVE,400,NULL);
					return ;
				}
				DelayHide = true;
			}
            PopWin->Hide();
		}
	}
	if(m_SpaceFocused){
		m_SpaceFocused->m_State  |= SPACE_FOCUSED;
		Invalidate();
		if(m_SpaceFocused->m_State & SPACE_DISABLE || m_SpaceFocused->m_State & SPACE_SELECTED)return;
		ButtonItem* ToolItem = (ButtonItem*)m_SpaceFocused;
		CWSPopView* PopWin = (CWSPopView*)ToolItem->m_AttachSpace;
		if(PopWin == NULL)return;
	
		if(!PopWin->GetHwnd()){
			RECT rc;
			::SetRect(&rc,0,0,0,0);
			if(!PopWin->Create(
				(HINSTANCE)GetWindowLong(m_hWnd,GWL_HINSTANCE),
				_T(""),
				WS_CHILD|WS_THICKFRAME|WS_CLIPCHILDREN,
				rc,
				m_hWnd,
				0,
				NULL))
			{
				//int n = GetLastError();
				return;
			}
		}
		PopWin->AsPopView(this);

		PopWin->Pop();		
	}

} 

LRESULT CWSAutoShowBar::ParentRecttion(SpaceRectionMsg* SRM){
	if(SRM->Msg == WM_NCLBUTTONDOWN){
		return m_DockSide;
	}
	return 0;
};

} //namespace VISUALSPACE