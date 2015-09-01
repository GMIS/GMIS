// WSScrollView.cpp : implementation file
//

#include "WSScrollView.h"

namespace VISUALSPACE{

CWSScrollView::CWSScrollView()
{
	m_UseSystemScrollbar = false;
	m_ThumbDragPos   = -1;

	m_ScrollSelected = NULL;
	m_ScrollFocused  = NULL;

	m_ScrollMoveUnit = SCROLL_MOVE_UNIT;
 
	m_HScrollbar.DeleteAll();
	m_VScrollbar.DeleteAll();

	ArrowItem* LeftArrow  = new ArrowItem(1,ARROW_LEFT);
	ThumbItem* HThumb     = new ThumbItem(2,false);
	ArrowItem* RightArrow = new ArrowItem(3,ARROW_RIGHT);

	m_HScrollbar.PushChild(LeftArrow);
	m_HScrollbar.PushChild(HThumb);
	m_HScrollbar.PushChild(RightArrow);

	ArrowItem* UpArrow   = new ArrowItem(1,ARROW_UP);
	ThumbItem* VThumb    = new ThumbItem(2,true);
	ArrowItem* DownArrow = new ArrowItem(3,ARROW_DOWN);

	m_VScrollbar.PushChild(UpArrow);
	m_VScrollbar.PushChild(VThumb);
	m_VScrollbar.PushChild(DownArrow);

   
	m_ScrollWidth    = 16;
  
}

CWSScrollView::~CWSScrollView()
{
}


void CWSScrollView::LayoutScrollBar(){
	RECT rcClient;
	::GetClientRect(GetHwnd(),&rcClient);
	
	if (m_UseSystemScrollbar)
	{
		
		int32 VPageLen   = RectHeight(rcClient); 
		int32 HPageLen   = RectWidth(rcClient);
		
		//确定Scrollbar rect
		if(HPageLen< m_SizeX ){
			::ShowScrollBar(m_hWnd, SB_HORZ,TRUE);
			SCROLLINFO	si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_PAGE | SIF_RANGE;
			si.nPage = HPageLen;
			si.nMax = m_SizeX;
			si.nMin = 0 ;
			
			::SetScrollInfo( m_hWnd, SB_HORZ, &si,TRUE );
		}else{
			 ::SetScrollPos(m_hWnd, SB_HORZ, 0,FALSE );
			 ::ShowScrollBar(m_hWnd, SB_HORZ, FALSE );	// Hide it
		}
		
		if(VPageLen < m_SizeY ){
			::ShowScrollBar(m_hWnd, SB_VERT,TRUE);
			SCROLLINFO	si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_PAGE | SIF_RANGE;
			si.nPage = VPageLen;
			si.nMax = m_SizeY;
			si.nMin = 0 ;
			
			::SetScrollInfo( m_hWnd, SB_VERT, &si,TRUE );
		}else{
			::SetScrollPos(m_hWnd, SB_VERT, 0 ,TRUE);
			::ShowScrollBar( m_hWnd,SB_VERT, FALSE );	// Hide it
		}
		
	}else{

		
		int32& VPageLen   = m_VScrollbar.m_SizeX;
		int32& VScrollPos = m_VScrollbar.m_SizeY;
		
		int32& HPageLen   = m_HScrollbar.m_SizeX;
		int32& HScrollPos = m_HScrollbar.m_SizeY;
		
		//确定Scrollbar rect
		if(RectWidth(rcClient)< m_SizeX ){
			m_HScrollbar.SetArea(rcClient);
			m_HScrollbar.m_AreaTop = rcClient.bottom - m_ScrollWidth;
			rcClient.bottom = m_HScrollbar.m_AreaTop;	
			if(RectHeight(rcClient) < m_SizeY){
				m_HScrollbar.m_AreaRight -= m_ScrollWidth;
				m_VScrollbar.SetArea(rcClient);
				m_VScrollbar.m_AreaLeft = rcClient.right -m_ScrollWidth;
				m_VScrollbar.m_AreaBottom += m_ScrollWidth; //让垂直棒填满交叉地带
				rcClient.right = m_VScrollbar.m_AreaLeft;
			}
			else{
				m_VScrollbar.SetArea(0,0,0,0);
				VScrollPos = 0;
			}
		}
		else if(RectHeight(rcClient)< m_SizeY ){
			m_VScrollbar.SetArea(rcClient);
			m_VScrollbar.m_AreaLeft = rcClient.right - m_ScrollWidth;
			rcClient.right = m_VScrollbar.m_AreaLeft;		 
			if(RectWidth(rcClient) < m_SizeX){
				//m_VScrollbar.m_AreaBottom -= m_ScrollWidth;
				m_HScrollbar.SetArea(rcClient);
				m_HScrollbar.m_AreaTop = rcClient.bottom -m_ScrollWidth;
				rcClient.bottom  = m_HScrollbar.m_AreaTop;
			}
			else{
				m_HScrollbar.SetArea(0,0,0,0);
				HScrollPos = 0;
			}
		}
		else{
			m_HScrollbar.SetArea(0,0,0,0);
			m_VScrollbar.SetArea(0,0,0,0);
			VScrollPos = 0;
			HScrollPos = 0;	
		}
		HPageLen = RectWidth(rcClient);
		VPageLen = RectHeight(rcClient);
		
		//m_Area = rcClient;
		
		LayoutHScrollBar();
		LayoutVScrollBar();
	}

}

void CWSScrollView::LayoutVScrollBar(){
	if((m_VScrollbar.m_AreaRight - m_VScrollbar.m_AreaLeft)==0)return;
    
	int32 DocLen      = m_SizeY; 
	int32& VPageLen   = m_VScrollbar.m_SizeX;
    int32& VScrollPos = m_VScrollbar.m_SizeY;

    CVSpace2* FrontArrow = m_VScrollbar.m_ChildList.front();
    CVSpace2* BackArrow  = m_VScrollbar.m_ChildList.back();
    CVSpace2* Thumb      = m_VScrollbar.m_ChildList[1];
   
	int32 h   = m_VScrollbar.m_AreaRight-m_VScrollbar.m_AreaLeft;
	int32 len = m_VScrollbar.m_AreaBottom-m_VScrollbar.m_AreaTop;
	int32 ScrollH = len-h-h;
	
	if(ScrollH<MINTHUMB){
		h=len/2;
		FrontArrow->m_AreaLeft = m_VScrollbar.m_AreaLeft;
		FrontArrow->m_AreaTop  = m_VScrollbar.m_AreaTop;
		FrontArrow->m_AreaRight = m_VScrollbar.m_AreaRight;
		FrontArrow->m_AreaBottom = FrontArrow->m_AreaTop+h;
		
		BackArrow->m_AreaLeft = m_VScrollbar.m_AreaLeft;
		BackArrow->m_AreaRight = m_VScrollbar.m_AreaRight;
		BackArrow->m_AreaBottom = m_VScrollbar.m_AreaBottom;
		BackArrow->m_AreaTop = BackArrow->m_AreaBottom-h;
		
		Thumb->SetArea(0,0,0,0);		
	}
	else{
		FrontArrow->m_AreaLeft = m_VScrollbar.m_AreaLeft;
		FrontArrow->m_AreaTop  = m_VScrollbar.m_AreaTop;
		FrontArrow->m_AreaRight = m_VScrollbar.m_AreaRight;
		FrontArrow->m_AreaBottom = FrontArrow->m_AreaTop+h;
		
		BackArrow->m_AreaLeft = m_VScrollbar.m_AreaLeft;
		BackArrow->m_AreaRight = m_VScrollbar.m_AreaRight;
		BackArrow->m_AreaBottom = m_VScrollbar.m_AreaBottom;
		BackArrow->m_AreaTop = BackArrow->m_AreaBottom-h;
		
		
		int32 ThumbLen = (int32)((float)ScrollH/(float)DocLen*(float)VPageLen + 0.5f);
		ThumbLen = max(ThumbLen,MINTHUMB);
		
		VScrollPos= max(VScrollPos,0);
		VScrollPos = min(VScrollPos,DocLen-VPageLen);
		
		int32 ThumbPos      =  (int32)((float)ScrollH/(float)DocLen*(float)VScrollPos+0.5f);
		Thumb->m_AreaLeft   =  m_VScrollbar.m_AreaLeft;
        Thumb->m_AreaRight  =  m_VScrollbar.m_AreaRight;
		Thumb->m_AreaTop    =  ThumbPos + FrontArrow->m_AreaBottom;
		Thumb->m_AreaBottom =  Thumb->m_AreaTop+ThumbLen;	
	}
}

void CWSScrollView::LayoutHScrollBar(){
	
	if((m_HScrollbar.m_AreaBottom-m_HScrollbar.m_AreaTop)==0)return;
	   
	int32 DocLen      = m_SizeX; 
	int32& HPageLen   = m_HScrollbar.m_SizeX;
	int32& HScrollPos = m_HScrollbar.m_SizeY;
	   
	CVSpace2* FrontArrow = m_HScrollbar.m_ChildList.front();
    CVSpace2* BackArrow  = m_HScrollbar.m_ChildList.back();
	CVSpace2* Thumb      = m_HScrollbar.m_ChildList[1];
	   
	int32 w   = m_HScrollbar.m_AreaBottom - m_HScrollbar.m_AreaTop;
	int32 len = m_HScrollbar.m_AreaRight-m_HScrollbar.m_AreaLeft;
	int32 ScrollW = len-w-w;
	   
	if(ScrollW<MINTHUMB){
		w=len/2;
		FrontArrow->m_AreaLeft   = m_HScrollbar.m_AreaLeft;
        FrontArrow->m_AreaTop    = m_HScrollbar.m_AreaTop;
		FrontArrow->m_AreaBottom = m_HScrollbar.m_AreaBottom;
		FrontArrow->m_AreaRight  = FrontArrow->m_AreaLeft+w;
		
		BackArrow->m_AreaRight  = m_HScrollbar.m_AreaRight;
		BackArrow->m_AreaTop    = m_HScrollbar.m_AreaTop;
		BackArrow->m_AreaBottom = m_HScrollbar.m_AreaBottom;
		BackArrow->m_AreaLeft = BackArrow->m_AreaRight-w;
		
		Thumb->SetArea(0,0,0,0);	
	}
	else{
		FrontArrow->m_AreaLeft   = m_HScrollbar.m_AreaLeft;
        FrontArrow->m_AreaTop    = m_HScrollbar.m_AreaTop;
		FrontArrow->m_AreaBottom = m_HScrollbar.m_AreaBottom;
		FrontArrow->m_AreaRight  = FrontArrow->m_AreaLeft+w;
		
		BackArrow->m_AreaRight  = m_HScrollbar.m_AreaRight;
		BackArrow->m_AreaTop    = m_HScrollbar.m_AreaTop;
		BackArrow->m_AreaBottom = m_HScrollbar.m_AreaBottom;
		BackArrow->m_AreaLeft = BackArrow->m_AreaRight-w;
		
		int32 ThumbLen = (int32)((float)ScrollW/(float)DocLen*(float)HPageLen + 0.5f);
		ThumbLen = max(ThumbLen,MINTHUMB);
		
		HScrollPos = max(HScrollPos,0);
		HScrollPos = min(HScrollPos,DocLen-HPageLen);
		
		int32 ThumbPos = (int32)((float)ScrollW/(float)DocLen*(float)HScrollPos +0.5f);
		Thumb->m_AreaTop       = m_HScrollbar.m_AreaTop;
		Thumb->m_AreaBottom    = m_HScrollbar.m_AreaBottom;
		Thumb->m_AreaLeft  = ThumbPos+FrontArrow->m_AreaRight;
		Thumb->m_AreaRight = Thumb->m_AreaLeft+ThumbLen;		
	}
  
}
		
int32 CWSScrollView::GetVScrollPos(){
	if(m_UseSystemScrollbar){
		return ::GetScrollPos(m_hWnd,SB_VERT);
	}
	else {
		return m_VScrollbar.m_SizeY;
	};
};
int32 CWSScrollView::GetHScrollPos(){ 
	if(m_UseSystemScrollbar){
		return ::GetScrollPos(m_hWnd,SB_HORZ);
	}else{
		return m_HScrollbar.m_SizeY;
	}
};	

void CWSScrollView::SetVScrollPos(int32 Pos,bool Redraw){
  	
	if(m_UseSystemScrollbar){
		::SetScrollPos(m_hWnd, SB_VERT, Pos ,(BOOL)Redraw);
		if(Redraw)Layout(Redraw);
	}else{
		int32& VScrollPos = m_VScrollbar.m_SizeY;
		VScrollPos = Pos;
		LayoutScrollBar();
	    if(Redraw)Layout(Redraw);
	}
}
	
void CWSScrollView::SetHScrollPos(int32 Pos,bool Redraw){
	if(m_UseSystemScrollbar){
       ::SetScrollPos(m_hWnd, SB_HORZ, 0,(BOOL)Redraw );
	   if(Redraw)Layout(Redraw);
	}else{
		int32& HScrollPos = m_HScrollbar.m_SizeY;
		HScrollPos = Pos;
		LayoutScrollBar();
		if(Redraw)Layout(Redraw);
	}
}
 
void  CWSScrollView::DrawScrollbar(HDC hDC){
	assert(m_UseSystemScrollbar == false);
	int n= 0;
	if((m_HScrollbar.m_AreaRight-m_HScrollbar.m_AreaLeft)>0){
		FillRect(hDC,m_HScrollbar.GetArea(), m_crViewBkg);
		deque<CVSpace2*>::iterator It = m_HScrollbar.m_ChildList.begin();
		while(It<m_HScrollbar.m_ChildList.end()){
			CVSpace2* Space = *It;
			Space->Draw(hDC);
			It++;
		}
		n++;
	}		
	if((m_VScrollbar.m_AreaRight-m_VScrollbar.m_AreaLeft)>0){
		FillRect(hDC,m_VScrollbar.GetArea(),m_crViewBkg);
		deque<CVSpace2*>::iterator It = m_VScrollbar.m_ChildList.begin();
		while(It<m_VScrollbar.m_ChildList.end()){
			CVSpace2* Space = *It;
			Space->Draw(hDC);
			It++;
		}
		n++;
	}
	
	//滚动棒交叉矩形
/*
    if(n==2){
		CRect rcBox = m_Area;
		rcBox.left = rcBox.right;
		rcBox.top  = rcBox.bottom;
		rcBox.right  += m_ScrollWidth;
		rcBox.bottom += m_ScrollWidth;
        pDC->FillSolidRect(rcBox,m_crViewBkg);
	}
*/
}
	
CVSpace2*  CWSScrollView::Hit(POINT& point){		
//		if(!::PtInRect(&m_Area,point))return NULL;

		CVSpace2* Ret = NULL;
		if(Ret = m_VScrollbar.HitTest(point.x,point.y))return Ret;
		if(Ret = m_HScrollbar.HitTest(point.x,point.y))return Ret;

		for(uint32 i=0; i<m_ChildList.size(); i++){
			Ret = m_ChildList[i]->HitTest(point.x,point.y); 	
			if(Ret)return Ret;
		};
		return Ret;
}		
LRESULT CWSScrollView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	switch(message){
	case WM_PAINT:
		return OnPaint(wParam,lParam);
	case WM_ERASEBKGND:
		return TRUE;
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam,lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam,lParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam,lParam);
	case WM_HSCROLL:
		return OnHScroll(wParam,lParam);
	case WM_VSCROLL:
		return OnVScroll(wParam,lParam);
    case WM_TIMER:  
		return OnTimer(wParam,lParam);
	default:
		return ::DefWindowProc(m_hWnd,message,wParam,lParam);
	}
}
LRESULT CWSScrollView::OnMouseWheel(WPARAM wParam, LPARAM lParam) 
{
	
	
	int32 ScrollPos = GetVScrollPos();
	short zDelta = (short) HIWORD(wParam);
	ScrollPos += ( zDelta > 0 )? - m_ScrollMoveUnit:m_ScrollMoveUnit;
	SetVScrollPos(ScrollPos,true);
	
	if (m_UseSystemScrollbar)
	{
		Layout();
	} 
	return ::DefWindowProc(m_hWnd,WM_MOUSEWHEEL,wParam,lParam);
}

