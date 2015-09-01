// WSTitleView.cpp : implementation file
//

#include "WSTitleView.h"

namespace VISUALSPACE{
/////////////////////////////////////////////////////////////////////////////
// CWSTitleView

CWSTitleView::CWSTitleView()
{
   m_crWinBorder = 0x000000;
   m_NcSpaceFocused = NULL;
  
   m_State |= SPACE_SHOWCLIENTBORDER;
   m_TitleHeight  = 24;
   m_NcBoderWidth   = 4;

   BorderItem* NcTitleSpace = new BorderItem(ID_TITLE);
   NcTitleSpace->m_crFocused = NcTitleSpace->m_crActivate;
   BorderItem* NcLeftBorderSpace = new BorderItem(ID_LEFTSIDE);
   BorderItem* NcRightBorderSpace = new BorderItem(ID_RIGHTSIDE);
   BorderItem* NcBottomBorderSpace = new BorderItem(ID_BOTTOMSIDE);

   m_NcBorderList[0] = NcTitleSpace;
   m_NcBorderList[1] = NcLeftBorderSpace;
   m_NcBorderList[2] = NcRightBorderSpace;
   m_NcBorderList[3] = NcBottomBorderSpace;

}

CWSTitleView::~CWSTitleView()
{
	DeleteAllBorder();
}
	
CVSpace2* CWSTitleView::SetTitleBorder(CVSpace2* NewTitle){
	CVSpace2* Old = m_NcBorderList[0];
	m_NcBorderList[0] = NewTitle;
	return Old;
}

CVSpace2* CWSTitleView::SetLeftBorder(CVSpace2* NewLeft){
	CVSpace2* Old = m_NcBorderList[1];
	m_NcBorderList[1] = NewLeft;
	return Old;
}
	
CVSpace2* CWSTitleView::SetRightBorder(CVSpace2* NewRight){
	CVSpace2* Old = m_NcBorderList[2];
	m_NcBorderList[2] = NewRight;
	return Old;
}
	
CVSpace2* CWSTitleView::SetBottomBorder(CVSpace2* NewBottom){
	CVSpace2* Old = m_NcBorderList[3];
	m_NcBorderList[3] = NewBottom;
	return Old;
}
	
void  CWSTitleView::DeleteAllBorder(){
	for(int i=0; i<4; i++){
		CVSpace2*  Space = m_NcBorderList[i];
		if(Space)delete Space;
	}
}
void  CWSTitleView::NcLayout()
{       
	RECT rcWin;
    ::GetClientRect(m_hWnd,&rcWin);
	m_rcClient         = rcWin;
	if(rcWin.right==rcWin.left || rcWin.top==rcWin.bottom)return;

    m_rcClient.left   += m_NcBoderWidth;
	m_rcClient.right  -= m_NcBoderWidth;
	m_rcClient.top    += m_TitleHeight;
	m_rcClient.bottom -= m_NcBoderWidth;

	BorderItem* NcTitleSpace        = (BorderItem*)m_NcBorderList[0];
	BorderItem* NcLeftBorderSpace   = (BorderItem*)m_NcBorderList[1];
	BorderItem* NcRightBorderSpace  = (BorderItem*)m_NcBorderList[2];
	BorderItem* NcBottomBorderSpace = (BorderItem*)m_NcBorderList[3];

	int padding = 0;
	if(m_State & SPACE_SHOWWINBORDER)padding = 1;

    NcTitleSpace->SetArea(padding,padding,rcWin.right-padding,m_TitleHeight);
    NcLeftBorderSpace->SetArea(padding,m_rcClient.top,m_rcClient.left,rcWin.bottom);  
	NcRightBorderSpace->SetArea(m_rcClient.right,m_rcClient.top,rcWin.right-padding,rcWin.bottom);  
	NcBottomBorderSpace->SetArea(padding,m_rcClient.bottom,rcWin.right-padding,rcWin.bottom-padding);


}  

CVSpace2* CWSTitleView::NcHit(int32 x, int32 y){	
	CVSpace2*  Ret = NULL;
	for(int i=0; i<4; i++){
		Ret = m_NcBorderList[i]->HitTest(x,y);
		if(Ret)return Ret;
	}	
	return NULL;
}

 
void  CWSTitleView::NcDraw( HDC hDC){

	for(int i=0; i<4; i++){
		m_NcBorderList[i]->Draw(hDC);
	};	
}

LRESULT CWSTitleView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	switch(message){
		
	case WM_SIZE:
		return OnSize(wParam,lParam);
	case WM_PAINT:
		return OnPaint(wParam,lParam);
	case WM_ERASEBKGND:
		return TRUE;
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam,lParam);
	case WM_NCACTIVATE:
		return OnNcActivate(wParam,lParam);
    case WM_TIMER:  
		return OnTimer(wParam,lParam);
    case WM_NCCALCSIZE:
		return OnNcCalcSize(wParam,lParam);
	default:
		if(message == WM_NCMOUSEMOVE ||
			message == WM_SYNCPAINT   ||
			message == WM_NCPAINT     
			){
		       return 0;
		}
		LRESULT Ret=::DefWindowProc(m_hWnd,message,wParam,lParam);
		return Ret;
	}
}

