// WSFrameView.cpp : implementation file
//

#include "WSFrameView.h"

namespace VISUALSPACE{
	/////////////////////////////////////////////////////////////////////////////
	// CWSFrameView
	
	CWSFrameView::CWSFrameView()
	{
		HICON	 hIcon  = 0;
  	  
		IconItem*     Icon     = new IconItem(ID_ICON,hIcon);
		TextItem*     Text     = new TextItem(ID_TITLETEXT,_T("Frame"));
		MaxboxItem*   Maxbox   = new MaxboxItem(ID_MAXBNT);
		
		MinboxItem*   Minbox   = new MinboxItem(ID_MINBNT);		
		CloseboxItem* Closebox = new CloseboxItem(ID_CLOSE);
		
		BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];

		if(TitleSpace){
			TitleSpace->DeleteAll();
			
			TitleSpace->PushChild(Icon);
			TitleSpace->PushChild(Text);
			TitleSpace->PushChild(Maxbox);
			TitleSpace->PushChild(Minbox);
			TitleSpace->PushChild(Closebox);
		}

	}
	
	CWSFrameView::~CWSFrameView()
	{
	}
	

	void CWSFrameView::SetIcon(HICON nIcon){
		BorderItem* TitleSpace = NULL;		
		if(m_NcBorderList[0] && m_NcBorderList[0]->m_Alias == ID_TITLE){
			TitleSpace = (BorderItem*)m_NcBorderList[0];
		}	
		if(TitleSpace == NULL)return;
		IconItem*     Item     = (IconItem*)TitleSpace->FindSpace(ID_ICON);
		if(Item)Item->m_hIcon = nIcon;
	}
	
	void CWSFrameView::SetTitle(const TCHAR* Text){
		BorderItem* TitleSpace = NULL;		
		if(m_NcBorderList[0] && m_NcBorderList[0]->m_Alias == ID_TITLE){
			TitleSpace = (BorderItem*)m_NcBorderList[0];
		}	
		if(TitleSpace == NULL)return;
		TextItem*     TextLabel = (TextItem*)TitleSpace->FindSpace(ID_TITLETEXT);
		if(TextLabel)TextLabel->m_Text = Text;
	}
	tstring  CWSFrameView::GetTitle(){
		tstring s;
		BorderItem* TitleSpace = NULL;		
		if(m_NcBorderList[0] && m_NcBorderList[0]->m_Alias == ID_TITLE){
			TitleSpace = (BorderItem*)m_NcBorderList[0];
		}	
		if(TitleSpace == NULL)return s;
		TextItem*     TextLabel = (TextItem*)TitleSpace->FindSpace(ID_TITLETEXT);
		if(TextLabel==NULL)return s;
		return TextLabel->m_Text;
	}
	void CWSFrameView::NcLayout(){
		
		CWSTitleView::NcLayout();
		
		BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
		if(TitleSpace==NULL || TitleSpace->m_ChildList.size()==0)return;
		
		IconItem*     Icon      = (IconItem*)TitleSpace->m_ChildList[0];
		TextItem*     TextLabel = (TextItem*)TitleSpace->m_ChildList[1];
		MaxboxItem*   Maxbox    = (MaxboxItem*)TitleSpace->m_ChildList[2];
		MinboxItem*   Minbox    = (MinboxItem*)TitleSpace->m_ChildList[3];
		CloseboxItem* Closebox  = (CloseboxItem*)TitleSpace->m_ChildList[4];
		
		Icon->m_AreaLeft = TitleSpace->m_AreaLeft;
		Icon->m_AreaRight = Icon->m_AreaLeft + TitleSpace->AreaHeight();
 		Icon->m_AreaTop  = TitleSpace->m_AreaTop;
        Icon->m_AreaBottom = TitleSpace->m_AreaBottom;
		
        
		Closebox->SetArea(TitleSpace);
		int32 len = ((TitleSpace->m_AreaBottom-TitleSpace->m_AreaTop)-13)/2;
		Closebox->DeflateArea(len,len,len,len);	
		
		Closebox->m_AreaLeft = Closebox->m_AreaRight-14;
		//Closebox->m_AreaLeft  -=2;
		//Closebox->m_AreaRight -=2;
		
		Maxbox->m_AreaTop    = Closebox->m_AreaTop;
		Maxbox->m_AreaBottom = Closebox->m_AreaBottom;
		Maxbox->m_AreaLeft   = Closebox->m_AreaLeft -17;
		Maxbox->m_AreaRight  = Closebox->m_AreaRight-17;	
		
		Minbox->m_AreaTop    = Maxbox->m_AreaTop;
		Minbox->m_AreaBottom = Maxbox->m_AreaBottom;
		Minbox->m_AreaLeft   = Maxbox->m_AreaLeft-17;
		Minbox->m_AreaRight  = Maxbox->m_AreaRight-17;	
		
		TextLabel->m_AreaTop    = TitleSpace->m_AreaTop;
		TextLabel->m_AreaBottom = TitleSpace->m_AreaBottom;
		TextLabel->m_AreaLeft = Icon->m_AreaRight+4;
		TextLabel->m_AreaRight = Minbox->m_AreaLeft-4;
	}
	LRESULT CWSFrameView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
		
		switch(message){		
		case WM_LBUTTONDOWN:
			return OnLButtonDown(wParam,lParam);
		case WM_LBUTTONDBLCLK:
			return OnLButtonDblClk(wParam,lParam);
		case WM_GETMINMAXINFO:
			return OnGetMinMaxInfo(wParam,lParam);
		default:
			return CWSTitleView::Reaction(message,wParam,lParam);
		}
	};

	LRESULT CWSFrameView::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
	{
		POINT point;
		point.x = GET_X_LPARAM(lParam); 
		point.y = GET_Y_LPARAM(lParam);
		
		CVSpace2 * Space = NcHit(point.x, point.y);	
		if(Space){
			int64 ID = Space->m_Alias;
			switch(ID){
				
			case ID_LEFTSIDE:
				::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTBOTTOMLEFT,MAKELPARAM(point.x,point.y));
				break;		
			case ID_RIGHTSIDE:
				::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTBOTTOMRIGHT,MAKELPARAM(point.x,point.y));
				break;	
			case ID_BOTTOMSIDE:
				::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTBOTTOM,MAKELPARAM(point.x,point.y));
				break;		
			case ID_TITLETEXT: //TextLabel
				::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
				break;	
			case ID_MAXBNT:
				{	
					MaxboxItem* MaxItem = (MaxboxItem*)Space;
					if(MaxItem->m_bMax){
						::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_RESTORE, 0);
						MaxItem->m_bMax=false;
					}
					else{
						::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_MAXIMIZE, 0);
						MaxItem->m_bMax=true;
					}
					break;
				}
			case ID_MINBNT:{
				        ::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_MINIMIZE, 0);
						   }
				break;
			case ID_CLOSE:
				::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_CLOSE, 0);
				return 0;
			}
			
		}
		return ::DefWindowProc(m_hWnd,WM_LBUTTONDOWN,wParam,lParam);
	}
	
	LRESULT CWSFrameView::OnLButtonDblClk(WPARAM wParam, LPARAM lParam) 
	{
		POINTS* p = (POINTS*)(&lParam);

		CVSpace2 * Space = NcHit(p->x, p->y);	
		if(Space){
			int64 ID = Space->m_Alias;
			if(ID == ID_TITLETEXT){//TextLabel
				BorderItem* Title = (BorderItem*)m_NcBorderList[0];
				MaxboxItem* MaxItem = (MaxboxItem*)Title->m_ChildList[2];
				if(MaxItem->m_bMax){
					::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_RESTORE, 0);
					MaxItem->m_bMax=false;
				}
				else{
					::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_MAXIMIZE, 0);
					MaxItem->m_bMax=true;
				}	
			}
		}
		return ::DefWindowProc(m_hWnd,WM_LBUTTONDBLCLK,wParam,lParam);
	}
	
	LRESULT CWSFrameView::OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam)
	{
		LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
		int32 h =GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYCAPTION)-GetSystemMetrics(SM_CYFRAME); 
		//由于没有非Client区，max时会遮盖整个桌面
		lpmmi->ptMaxSize.y = h;	
		return 0;
	}

} // namespace VISUALSPACE