LRESULT CWSScrollView::OnTimer(WPARAM wParam, LPARAM lParam) 
{
	int32 nIDEvent = wParam;
	::SetTimer(m_hWnd,nIDEvent, 100, NULL);
	
	switch (nIDEvent)
	{
    case TIMER_SCROLL_INCR:
		if(m_ScrollBarDraged == V_SCROLL){
			int32 ScrollPos = GetVScrollPos();
			ScrollPos += SCROLL_MOVE_UNIT;
			SetVScrollPos(ScrollPos,false);
		}
		else{
			int32 ScrollPos = GetHScrollPos();
			ScrollPos += SCROLL_MOVE_UNIT;
			SetHScrollPos(ScrollPos,false);
		}
		RedrawWindow(m_hWnd,NULL,NULL,RDW_UPDATENOW);
		break;
    case TIMER_SCROLL_DECR: 
		if(m_ScrollBarDraged == V_SCROLL){
			int32 ScrollPos = GetVScrollPos();
			ScrollPos -= SCROLL_MOVE_UNIT;
			SetVScrollPos(ScrollPos,false);
		}
		else{
			int32 ScrollPos = GetHScrollPos();
			ScrollPos -= SCROLL_MOVE_UNIT;
			SetHScrollPos(ScrollPos,false);
		}
		RedrawWindow(m_hWnd,NULL,NULL,RDW_UPDATENOW);
		break;
	}	
    return ::DefWindowProc(m_hWnd,WM_TIMER,wParam,lParam);
}