LRESULT CWSTitleView::OnSize(WPARAM wParam, LPARAM lParam)
{
	if(wParam != SIZE_MINIMIZED)Layout();
	return 0;
}


LRESULT CWSTitleView::OnNcActivate(WPARAM wParam, LPARAM lParam)
{
	BOOL bActive = (BOOL) wParam;
						
	
	for(int i=0; i<4; i++){
		CVSpace2*  Space = m_NcBorderList[i];	
		if(bActive)Space->m_State |= SPACE_ACTIVATE;
		else Space->m_State &= ~SPACE_ACTIVATE;
	};
	::DefWindowProc(m_hWnd,WM_NCACTIVATE,wParam,lParam);
	::InvalidateRect(m_hWnd,NULL,TRUE);
    if(!bActive)::UpdateWindow(m_hWnd);
	return 1;
}


LRESULT CWSTitleView::OnNcCalcSize(WPARAM wParam, LPARAM lParam) 
{
	LONG style = ::GetWindowLong(m_hWnd,GWL_STYLE);
	style &= ~WS_CAPTION;
	::SetWindowLong(m_hWnd,GWL_STYLE,style);
	
	BOOL bCalcValidRects = (BOOL) wParam;  
	NCCALCSIZE_PARAMS FAR* lpncsp = (LPNCCALCSIZE_PARAMS) lParam;
    RECT rcOld = lpncsp->rgrc[0];	
	LRESULT ret = DefWindowProc(m_hWnd,WM_NCCALCSIZE,wParam,lParam);
    RECT& rc = lpncsp->rgrc[0];	
	rc = rcOld;
	return ret;
}

LRESULT CWSTitleView::OnPaint(WPARAM wParam, LPARAM lParam) 
{
	RECT rcWin; 
	GetClientRect(m_hWnd,&rcWin);
	
	PAINTSTRUCT ps;				
	HDC hdc = BeginPaint(m_hWnd, &ps);

	if(rcWin.right==rcWin.left || rcWin.top==rcWin.bottom){
		::EndPaint(m_hWnd, &ps);	
		return 0;
	}
 	
	HDC DCMem = ::CreateCompatibleDC(hdc);
	HBITMAP bmpCanvas=::CreateCompatibleBitmap(hdc, RectWidth(rcWin),RectHeight(rcWin));
	assert(bmpCanvas);
	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, bmpCanvas );

	if(m_State & SPACE_SHOWBKG){
		FillRect(DCMem,rcWin,m_crViewBkg);
	}

	UINT	OldMode	= ::SetBkMode(DCMem,TRANSPARENT );
	NcDraw(DCMem);
    Draw(DCMem);
	::SetBkMode(DCMem, OldMode );

	if(m_State & SPACE_SHOWCLIENTBORDER){
		DrawEdge(DCMem,m_rcClient,m_crClientBorder);// Border
	}		

	if(m_State & SPACE_SHOWWINBORDER){
		DrawEdge(DCMem,rcWin,m_crWinBorder);
	}

	::BitBlt(hdc, 0, 0, RectWidth(rcWin), RectHeight(rcWin), DCMem, 0, 0, SRCCOPY );
	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);
	::EndPaint(m_hWnd, &ps);
	return 0;
}

LRESULT CWSTitleView::OnMouseMove(WPARAM wParam, LPARAM lParam) 
{
	CVSpace2 * Space = NcHit(LOWORD(lParam),HIWORD(lParam));		
	if(Space != m_NcSpaceFocused){
		CVSpace2* OldSpace = m_NcSpaceFocused;
		m_NcSpaceFocused = Space;
		if(Space){
			Space->m_State |= SPACE_FOCUSED;
			::KillTimer(m_hWnd,TIMER_NCMOUSELEAVE);
			::SetTimer(m_hWnd,TIMER_NCMOUSELEAVE,250,NULL);
		}
		if(OldSpace)OldSpace->m_State &= ~SPACE_FOCUSED;
		Invalidate();
	}
	return ::DefWindowProc(m_hWnd,WM_MOUSEMOVE,wParam,lParam);
}

LRESULT CWSTitleView::OnTimer(WPARAM wParam, LPARAM lParam) 
{
	long nIDEvent = wParam;
	if(nIDEvent == TIMER_NCMOUSELEAVE){
		if(m_NcSpaceFocused){
			POINT pt;
			::GetCursorPos(&pt);
			::ScreenToClient(m_hWnd,&pt);
			RECT rc = m_NcSpaceFocused->GetArea();
			if(::PtInRect(&rc,pt)){
				return 0;
			}
			m_NcSpaceFocused->m_State &= ~SPACE_FOCUSED;
			Invalidate();
		} 
		::KillTimer(m_hWnd,nIDEvent);
	}
    return ::DefWindowProc(m_hWnd,WM_TIMER,wParam,lParam);
}

} //namespace VISUALSPACE
