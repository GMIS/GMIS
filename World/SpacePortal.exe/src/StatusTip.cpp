// StatusTip.cpp: implementation of the CStatusTip class.
//
//////////////////////////////////////////////////////////////////////


#include "StatusTip.h"
//#include "Win32Tool.h"

CStatusTip::ProgressItem::ProgressItem()
{
	m_Alpha = 255;
	m_Per = 0;
	m_crBorder = RGB(0,0,0);
	m_crThumb = RGB(0,0,130);
}
void CStatusTip::ProgressItem::Draw(HDC hDC,ePipeline* Pipe /*=Pipe*/){
	RECT rc = GetArea();
	if(m_Per >0){
		DrawEdge(hDC,&rc,m_crBorder);
        rc.right = rc.left + RectWidth(rc)*m_Per/100;
		DeflateRect(&rc,1,1,1,1);
		AlphaBlendGlass(hDC,rc.left,rc.top,RectWidth(rc),RectHeight(rc),m_crThumb,m_Alpha);

		COLORREF crOld = ::SetTextColor(hDC,RGB(255,255,255));
		::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_RIGHT|DT_VCENTER);
		::SetTextColor(hDC,crOld);

		if (m_Per == 100)
		{ 
			m_Per = 0;
		}
	}
}

void CStatusTip::ControlBnt::Draw(HDC hDC,ePipeline* Pipe /*=Pipe*/){

	RECT rc = GetArea();
	HFONT GuiFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	HFONT OldFont = (HFONT)::SelectObject(hDC, GuiFont );	
	DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;	
	
	COLORREF crBorder = RGB(0,255,0);
	COLORREF crFkg = RGB(0,0,0);
	COLORREF crBkg = RGB(255,255,255);
		
//	DrawEdge(hDC,m_Area,crBorder);
    DeflateRect(&rc,1,1,1,1);
	COLORREF Oldcr; 
	if(m_State & SPACE_DISABLE){
		FillRect(hDC,&rc,crFkg);
		Oldcr = ::SetTextColor(hDC,crBkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
    else if(m_State & SPACE_WARNING){
		FillRect(hDC,&rc,crFkg);			
		Oldcr = ::SetTextColor(hDC,crBkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);		
	}
	else if(m_State & SPACE_SELECTED){
		DrawEdge(hDC,&rc,crBorder);
		//FillRect(hDC,rc,crFkg);
		Oldcr = ::SetTextColor(hDC,crBkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else if(m_State&SPACE_FOCUSED){
		FillRect(hDC,&rc,crBkg);
		DrawEdge(hDC,&rc,RGB(255,0,0));
		Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else {
		FillRect(hDC,rc,crBkg);
		DrawEdge(hDC,&rc,BDR_RAISEDOUTER|BDR_RAISEDINNER,BF_BOTTOMRIGHT|BF_TOPLEFT);
		COLORREF Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);	
	}
	
    ::SelectObject(hDC,OldFont ); 
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStatusTip::CStatusTip()
{

//	m_crWinBorder  = RGB(0,0,130);
    m_crViewBkg    = RGB(48,101,150);
	m_crText  = RGB(255,255,255);
	m_Font = NULL;
}

CStatusTip::~CStatusTip()
{
	if(m_Font)::DeleteObject(m_Font);
}
	
void   CStatusTip::SetTip(tstring Text){
	if (GetHwnd())
	{
		SendChildMessage(m_hWnd,STATUS_SETTEXT,(WPARAM)Text.c_str(),0);
	}else{
		m_Text = Text;
	}
}
	
void   CStatusTip::SetProgressPer(int32 Per){
	if(GetHwnd()){
		SendChildMessage(m_hWnd,STATUS_SETPROGRESSPOS,(WPARAM)Per,0);
		if(Per == 100)Invalidate();
	}
	else m_ProgressBar.m_Per = Per;
} 
void  CStatusTip::SetProgressAlpha(uint8 Alpha){
		m_ProgressBar.m_Alpha = Alpha;
}
	
void  CStatusTip::SetProgressColor(COLORREF cr){
		m_ProgressBar.m_crThumb = cr;
}



void CStatusTip::SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName )
{
	LOGFONT	LogFont;
	LogFont.lfHeight			= -MulDiv( nHeight, GetDeviceCaps( GetDC(m_hWnd), LOGPIXELSY ), 72 );
	LogFont.lfWidth				= 0;
	LogFont.lfEscapement		= 0;
	LogFont.lfOrientation		= 0;
	LogFont.lfWeight			= ( bBold )? FW_BOLD:FW_DONTCARE;
	LogFont.lfItalic			= (BYTE)( ( bItalic )? TRUE:FALSE );
	LogFont.lfUnderline			= FALSE;
	LogFont.lfStrikeOut			= FALSE;
	LogFont.lfCharSet			= DEFAULT_CHARSET;
	LogFont.lfOutPrecision		= OUT_DEFAULT_PRECIS;
	LogFont.lfClipPrecision		= CLIP_DEFAULT_PRECIS;
	LogFont.lfQuality			= DEFAULT_QUALITY;
	LogFont.lfPitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;

	_tccpy( LogFont.lfFaceName, csFaceName );

	if( m_Font!= NULL )
		::DeleteObject(m_Font);
	
	m_Font = ::CreateFontIndirect( &LogFont );
}



void  CStatusTip::Layout(bool Redraw){
    RECT rc;
    GetClientRect(GetHwnd(),&rc);
	DeflateRect(&rc,4,2,4,2);

	m_ProgressBar.SetArea(rc);
}

LRESULT CStatusTip::ChildReaction(SpaceRectionMsg* SRM){
	switch(SRM->Msg)
	{
	case STATUS_SETTEXT:
		m_Text = (const TCHAR*)SRM->wParam;
		break;
	case STATUS_SETPROGRESSPOS:
		m_ProgressBar.m_Per = (int32)SRM->wParam;
		m_ProgressBar.m_Text = Format1024(_T("%2d%%"),SRM->wParam);
		break;	
	}
	Invalidate();
	return 0;
}
LRESULT CStatusTip::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	if(message == WM_ERASEBKGND)
		return TRUE;
	else if(message == WM_PAINT)
		return OnPaint();
	else if(message == WM_LBUTTONDOWN){
		return OnLButtonDown(wParam,lParam);
	}else if (message == WM_SIZE){
		Layout();
		return 0;
	}
	return CWinSpace2::Reaction(message,wParam,lParam);
}

LRESULT CStatusTip::OnPaint(){

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
	if(m_Font)pOldFont  = (HFONT)::SelectObject(DCMem, m_Font);

    RECT rcText = rcViewport;
	DeflateRect(&rcText,4,1,4,1);
	COLORREF crOld = ::SetTextColor(DCMem,m_crText);
	::DrawText(DCMem,m_Text.c_str(),m_Text.size(),&rcText,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
	      DT_NOPREFIX|DT_VCENTER);	
    ::SetTextColor(DCMem,crOld);

    m_ProgressBar.Draw(DCMem);


	if(pOldFont)::SelectObject(DCMem, pOldFont );
	::SetBkMode(DCMem, OldMode );

	if(m_State & SPACE_SHOWWINBORDER){
	//	DrawEdge(DCMem,rcViewport,m_crWinBorder);// Border
		::DrawEdge(DCMem,&rcViewport,BDR_SUNKENOUTER,BF_RECT);
	}
	
	::BitBlt(hdc, 0, 0, RectWidth(rcViewport), RectHeight(rcViewport), DCMem, 0, 0, SRCCOPY );
	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);
	::EndPaint(m_hWnd, &ps);
	return 0;
}



LRESULT CStatusTip::OnLButtonDown(WPARAM wParam, LPARAM lParam){

	POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	return 0;
}