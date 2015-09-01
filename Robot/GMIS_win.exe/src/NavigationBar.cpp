// NavigationBar.cpp: implementation of the CNavigationBar class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "NavigationBar.h"
#include "WinSpace2.cpp"
#include "GUIElementMisc.h"
#include "SystemSetting.h"
using namespace VISUALSPACE;



#define  TOOL_CLASSNAME _T("ToolSpace")
#define  IDC_HAND    MAKEINTRESOURCE(32649)

typedef BOOL (WINAPI *lpfnUpdateLayeredWindow)(	HWND hWnd, HDC hdcDst, POINT *pptDst, 
												SIZE *psize,HDC hdcSrc, POINT *pptSrc, 
												COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags );

typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)( HWND hwnd, COLORREF crKey, BYTE xAlpha, DWORD dwFlags );



BOOL RegisterToolClass(HINSTANCE hInstance){
 
	WNDCLASS WndClass;
	if (::GetClassInfo(hInstance, TOOL_CLASSNAME, &WndClass))return TRUE;
	
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	
	wcex.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW |CS_SAVEBITS;
	wcex.lpfnWndProc	= (WNDPROC)SpaceDefaultProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_HAND);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TOOL_CLASSNAME;
	wcex.hIconSm		= NULL;
	
	return RegisterClassEx(&wcex);
}

TurnItem::TurnItem(int32 ID)
:CVSpace2(ID){

}
TurnItem::~TurnItem(){

}
void TurnItem::Draw(HDC hDC,ePipeline* Pipe ){
	
	RECT rc = GetArea();
	tstring s = _T("Turn");
	if(m_State&SPACE_FOCUSED){
		s = _T("Drag");
	}
	DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;
	::DrawText(hDC,s.c_str(),s.size(),&rc,Format);
//    DrawEdge(hDC,rc,RGB(0,255,0));
}

#define DRAW_LINE_H(x,y,l)   \
	{\
	::MoveToEx(hDC,x,y,NULL); \
	::LineTo(hDC,x+l,y);   \
}
#define DRAW_LINE_V(x,y,l)   \
	{\
	::MoveToEx(hDC,x,y,NULL); \
	::LineTo(hDC,x,y+l);   \
}

ForwardItem::ForwardItem(int32 ID)
:CVSpace2(ID){

}
ForwardItem::~ForwardItem(){

}
void ForwardItem::Draw(HDC hDC,ePipeline* Pipe ){
	
	RECT rc = GetArea();
				
	POINT pt;
		
    COLORREF crArrow = RGB(0,128,0);

	if(m_State & SPACE_FOCUSED){
	    crArrow = RGB(0,255,0);
	}

	HPEN   hPen   = ::CreatePen(PS_SOLID,1,crArrow);
	HPEN   hOldPen   = (HPEN)::SelectObject(hDC,hPen);
		
	pt.x = rc.left+ RectWidth(rc)/2;
	pt.y = rc.top+ (RectHeight(rc)-4)/2;
			
	DRAW_LINE_H(pt.x,pt.y,1);
	DRAW_LINE_H(pt.x-1,pt.y+1,3);
	DRAW_LINE_H(pt.x-2,pt.y+2,5);
	DRAW_LINE_H(pt.x-3,pt.y+3,7);
		
	::SelectObject(hDC,hOldPen);	
	::DeleteObject(hPen);

}

