// StatusBar.cpp: implementation of the CStatusBar class.
//
//////////////////////////////////////////////////////////////////////

#include "MainFrame.h"
#include "StatusBar.h"
#include "format.h"

CStatusBar::ProgressItem::ProgressItem()
{
	m_Alpha = 255;
	m_Per = 0;
	m_crBorder = RGB(0,0,0);
	m_crThumb = RGB(0,0,130);
};

void CStatusBar::ProgressItem::Draw(HDC hDC,ePipeline* Pipe /*=Pipe*/){
	RECT rc = GetArea();
	if(m_Per >0){
		DrawEdge(hDC,&rc,m_crBorder);
        rc.right = rc.left + RectWidth(rc)*m_Per/100;
		DeflateRect(&rc,1,2,1,2);
		AlphaBlendGlass(hDC,rc.left,rc.top,RectWidth(rc),RectHeight(rc),m_crThumb,m_Alpha);

		COLORREF crOld = ::SetTextColor(hDC,RGB(255,255,255));
		rc.right -=4;
		::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_RIGHT|DT_VCENTER);
		::SetTextColor(hDC,crOld);

		if (m_Per == 100)
		{ 
			m_Per = 0;
		}
	}
}

void CStatusBar::ControlBnt::Draw(HDC hDC,ePipeline* Pipe /*=Pipe*/){

	RECT rc = GetArea();
	HFONT GuiFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	HFONT OldFont = (HFONT)::SelectObject(hDC, GuiFont );	
	DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;	
	
	COLORREF crBorder = RGB(192,192,192);
	COLORREF crFkg = RGB(0,0,0);
	COLORREF crBkg = RGB(192,192,192);

	//画文字边框			
//	DrawEdge(hDC,m_Area,crBorder);
    DeflateRect(&rc,1,1,1,1);
	COLORREF Oldcr; 
	if(m_State & SPACE_DISABLE){
		DrawEdge(hDC,&rc,crBorder);
		Oldcr = ::SetTextColor(hDC,crBkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else if(m_State & SPACE_SELECTED){
		DrawEdge(hDC,&rc,crBorder);
		//FillRect(hDC,rc,crFkg);
		Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else if (m_State & SPACE_USING)
	{
		FillRect(hDC,rc,crBkg);
		DrawEdge(hDC,&rc,EDGE_SUNKEN,BF_BOTTOMRIGHT|BF_TOPLEFT);
		COLORREF Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);	
	}else if(m_State&SPACE_FOCUSED){
		FillRect(hDC,rc,crBkg);
		DrawEdge(hDC,&rc,BDR_RAISEDOUTER|BDR_RAISEDINNER,BF_BOTTOMRIGHT|BF_TOPLEFT);

		Oldcr = ::SetTextColor(hDC,crFkg);
		rc.left-=1;
		rc.top -=1;

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

CStatusBar::CStatusBar()
:
 m_BisaiBnt(BNT_TASK_BISAI,_T("比赛")),
 m_DebugBnt(BNT_TASK_DEBUG,_T("调试")),
 m_StopBnt(BNT_TASK_STOP,_T("Stop")),
 m_PauseBnt(BNT_TASK_PAUSE,_T("Pause")),
 m_RunBnt(BNT_TASK_RUN,_T("Run")),
 m_ShuaBnt(BNT_TASK_SHUPENG,_T("刷盆")),
 m_Jiaozheng(BNT_TASK_JIAOZHENG,_T("校正")),
 m_OptionBnt(BNT_TASK_OPTION,_T("选项"))
{

	m_crWinBorder  = RGB(128,128,128);
    m_crViewBkg    = RGB(245,245,243);
	m_crText       = RGB(0,0,0);
	m_Font         = NULL;

#ifdef _WINDLL
	PushChild(&m_OptionBnt);
	PushChild(&m_RunBnt);
	PushChild(&m_PauseBnt);

	m_PauseBnt.m_State |= SPACE_DISABLE;
	m_RunBnt.m_State |= SPACE_DISABLE;
#else
	PushChild(&m_StopBnt);
	PushChild(&m_PauseBnt);
	PushChild(&m_RunBnt);
	PushChild(&m_OptionBnt);

	m_StopBnt.m_State  |= SPACE_DISABLE;
	m_PauseBnt.m_State |= SPACE_DISABLE;

#endif	
	SetTextFont(12,FALSE,FALSE,_T("楷体"));
	/*
#ifdef LOCAL_USE
	PushChild(&m_ShuaBnt);
	PushChild(&m_DebugBnt);
	PushChild(&m_BisaiBnt);
	PushChild(&m_Jiaozheng);
#endif
	*/

	
}

CStatusBar::~CStatusBar()
{
	m_ChildList.clear();
	if(m_Font)::DeleteObject(m_Font);
}
	
void   CStatusBar::SetTip(tstring Text){
	if (GetHwnd())
	{
		SendChildMessage(m_hWnd,STATUS_SETTEXT,(WPARAM)Text.c_str(),0);
	}else{
		m_Text = Text;
	}
}


void   CStatusBar::SetProgressPer(int32 Per){
	if(GetHwnd()){
		SendChildMessage(m_hWnd,STATUS_SETPROGRESSPOS,(WPARAM)Per,0);
		if(Per == 100){
			Invalidate();
		}
	}
	else m_ProgressBar.m_Per = Per;
} 
void  CStatusBar::SetProgressAlpha(uint8 Alpha){
		m_ProgressBar.m_Alpha = Alpha;
}
	
void  CStatusBar::SetProgressColor(COLORREF cr){
		m_ProgressBar.m_crThumb = cr;
}

void   CStatusBar::SetBntState(int32 BntID,uint32 State){

	deque<CVSpace2*>::iterator It = m_ChildList.begin();
	while(It<m_ChildList.end()){
		CVSpace2* Space = (*It);
		if(Space->m_Alias == BntID){
			Space->m_State = State;			
		}
		It++;
	}
	Invalidate();
}

void CStatusBar::SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName )
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

void  CStatusBar::StopTask(){
	m_StopBnt.m_State  = SPACE_DISABLE;
	m_RunBnt.m_State &= ~SPACE_DISABLE;
	m_PauseBnt.m_State = SPACE_DISABLE;
   Invalidate();
}

void   CStatusBar::CloseOptionView(){
	m_OptionBnt.m_State = 0;
	Invalidate();
}

void  CStatusBar::Layout(bool Redraw){
	int32 w = 50;

    RECT rc;
    GetClientRect(GetHwnd(),&rc);
	int left = rc.right-w-5;

	rc.top+=1;
	rc.bottom-=1;

	deque<CVSpace2*>::iterator It = m_ChildList.begin();
	while(It<m_ChildList.end()){
		CVSpace2* Space = (*It);
		Space->m_AreaTop  = rc.top;
		Space->m_AreaBottom = rc.bottom;
		Space->m_AreaLeft = left;
        Space->m_AreaRight = left+w;
		left -= w+5;		
		It++;
	}		
	rc.right = left-5+w;

    m_ProgressBar.SetArea(rc);
	if(Redraw)Invalidate();

}


LRESULT CStatusBar::ChildReaction(SpaceRectionMsg* SRM){
	switch(SRM->Msg)
	{
	case STATUS_SETTEXT:
		m_Text = (const TCHAR*)SRM->wParam;
		break;
	case STATUS_SETPROGRESSPOS:
		m_ProgressBar.m_Per = SRM->wParam;
		m_ProgressBar.m_Text = tformat(_T("%2d%%"),SRM->wParam);
		break;	
	}
	Invalidate();
	return 0;
}
LRESULT CStatusBar::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	if(message == WM_ERASEBKGND)
		return TRUE;
	else if(message == WM_PAINT)
		return OnPaint();
	else if(message == WM_LBUTTONDOWN){
		return OnLButtonDown(wParam,lParam);
	}else if (message == WM_LBUTTONUP)
	{
		return OnLButtonUp(wParam,lParam);
	}else if (message == WM_SIZE){
		Layout();
		return 0;
	}
	return CWinSpace2::Reaction(message,wParam,lParam);
}

LRESULT CStatusBar::OnPaint(){

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
	//::TextOut(DCMem,rcText.left,rcText.top,m_Text.c_str(),m_Text.size());
    ::SetTextColor(DCMem,crOld);

	Draw(DCMem);

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

LRESULT CStatusBar::OnLButtonUp(WPARAM wParam, LPARAM lParam){
	
	POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	::ReleaseCapture();

	CVSpace2* v = Hit(point);
	if(v && v==m_SpaceSelected){

		if (v == &m_RunBnt)
		{
			m_RunBnt.m_State = SPACE_DISABLE;
			m_StopBnt.m_State &= ~SPACE_DISABLE;
			m_PauseBnt.m_State &= ~SPACE_DISABLE;
			
		}else if (v == &m_StopBnt)
        {
			m_StopBnt.m_State  = SPACE_DISABLE;
			m_RunBnt.m_State &= ~SPACE_DISABLE;
			m_PauseBnt.m_State = SPACE_DISABLE;
		}else if (v == &m_PauseBnt)
		{
			m_PauseBnt.m_State = SPACE_DISABLE;
			m_RunBnt.m_State &= ~SPACE_DISABLE;
			m_StopBnt.m_State &= ~SPACE_DISABLE;
		}else if (v == &m_Jiaozheng)
		{

		}
		else
		{
			if(v->m_State & SPACE_USING){
				v->m_State &=~SPACE_USING;
			}else{
			    v->m_State |= SPACE_USING;
			};
		}

		v->m_State &=~SPACE_SELECTED;
		Invalidate();

		SendParentMessage(v->m_Alias,0,0,NULL);
	}
	return 0;
}

LRESULT CStatusBar::OnLButtonDown(WPARAM wParam, LPARAM lParam){

	POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;

	::SetCapture(m_hWnd);
	
	m_SpaceSelected = Hit(point);
	if(m_SpaceSelected){
		if (m_SpaceSelected->m_Alias == 0 || m_SpaceSelected->m_State & SPACE_DISABLE)
		{
			m_SpaceSelected = NULL;
			return 0;
		}

		m_SpaceSelected->m_State |= SPACE_SELECTED;
		Invalidate();
	}
	return 0;
}