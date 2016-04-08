// AddressBar.cpp: implementation of the CAddressBar class.
//
//////////////////////////////////////////////////////////////////////

#include "MainFrame.h"
#include "AddressBar.h"

BOOL CAddressBar::CAddressEdit::ProcessWindowMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
{
/*		
	if(uMsg == WM_KILLFOCUS){
	    SendParentMessage(CM_KILLFOCUS,0,0);
		return TRUE;
	}
*/
	return CEditWin::ProcessWindowMessage( uMsg, wParam, lParam, lResult );
}

		
CAddressBar::CBnt::CBnt(int64 ID,tstring text)
			:ButtonItem(ID,text.c_str(),NULL,false)
{
	
};

CAddressBar::CBnt::~CBnt()
{

};

void CAddressBar::CBnt::Draw(HDC hDC, ePipeline* Pipe){

    COLORREF crText       = RGB(91,91,91);

 	DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;	

	RECT rc = GetArea();
	
	FillRectGlass(hDC,&rc,RGB(192,192,200),FALSE,127);
   
	if (m_State & SPACE_DISABLE)
    {
		COLORREF Oldcr = ::SetTextColor(hDC,RGB(180,150,150));
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
		//Draw3DEdge(hDC,&rc);
		::DrawEdge(hDC,&rc,BDR_SUNKENOUTER,BF_RECT);
    }
	else if(m_State & SPACE_SELECTED){
		COLORREF Oldcr = ::SetTextColor(hDC,crText);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
		//Draw3DEdge(hDC,&rc);
		::DrawEdge(hDC,&rc,BDR_SUNKENOUTER,BF_RECT);
        
	}
    else if(m_State & SPACE_WARNING || m_State & SPACE_FOCUSED){	
		COLORREF Oldcr = ::SetTextColor(hDC,RGB(0xFF,0x1A,0));
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
		::DrawEdge(hDC,&rc,BDR_RAISEDOUTER,BF_RECT);
	}
	else {
		COLORREF Oldcr = ::SetTextColor(hDC,crText);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
		::DrawEdge(hDC,&rc,BDR_RAISEDOUTER,BF_RECT);
	}	

//	::SelectObject(hDC,OldFont ); 
}


CAddressBar::SeparatorLine::SeparatorLine(int32 ID,int32 nline /*= 1*/)
		:m_NumLine(nline),m_bVerDraw(FALSE){

	m_crDark  = RGB(13,13,13);
	m_crLight = RGB(120,120,120);
};

void CAddressBar::SeparatorLine::Draw(HDC hDC, ePipeline* Pipe){
	
	RECT rc = GetArea();
	int32 pading = 2;
	if (m_bVerDraw)
	{
		rc.top   +=pading;
		rc.bottom-=pading;

		int w = m_NumLine*(2);

		rc.left  = rc.left+(RectWidth(rc)-w)/2;
		

		
		for (int i=0; i<m_NumLine; i++)
		{
			rc.right = rc.left+1;
			//::DrawEdge(hDC,&rc,BDR_RAISEDINNER,BF_RECT);	
			FillRect(hDC,rc,m_crDark);
			rc.left = rc.right;
            rc.right = rc.left+1;
	        FillRect(hDC,rc,m_crLight);
			rc.left = rc.right+2;
		}
		

	}else{
		rc.left   +=pading;
		rc.right  -=pading;

		int h = m_NumLine*(2);

		rc.top  = rc.top+(RectHeight(rc)-h)/2;
		
		for (int i=0; i<m_NumLine; i++)
		{
			rc.bottom = rc.top+1;
			//::DrawEdge(hDC,&rc,BDR_RAISEDINNER,BF_RECT);
			FillRect(hDC,rc,m_crDark);
			//rc.top = rc.bottom;
            //rc.bottom = rc.top+1;
	        //FillRect(hDC,rc,RGB(64,64,64));
			rc.top = rc.bottom;
            rc.bottom = rc.top+1;
	        FillRect(hDC,rc,m_crLight);
			rc.top = rc.bottom+2;
		}
	}	
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAddressBar::CAddressBar()
:m_bConnected(FALSE),
 m_ConnectBnt(CM_CONNECT,_T(""))
{

 PushChild(&m_ConnectBnt);
}

CAddressBar::~CAddressBar()
{
	m_ChildList.clear();
}

void  CAddressBar::SetText(tstring text){
	
	::SendMessage(m_AddressEdit,WM_SETTEXT,0,(LPARAM)text.c_str());
	
}
void  CAddressBar::SetConnectState(BOOL success){
	if (success)
	{
		m_ConnectBnt.m_Name = _T("Break");
        m_ConnectBnt.m_Alias = CM_DISCONNECT;
		m_ConnectBnt.m_State = 0;
		m_bConnected = TRUE;
//		MoveWindow(m_AddressEdit.GetHandle(),0,0,0,0,FALSE);
		MoveWindow(m_AddressEdit,0,0,0,0,FALSE);

	}else
	{
		m_ConnectBnt.m_Name = _T("Connect");
        m_ConnectBnt.m_Alias = CM_CONNECT;
		m_bConnected = FALSE;
//		m_AddressEdit.SetReadOnly(FALSE);
		SendMessage(m_AddressEdit,WM_ENABLE,FALSE,0);
		m_ConnectBnt.m_State = 0;
		Layout();
	}
	Invalidate();
}

void  CAddressBar::Layout(bool Redraw){
    RECT rc;
    GetClientRect(GetHwnd(),&rc);
	int left = rc.left;

	rc.top+=2;
	rc.bottom-=2;

	m_ConnectBnt.m_AreaTop    = rc.top+2;
	m_ConnectBnt.m_AreaLeft   = rc.right-60;
	m_ConnectBnt.m_AreaBottom = rc.bottom-2;
	m_ConnectBnt.m_AreaRight  = rc.right-3;

	rc.left = left+1;
 	rc.right -=62;

	if(m_AddressEdit){
		::MoveWindow(m_AddressEdit,rc.left,rc.top,rc.right-rc.left,100,TRUE);
	}
	if(Redraw)Invalidate();

}


LRESULT CAddressBar::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	if(message == WM_ERASEBKGND)
		return TRUE;
	else if(message == WM_PAINT)
	    return OnPaint();
	else if (message == WM_LBUTTONUP)
	{
		return OnLButtonUp(wParam,lParam);
	}
	else if(message == WM_LBUTTONDOWN)
		return OnLButtonDown(wParam,lParam);
	else if (message == WM_SIZE)
	{
		Layout();
		return 0;
	}
	else if (message == WM_CREATE)
	{
		return OnCreate(wParam,lParam);
	}
	else return ::DefWindowProc(m_hWnd,message,wParam,lParam);
}