BackItem::BackItem(int32 ID)
:CVSpace2(ID){

}
BackItem::~BackItem(){

}
void BackItem::Draw(HDC hDC,ePipeline* Pipe ){
	
	RECT rc = GetArea();
				
	POINT pt;
		
    COLORREF crArrow = RGB(0,128,0);

    if(m_State & SPACE_FOCUSED){
	    crArrow = RGB(0,255,0);
	}

	HPEN   hPen   = ::CreatePen(PS_SOLID,1,crArrow);
	HPEN   hOldPen   = (HPEN)::SelectObject(hDC,hPen);
	
	pt.x = rc.left+ RectWidth(rc)/2;
	pt.y = rc.bottom-(RectHeight(rc)-4)/2;
	
	DRAW_LINE_H(pt.x,pt.y,1);
	DRAW_LINE_H(pt.x-1,pt.y-1,3);
	DRAW_LINE_H(pt.x-2,pt.y-2,5);
	DRAW_LINE_H(pt.x-3,pt.y-3,7);
	
	::SelectObject(hDC,hOldPen);	
	::DeleteObject(hPen);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNavigationBar::CNavigationBar()
{
   m_SizeX = 45;
   m_SizeY = 60;
   m_MoveAcceleration = 2;
   m_MoveSpeed = 0;
   m_MoveTime = 0;
   m_SpaceSelected= NULL;
   	
   CVSpace2*     Title     = new CVSpace2(ID_TITLE); 
   DockboxItem*  Dockbox   = new DockboxItem(ID_DOCKBNT);	
   Dockbox->m_crNormal = RGB(192,192,192);

   TurnItem*     Turn      = new TurnItem(ID_TURN);
   ForwardItem*  ArrowUp   = new ForwardItem(ID_GOAHREAD);
   BackItem*     ArrowDown = new BackItem(ID_GOBACK);

   
   PushChild(Dockbox);
   PushChild(Turn);
   PushChild(ArrowUp);
   PushChild(ArrowDown);
   PushChild(Title);

   m_IsDock = false;
}

CNavigationBar::~CNavigationBar()
{

}

BOOL CNavigationBar::Create(DWORD dwStyle, const RECT& rect, HWND pParentWnd){
	#define WS_EX_LAYERED					0x00080000

	SS.Init();

	m_hWnd = ::CreateWindowEx(0/*WS_EX_LAYERED|WS_EX_TRANSPARENT*/, TOOL_CLASSNAME, NULL, dwStyle,
		rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,
		pParentWnd,
		(HMENU)0,
		GetHinstance(),
		this);
	int n = GetLastError();
/*	
	if(m_hWnd){
		HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
		
		lpfnUpdateLayeredWindow   lpfnUpdate =	(lpfnUpdateLayeredWindow)GetProcAddress( hUser32, _T("UpdateLayeredWindow") );
		lpfnSetLayeredWindowAttributes lpfnSetLayer = (lpfnSetLayeredWindowAttributes)GetProcAddress( hUser32, _T("SetLayeredWindowAttributes") );
		
		if( lpfnUpdate && lpfnSetLayer){
			lpfnSetLayer( GetHwnd(), 0x000000, 160, 0x00000002 );  //ULW_ALPHA=0x00000002,ULW_COLORKEY=0x00000001
		}
	}
*/	
	return m_hWnd!=NULL;
}

void CNavigationBar::MoveTo(int32 x, int32 y){
	m_x = x;
	m_y = y;
	x -= m_SizeX/2;
  	y -= m_SizeY/2;	


	if(m_IsDock ==false){
		::SetWindowPos(GetHwnd(),HWND_TOP,x,y,m_SizeX,m_SizeY,SWP_SHOWWINDOW);
	}
}

void CNavigationBar::Layout(bool Redraw /*= true*/){
   DockboxItem*  Dockbox   = (DockboxItem*)m_ChildList[0];		
   TurnItem*     Turn      = (TurnItem*)m_ChildList[1];
   ForwardItem*    ArrowUp   = (ForwardItem*)m_ChildList[2];
   BackItem*    ArrowDown = (BackItem*)m_ChildList[3];
   CVSpace2*   Title = m_ChildList[4];

   RECT rc = GetArea();
   rc.right-=2;
   rc.left = rc.right-10;
   rc.top  +=3;
   rc.bottom = rc.top+9;
   
   Dockbox->SetArea(rc);

   rc = GetArea();
   rc.right -= 12;
   rc.bottom =rc.top+18;
   Title->SetArea(rc);

   rc = GetArea();
   rc.left+=3;
   rc.right-=3;
   rc.top +=18;
   rc.bottom-=18;
   Turn->SetArea(rc);

   int l = RectWidth(rc)/2;
   rc.top    = rc.bottom+2;
   rc.bottom = m_AreaBottom-2;
   rc.left = m_AreaLeft+4;
   rc.right = rc.left+l;
   ArrowUp->SetArea(rc);

   rc.left  = rc.right;
   rc.right = m_AreaRight-4;
   ArrowDown->SetArea(rc);

};

LRESULT CNavigationBar::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message)
	{
	case WM_PAINT:
		return OnPaint(wParam,lParam);
	case WM_ERASEBKGND:
		return 1;
	case WM_TIMER:
		return OnTimer(wParam,lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam,lParam);
	case WM_SIZE:
		return OnSize(wParam,lParam);
	default:
		return CWinSpace2::Reaction(message,wParam,lParam);
	}
};