LRESULT CWSScrollView::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
{
    POINTS* p = (POINTS*)(&lParam);
	
	CVSpace2* Space = NULL;
	
	if(Space = m_HScrollbar.HitTest(p->x,p->y)){
		m_ScrollSelected = Space; 
		m_ScrollBarDraged  = H_SCROLL;
		if(Space->m_Alias>0){
            Space->m_State |= SPACE_SELECTED;
			int32 HScrollPos = GetHScrollPos();
			if(Space->m_Alias == 1){ //front
				HScrollPos -= m_ScrollMoveUnit;
				SetHScrollPos(HScrollPos,true);
				::SetTimer(m_hWnd,TIMER_SCROLL_DECR,500,NULL);
			}
			else if(Space->m_Alias ==2){ //thumb
				m_ThumbDragPos = p->x;	
				::SetCapture(m_hWnd);
				Invalidate();
				return 0;
			}
			else if(Space->m_Alias == 3){//back
				HScrollPos += m_ScrollMoveUnit;
				SetHScrollPos(HScrollPos,true);
				::SetTimer(m_hWnd,TIMER_SCROLL_INCR,500,NULL);
			}			
		}
	}
	else if(Space = m_VScrollbar.HitTest(p->x,p->y)){
		m_ScrollBarDraged  = V_SCROLL;
		m_ScrollSelected = Space;  
		if(Space->m_Alias>0){
			::SetCapture(m_hWnd);
            Space->m_State |= SPACE_SELECTED;
			int32 VScrollPos = GetVScrollPos();
			if(Space->m_Alias == 1){ //front
				VScrollPos -= m_ScrollMoveUnit;
				SetVScrollPos(VScrollPos,true);
				::SetTimer(m_hWnd,TIMER_SCROLL_DECR,500,NULL);
			}
			else if(Space->m_Alias == 2){ //thumb
				m_ThumbDragPos = p->y;
				Invalidate();
				return 0;
			}
			else if(Space->m_Alias == 3){ //back
				VScrollPos += m_ScrollMoveUnit;
				SetVScrollPos(VScrollPos,true);
				::SetTimer(m_hWnd,TIMER_SCROLL_INCR,500,NULL);
			}			
		}
	}
	else m_ScrollSelected = NULL;
	
    return 0;
}

