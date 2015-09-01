// PopViewporcWin.cpp : implementation file
//

#include "WSPopView.h"
#include "WSAutoShowBar.h"

namespace VISUALSPACE{
	
	/////////////////////////////////////////////////////////////////////////////
	// CWSPopView
	
	CWSPopView::CWSPopView()
	{
		m_TitleHeight  = 20;
		m_StepSize = 30;
		m_PopSize  = 300;
		m_bDragBorder = false;
		m_PopDirection    = POP_NULL;
		m_IsPop       = false;
		m_RightPad = 0;
		m_LeftPad  = 0;

		BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];

		IconItem*     Icon     = new IconItem(ID_ICON,NULL);
		TextItem*     TextLabel= new TextItem(ID_TITLETEXT,_T("PopView"));
		DockboxItem*  Dockbox  = new DockboxItem(ID_DOCKBNT,RGB(192,192,192),RGB(192,192,192),RGB(0,255,0));		
		CloseboxItem* Closebox = new CloseboxItem(ID_CLOSE,RGB(192,192,192),RGB(192,192,192),RGB(0,255,0));
		
		TitleSpace->PushChild(Icon);
		TitleSpace->PushChild(TextLabel);
		TitleSpace->PushChild(Dockbox);
		TitleSpace->PushChild(Closebox);
		
	}
	
	CWSPopView::~CWSPopView()
	{
	}
	
     LRESULT CWSPopView::SendParentMessage(UINT Msg,int64 wParam, int64 lParam,CVSpace2* Space){
			HWND parent;
			if(m_Parent) {
				CWinSpace2* Win = (CWinSpace2*)m_Parent;
				if(Win->GetHwnd())parent = Win->GetHwnd();
			}
			else parent = GetParent(m_hWnd);
			if(parent==NULL)return 0;
			SpaceRectionMsg SRM;
			
			SRM.Msg = Msg;
			SRM.wParam = wParam;
			SRM.lParam = lParam;
			SRM.ChildAffected = Space;
			SRM.WinSpace = this;
			return ::SendMessage(parent,WM_PARENTRECTION,(WPARAM)&SRM,0);
	 }

	void CWSPopView::Pop(){
		assert(m_hWnd);
		
		RECT rcWin;
        CWinSpace2* Parent = (CWinSpace2*)m_Parent;
		assert(Parent);
        
		::GetWindowRect(Parent->GetHwnd(),&rcWin);
		switch(m_PopDirection)
		{
		case POP_LEFT:
			rcWin.right = rcWin.left;
		    rcWin.left = rcWin.right-min(m_StepSize, m_PopSize);
			rcWin.top += m_LeftPad;
			rcWin.bottom -= m_RightPad;
			break;
		case POP_RIGHT:
		    rcWin.left = rcWin.right;
		    rcWin.right = rcWin.left+min(m_StepSize, m_PopSize);
			rcWin.top += m_LeftPad;
			rcWin.bottom -= m_RightPad;
			break;
		case POP_UP:
			rcWin.bottom = rcWin.top;
			rcWin.top =  rcWin.bottom-min(m_StepSize, m_PopSize);
			rcWin.left += m_LeftPad;
			rcWin.right -= m_RightPad;
			break;
		case POP_DOWN:
			rcWin.top = rcWin.bottom;
			rcWin.bottom = rcWin.top + min(m_StepSize, m_PopSize);
			rcWin.left += m_LeftPad;
			rcWin.right -= m_RightPad;
			break;
		case POP_NULL:
			return;
		}
		m_IsPop = true;
		::SetTimer(m_hWnd,TIMER_POPSHOW, 100, NULL);
		::SetWindowPos(m_hWnd,HWND_TOP,rcWin.left,rcWin.top,RectWidth(rcWin),RectHeight(rcWin),SWP_SHOWWINDOW|SWP_NOACTIVATE);
		::SetFocus(m_hWnd);
	}
	
	void CWSPopView::Hide(){
		//DockboxItem* DockBnt = GetDockBnt();
		//if(DockBnt && DockBnt->m_IsDock)return;
		if(m_IsPop)KillTimer(m_hWnd,TIMER_POPSHOW);
		SetWindowPos(m_hWnd,NULL,0,0,0,0,SWP_HIDEWINDOW);
		m_IsPop = false;
	}
	
	void CWSPopView::AsPopView(CWinSpace2* Parent){
		m_Parent = Parent;		
		UINT32 style = WS_POPUP|WS_THICKFRAME|WS_CLIPCHILDREN;
		::SetWindowLong(m_hWnd,GWL_STYLE,style);
		style = WS_EX_TOOLWINDOW;
		::SetWindowLong(m_hWnd,GWL_EXSTYLE,style);	
		::SetParent(m_hWnd,NULL);
}
	
	void CWSPopView::AsChildView(CWinSpace2* Parent,UINT32 Style/*=0*/){
		m_Parent = Parent;
		long style = ::GetWindowLong(m_hWnd,GWL_STYLE);
		style &= ~WS_POPUP;
		style |= WS_CHILD|WS_VISIBLE|Style;
		::SetWindowLong(m_hWnd,GWL_STYLE,style);
		::SetParent(m_hWnd,Parent->GetHwnd());
		m_IsPop = false;
	}
	
	void CWSPopView::SetIcon(HICON nIcon){
		BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
		if(TitleSpace==NULL)return;
		IconItem*     Item       = (IconItem*)TitleSpace->FindSpace(ID_ICON);
		if(Item)Item->m_hIcon = nIcon;
	}
	
	void CWSPopView::SetTitle(const TCHAR* Text){
		BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
		if(TitleSpace==NULL)return;
		TextItem*     TextLabel  = (TextItem*)TitleSpace->FindSpace(ID_TITLETEXT); //  TitleSpace->m_ChildList[1];
		if(TextLabel)TextLabel->m_Text = Text;
	}

	tstring CWSPopView::GetTitle(){
		tstring s;
		BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
		if(TitleSpace==NULL)return s;
		TextItem* TextLabel  = (TextItem*)TitleSpace->FindSpace(ID_TITLETEXT); //  TitleSpace->m_ChildList[1];
		if(TextLabel) return TextLabel->m_Text;
		return s;
	}

	DockboxItem* CWSPopView::GetDockBnt(){
		BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
		if(TitleSpace==NULL)return NULL;
		return (DockboxItem*)TitleSpace->FindSpace(ID_DOCKBNT);
	}
	
	void CWSPopView::NcLayout(){
		
		CWSTitleView::NcLayout();
		
		BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
		if(TitleSpace==NULL || TitleSpace->m_ChildList.size()<4)return;
		
		IconItem*     Icon      = (IconItem*)TitleSpace->m_ChildList[0];
		TextItem*     TextLabel = (TextItem*)TitleSpace->m_ChildList[1];
		DockboxItem*  DockBox   = (DockboxItem*)TitleSpace->m_ChildList[2];
		CloseboxItem* Closebox  = (CloseboxItem*)TitleSpace->m_ChildList[3];
		

		Icon->m_AreaLeft = TitleSpace->m_AreaLeft;
		Icon->m_AreaTop  = TitleSpace->m_AreaTop;
		Icon->m_AreaBottom = TitleSpace->m_AreaBottom;
		Icon->m_AreaRight = TitleSpace->m_AreaBottom - TitleSpace->m_AreaTop;
		

		Closebox->SetArea(TitleSpace);
		int32 len = ((TitleSpace->m_AreaBottom-TitleSpace->m_AreaTop)-13)/2;
		Closebox->DeflateArea(len,len,len,len);	
		
		Closebox->m_AreaLeft = Closebox->m_AreaRight-14;
		Closebox->m_AreaLeft  -=2;
		Closebox->m_AreaRight -=2;
		
		DockBox->SetArea(Closebox);
		DockBox->m_AreaLeft-=17;
		DockBox->m_AreaRight-=17;
		
		TextLabel->m_AreaTop = TitleSpace->m_AreaTop;
		TextLabel->m_AreaBottom = TitleSpace->m_AreaBottom;
		TextLabel->m_AreaLeft = Icon->m_AreaRight+2;
		TextLabel->m_AreaRight = DockBox->m_AreaLeft-2;
	}
	
	LRESULT CWSPopView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
		if(message == WM_TIMER)  
			return OnTimer(wParam,lParam);
		else if(message == WM_LBUTTONDOWN)
			return OnLButtonDown(wParam,lParam);
		else return CWSTitleView::Reaction(message,wParam,lParam);
	}
	
	LRESULT CWSPopView::OnTimer(WPARAM wParam, LPARAM lParam) 
	{
		long nIDEvent = wParam;
		
		if(nIDEvent == TIMER_POPSHOW){
			RECT rcWin;
			bool Completed = false;
			::GetWindowRect(m_hWnd,&rcWin);
			        
			switch(m_PopDirection)
			{
			case POP_DOWN:
				rcWin.bottom += m_StepSize;
				if(RectHeight(rcWin)>=m_PopSize){
					rcWin.bottom = rcWin.top+m_PopSize;
					Completed = true;
				}
				break;
			case POP_UP:
				rcWin.top -= m_StepSize;
				if(RectHeight(rcWin)>=m_PopSize){
					rcWin.top = rcWin.bottom-m_PopSize;
					Completed = true;
				}
				break;
			case POP_RIGHT:
				rcWin.right += m_StepSize;
				if(RectWidth(rcWin)>=m_PopSize){
					rcWin.right = rcWin.left+m_PopSize;
					Completed = true;
				}
				break;
			case POP_LEFT:
				rcWin.left   -= m_StepSize;
				if(RectWidth(rcWin)>=m_PopSize){
					rcWin.left = rcWin.right-m_PopSize;
					Completed = true;
				}
				break;
			}
			//::SetWindowPos(m_hWnd,NULL,rcWin.left,rcWin.top,RectWidth(rcWin),RectHeight(rcWin),SWP_SHOWWINDOW);
			::MoveWindow(m_hWnd,rcWin.left,rcWin.top,RectWidth(rcWin),RectHeight(rcWin),TRUE);
			if(Completed){
				KillTimer(m_hWnd,TIMER_POPSHOW);
			}
			return 0;
		}
		else return CWSTitleView::OnTimer(wParam,lParam);
	}
	
	LRESULT CWSPopView::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
	{
		POINT point;
		point.x = GET_X_LPARAM(lParam); 
		point.y = GET_Y_LPARAM(lParam);
		
		CVSpace2 * Space = NcHit(point.x,point.y);	
		if(Space){
			int64 ID = Space->m_Alias;
			DockboxItem* DockBnt = GetDockBnt();
			if(DockBnt==NULL)return 0;
			switch(ID){
			case ID_LEFTSIDE:
				if(DockBnt->m_IsDock){
					::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTBOTTOMLEFT,MAKELPARAM(point.x,point.y));
				}
				else {
					
					if(m_PopDirection == POP_LEFT)return 0;
					m_bDragBorder = true;
					::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTLEFT,MAKELPARAM(point.x,point.y));
					RECT rc;
					::GetWindowRect(m_hWnd,&rc);
					m_PopSize = RectWidth(rc);
					m_bDragBorder = false;
				}
				break;		
			case ID_RIGHTSIDE:
				if(DockBnt->m_IsDock){
					::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTBOTTOMRIGHT,MAKELPARAM(point.x,point.y));
				}
				else{
					if(m_PopDirection == POP_RIGHT)return 0;
					m_bDragBorder = true;
					::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTRIGHT,MAKELPARAM(point.x,point.y));
					RECT rc;
					::GetWindowRect(m_hWnd,&rc);
					m_PopSize = RectWidth(rc);	
					m_bDragBorder = false;
				}
				break;
			case ID_BOTTOMSIDE:
				if(DockBnt->m_IsDock){
					::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTBOTTOM,MAKELPARAM(point.x,point.y));
				}
				else{
					if(m_PopDirection == POP_DOWN)return 0;
					m_bDragBorder = true;
					::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTBOTTOM,MAKELPARAM(point.x,point.y));
					RECT rc;
					::GetWindowRect(m_hWnd,&rc);
					m_PopSize = RectHeight(rc);	
					m_bDragBorder = false;
				}
				break;
			case ID_TITLETEXT: //TextLabel
				if(DockBnt->m_IsDock){
					::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
				}
				else{
					if(m_PopDirection == POP_UP)return 0;
					m_bDragBorder = true;
					::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTTOP,MAKELPARAM(point.x,point.y));
					RECT rc;
					::GetWindowRect(m_hWnd,&rc);
					m_PopSize = RectHeight(rc);	
					m_bDragBorder = false;
				}
				break;
			case ID_DOCKBNT:
				{	
					assert(m_Parent!=NULL);
					if(DockBnt->m_IsDock){
						DockBnt->m_IsDock = false;
						Hide();
						SendParentMessage(CM_DOCKVIEW,0,0,Space);
						
					}else{//偏移10
						DockBnt->m_IsDock = true;
						SendParentMessage(CM_DOCKVIEW,0,0,Space);
						RECT rc;
						::GetWindowRect(m_hWnd,&rc);
						POINT p;

						if(m_PopDirection == POP_RIGHT){
							p.x = rc.left+30;  p.y = rc.top+30;}
						else if(m_PopDirection == POP_LEFT){
							p.x = rc.left-30;  p.y = rc.top+30;}
						else if(m_PopDirection == POP_UP){
							p.x = rc.left;  p.y = rc.top-30;}
						
						::SetWindowPos(m_hWnd,HWND_TOP,p.x,p.y,RectWidth(rc),RectHeight(rc),0);	
						return 0;
					}
				}
			}
			
		}
		return ::DefWindowProc(m_hWnd,WM_LBUTTONDOWN,wParam,lParam);
	}
	
} //namespace VISUALSPACE