LRESULT CNavigationBar::OnSize(WPARAM wParam, LPARAM lParam){
	RECT rc;
	::GetClientRect(GetHwnd(),&rc);
	SetArea(rc);
	Layout();
	return 0;
}
	
LRESULT CNavigationBar::OnPaint(WPARAM wParam, LPARAM lParam){
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

	/*
	if(m_State & SPACE_SHOWBKG){
		FillRect(DCMem,rcViewport,m_crViewBkg);
	}
   */
   
    FillTile(DCMem,SS.m_imgNavibar,rcViewport);

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

	return 0;
}

LRESULT CNavigationBar::OnLButtonDown(WPARAM wParam, LPARAM lParam){
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
    CVSpace2 * Space = Hit(point);	
	
	if(Space){
        if(Space->m_Alias == ID_DOCKBNT){
			m_IsDock = !m_IsDock;
			DockboxItem* DockItem = (DockboxItem*)Space;
			DockItem->m_IsDock = m_IsDock;
			m_SpaceSelected = NULL;
			return 0;
		}
	    if (Space->m_Alias == ID_TITLE)
	    {
			::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTCAPTION,lParam);
			return 0;
	    }
	    m_SpaceSelected = Space;
		Space->m_State |= SPACE_SELECTED;
		int64 ID = Space->m_Alias;

		if(ID == ID_TURN ){
			m_OldMousePos.x = p->x;
			m_OldMousePos.y = p->y;
		}
		::SetCapture(m_hWnd);
		::KillTimer(GetHwnd(),ID_TIMER_MOVE);
		::SetTimer(GetHwnd(),ID_TIMER_MOVE,100,NULL);

	}
	Invalidate();
    return 0;	
};

LRESULT CNavigationBar::OnLButtonUp(WPARAM wParam, LPARAM lParam){
    POINTS* p = (POINTS*)(&lParam);

	//如果鼠标在ADD bnt之外放开则取消ADD的状态	
    if(m_SpaceSelected ){
		m_SpaceSelected->m_State &= ~SPACE_SELECTED;   
			
		::ReleaseCapture();
		::KillTimer(GetHwnd(),ID_TIMER_MOVE);
		
		if (m_SpaceSelected->m_Alias != ID_TURN)
		{
            m_MoveSpeed = 0;
	        m_MoveTime = 0;
		}
		m_SpaceSelected=NULL;
	}
	Invalidate();

    return 0;	
};


LRESULT CNavigationBar::OnTimer(WPARAM wParam, LPARAM lParam){
	if (wParam == ID_TIMER_MOVE)
	{
		if (m_SpaceSelected->m_Alias == ID_TURN)
		{
			POINT p; 	    
			::GetCursorPos(&p);	
			::ScreenToClient(GetHwnd(),&p);
			
			int dx = p.x-m_OldMousePos.x;
			int dy = p.y-m_OldMousePos.y;
			if(dx==0 && dy==0)return 0;
			SendParentMessage(CM_WORLDTRUN,dx,dy,NULL);
		} 
		else
		{
            //计算当前速度，以及根据最新速度将移动距离
			m_MoveTime++;
			m_MoveSpeed += m_MoveAcceleration*m_MoveTime; 
			if(m_MoveSpeed>10)m_MoveSpeed=10;
		    int32 r1 = m_MoveSpeed*m_MoveTime; //这里是整数，3D坐标系统中应该至少除以100
			
			SendParentMessage(CM_WORLDMOVE,m_SpaceSelected->m_Alias,r1,NULL);
		}
	}
	return 0;
}