LRESULT CWSScrollView::OnLButtonUp(WPARAM wParam, LPARAM lParam) 
{
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;

	if(m_ThumbDragPos != -1){
		m_ThumbDragPos = -1;
	}
	if(m_ScrollSelected){
		::ReleaseCapture();
		m_ScrollSelected->m_State &= ~SPACE_SELECTED;
		m_ScrollSelected = NULL;
		Invalidate();
	}
    ::KillTimer(m_hWnd,TIMER_SCROLL_INCR);
	::KillTimer(m_hWnd,TIMER_SCROLL_DECR);	
    return ::DefWindowProc(m_hWnd,WM_LBUTTONUP,wParam,lParam);
}

LRESULT CWSScrollView::OnMouseMove(WPARAM wParam, LPARAM lParam) 
{
	POINT point;
	point.x = ((int)(short)LOWORD(lParam));
	point.y = ((int)(short)HIWORD(lParam));
	
	if(m_ThumbDragPos>0){		
		if(m_ScrollBarDraged== H_SCROLL){
			//鼠标实际偏移距离必须根据放大比率作出调整后才能作为滚动距离
			int32 HDocLen     = m_SizeX; 
			int32 HPageLen    = m_HScrollbar.m_SizeX;
			int32 HScrollPos  = m_HScrollbar.m_SizeY;

			int32 w = m_HScrollbar.m_ChildList.front()->m_SizeX;
			int32 len = m_HScrollbar.m_AreaRight-m_HScrollbar.m_AreaLeft;
			int32 ScrollW = len-w-w;

			HScrollPos += (int32)((float)(point.x - m_ThumbDragPos)*(float)HDocLen /(float)ScrollW + 0.5f);
			m_ThumbDragPos = point.x;
			SetHScrollPos(HScrollPos,true);
		}
		else{
			int32 VDocLen    = m_SizeY; 
			int32 VPageLen   = m_VScrollbar.m_SizeX;
			int32 VScrollPos = m_VScrollbar.m_SizeY;

			int32 h = m_VScrollbar.m_ChildList.front()->m_SizeY;
			int32 len = m_VScrollbar.m_AreaBottom-m_VScrollbar.m_AreaTop;
			int32 ScrollH = len-h-h;

			VScrollPos += (int32)((float)(point.y - m_ThumbDragPos)*(float)VDocLen /(float)ScrollH + 0.5f);
            m_ThumbDragPos = point.y;
			//明天把滚动棒的检测与绘制完全隐藏
			//VScrollPos += (float)(point.y - m_ThumbDragPos) *(float)VDocLen /(float)VPageLen;
			SetVScrollPos(VScrollPos,true);
		}
		return 0;
	}	
	return 1;
}