LRESULT CAddressBar::OnPaint(){

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

	HFONT	pOldFont=NULL; 

	if (m_bConnected)
	{
		m_ConnectBnt.m_Name = _T("Break");
		m_ConnectBnt.Draw(DCMem);
	}else{
		m_ConnectBnt.m_Name = _T("Connect");
		m_ConnectBnt.Draw(DCMem);
	}


	if(pOldFont)::SelectObject(DCMem, pOldFont );
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


LRESULT CAddressBar::OnLButtonUp(WPARAM wParam, LPARAM lParam) 
{
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
    CVSpace2 * Space = Hit(point);	
	if(Space){
		int64 ID = Space->m_Alias;
		if(ID == CM_CONNECT && m_SpaceFocused==Space){
			::ReleaseCapture();
			Space->m_State = SPACE_DISABLE;
			//m_AddressEdit.SetReadOnly(TRUE);
            ::SendMessage(m_AddressEdit,WM_ENABLE,TRUE,0);  
		    SendParentMessage(CM_CONNECT,ID,0,Space); 
		}else if (ID == CM_DISCONNECT)
		{
		    SendParentMessage(CM_DISCONNECT,ID,0,Space); 
		}

	};
	
	//如果鼠标在bnt之外放开则取消的状态	
    Space = m_ChildList.front();
    if(Space->m_State & SPACE_SELECTED){
		::ReleaseCapture();
		Space->m_State = 0;   
	}

	Invalidate();

    return 0;	
}
LRESULT CAddressBar::OnLButtonDown(WPARAM wParam, LPARAM lParam){

	POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;	
	CVSpace2* Space = Hit(point);
	if(Space){
        if (Space->m_Alias ==CM_CONNECT || Space->m_Alias ==CM_DISCONNECT )
        {
		 	Space->m_State |= SPACE_SELECTED;
			Invalidate();
			::SetCapture(m_hWnd);
		}
	}
	return 0;
}


LRESULT CAddressBar::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT rcEdit;
	::GetClientRect(m_hWnd,&rcEdit);
	// Create the edit control
//	DWORD style = WS_VISIBLE | WS_CHILD |ES_LEFT | ES_WANTRETURN|WS_EX_STATICEDGE|WS_BORDER;
 
	DWORD style = WS_VISIBLE | WS_CHILD |CBS_AUTOHSCROLL|CBS_DROPDOWN;//|CBS_HASSTRINGS|WS_EX_STATICEDGE|WS_BORDER;
	
	m_AddressEdit =  CreateWindowEx(
		0,//WS_EX_STATICEDGE,      // extended window style
		_T("COMBOBOX"),  // pointer to registered class name
		_T("About:blank"), // pointer to window name
		style,        // window style
		rcEdit.left,                // horizontal position of window
		rcEdit.top,                // vertical position of window
		RectWidth(rcEdit),           // window width
		RectHeight(rcEdit),          // window height
		m_hWnd,      // handle to parent or owner window
		(HMENU)100,          // handle to menu, or child-window identifier
		GetHinstance(),  // handle to application instance
		NULL       // pointer to window-creation data
		);
	assert(m_AddressEdit != NULL);
	if(m_AddressEdit==NULL)return -1;
    TCHAR* s1 =_T("about:blank");

	SendMessage(m_AddressEdit,WM_SETTEXT,0,(LPARAM)s1);
	TCHAR* s2 = _T("http://apps.renren.com/boyaa_texas/index.php?origin=103");
	TCHAR* s3 = _T("http://game.weibo.com/texaspoker?origin=1002");

	//::SendMessage(m_AddressEdit,CB_ADDSTRING,0,(LPARAM)s1);
	::SendMessage(m_AddressEdit,CB_ADDSTRING,0,(LPARAM)s2);
	::SendMessage(m_AddressEdit,CB_ADDSTRING,0,(LPARAM)s3);

//	m_AddressEdit.SetEditCtrl(hEdit);

//	tstring s = _T("about:blank");
//    m_AddressEdit.SetEditText(s);
	return 0;
}