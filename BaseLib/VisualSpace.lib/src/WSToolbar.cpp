// WSToolbar.cpp : implementation file
//

#include "WSToolbar.h"
#include "GUIElementMisc.h"

namespace VISUALSPACE{

/////////////////////////////////////////////////////////////////////////////
// CWSToolbar

CWSToolbar::CWSToolbar()
{
	m_bVertical = false;
}

CWSToolbar::~CWSToolbar()
{
}

void CWSToolbar::Layout(bool Redraw ){

	if(m_ChildList.size()==0)return;
	
	RECT rcClient;
	GetClientRect(m_hWnd,&rcClient);

	if(m_bVertical){

		int32 len = RectHeight(rcClient)-8;
		len = len/m_ChildList.size();
		
		deque<CVSpace2*>::iterator It = m_ChildList.begin();
		RECT rc = rcClient;
		if(RectWidth(rcClient)>TOOL_DEFAULT_HEIGHT) rc.right = rc.left +TOOL_DEFAULT_HEIGHT;
		rc.top += 4;
		while(It<m_ChildList.end()){
			rc.bottom = rc.top +len;
			CVSpace2* Space = *It;
			
			Space->m_AreaLeft = rc.left+4;
            Space->m_AreaTop  = rc.top+2;
			Space->m_AreaRight = rc.right-4;
			Space->m_AreaBottom = rc.bottom-2;

			rc.top = rc.bottom;
			It++;
		}
	}
	else{
		int32 len = RectWidth(rcClient)-8;
		len = len/m_ChildList.size();
		
		deque<CVSpace2*>::iterator It =m_ChildList.begin();
		RECT rc = rcClient;
		if(RectHeight(rcClient)>TOOL_DEFAULT_HEIGHT) rc.bottom = rc.top +TOOL_DEFAULT_HEIGHT;
		rc.left += 4;
		while(It<m_ChildList.end()){
			rc.right = rc.left +len;
			CVSpace2* Space = *It;
			
			Space->m_AreaLeft = rc.left+4;
            Space->m_AreaTop  = rc.top+2;
			Space->m_AreaRight = rc.right-4;
			Space->m_AreaBottom = rc.bottom-2;

			rc.left = rc.right;
			It++;
		}
	}

    if(Redraw)Invalidate();
}
	
LRESULT CWSToolbar::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if(message ==WM_LBUTTONDOWN)
		return OnLButtonDown(wParam,lParam);
	else if(message == WM_ERASEBKGND)
		return TRUE;
	else if(message == WM_PAINT)
		return OnPaint();
	else if(message == WM_SIZE){
		Layout();
	    return 0;
	}
	else return ::DefWindowProc(m_hWnd,message,wParam,lParam);
}

LRESULT CWSToolbar::OnPaint(){

	RECT rcViewport; 
	GetClientRect(m_hWnd,&rcViewport);
	
	PAINTSTRUCT ps;				
	HDC hdc = BeginPaint(m_hWnd, &ps);

	if(rcViewport.right==rcViewport.left || rcViewport.top==rcViewport.bottom){
		::EndPaint(m_hWnd, &ps);	
		return 0;
	}


 	
	HDC DCMem = ::CreateCompatibleDC(hdc);
	HBITMAP bmpCanvas=::CreateCompatibleBitmap(hdc, RectWidth(rcViewport),RectHeight(rcViewport));
	assert(bmpCanvas);
	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, bmpCanvas );

	if(m_State & SPACE_SHOWBKG){
		FillRect(DCMem,rcViewport,m_crViewBkg);
	}

	UINT	OldMode	= ::SetBkMode(DCMem,TRANSPARENT );
    Draw(DCMem);
	::SetBkMode(DCMem, OldMode );

	if(m_State & SPACE_SHOWWINBORDER){
		DrawEdge(DCMem,rcViewport,m_crWinBorder);// Border
	}
	
	::BitBlt(hdc, 0, 0, RectWidth(rcViewport), RectHeight(rcViewport), DCMem, 0, 0, SRCCOPY );
	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);
	::EndPaint(m_hWnd, &ps);
	return 0;
}

LRESULT CWSToolbar::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
{
    POINTS* p = (POINTS*)(&lParam);
    POINT point;
	point.x = p->x;
	point.y = p->y;
	ButtonItem* Space = (ButtonItem*)Hit(point);
	if(Space){
		 if(Space->m_AttachSpace && Space->m_AttachSpace->GetHwnd())
			SendParentMessage(WM_COMMAND,Space->m_Alias,0,this);
	}
    return 0;
}

} //namespace VISUALSPACE