LRESULT CWSScrollView::OnPaint(WPARAM wParam, LPARAM lParam) 
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

	UINT	OldMode	= ::SetBkMode(DCMem,TRANSPARENT);
    
	Draw(DCMem);
    
	if(m_UseSystemScrollbar == false){
		DrawScrollbar(DCMem);
	};
	::SetBkMode(DCMem, OldMode );


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
LRESULT CWSScrollView::OnVScroll(WPARAM wParam, LPARAM lParam) 
{
	assert(m_UseSystemScrollbar);
	UINT  nSBCode = (int) LOWORD(wParam); 
	UINT  nPos    = (short int) HIWORD(wParam);  
	
	RECT rcClient;
	::GetClientRect(GetHwnd(),&rcClient);

	int iScrollBarPos = GetScrollPos(m_hWnd, SB_VERT );
	
	switch( nSBCode )
	{
		case SB_LINEUP:
			iScrollBarPos = max( iScrollBarPos - 20, 0 );
		break;
		case SB_LINEDOWN:
			iScrollBarPos = min( iScrollBarPos + 20, m_SizeY-RectHeight(rcClient) );
		break;
		case SB_PAGEUP:
			iScrollBarPos = max( iScrollBarPos - RectHeight(rcClient), 0 );
		break;
		case SB_PAGEDOWN:
			iScrollBarPos = min( iScrollBarPos + RectHeight(rcClient), m_SizeY-RectHeight(rcClient));
		break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
		{
			iScrollBarPos = (UINT)nPos;
			break;
		}
	}		

	SetScrollPos(m_hWnd, SB_VERT, iScrollBarPos,TRUE);
	Layout(true);
	return 0;
}


LRESULT CWSScrollView::OnHScroll(WPARAM wParam, LPARAM lParam) 
{
	assert(m_UseSystemScrollbar);
	UINT  nSBCode = (int) LOWORD(wParam); 
	UINT  nPos  = (short int) HIWORD(wParam);  
	
	RECT rcClient;
	::GetClientRect(GetHwnd(),&rcClient);

	int iScrollBarPos = GetScrollPos(m_hWnd, SB_HORZ );
	
	switch( nSBCode )
	{
		case SB_LINEUP:
			iScrollBarPos = max( iScrollBarPos - 20, 0 );
		break;
		case SB_LINEDOWN:
			iScrollBarPos = min( iScrollBarPos + 20, m_SizeX-RectWidth(rcClient) );
		break;
		case SB_PAGEUP:
			iScrollBarPos = max( iScrollBarPos - RectWidth(rcClient), 0 );
		break;
		case SB_PAGEDOWN:
			iScrollBarPos = min( iScrollBarPos + RectWidth(rcClient), m_SizeX-RectWidth(rcClient));
		break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
		{
			iScrollBarPos = (UINT)nPos;
			break;
		}
	}		
	SetScrollPos(m_hWnd, SB_HORZ, iScrollBarPos,TRUE);
	Layout(true);
	return 0;
}


} //namespace VISUALSPACE

