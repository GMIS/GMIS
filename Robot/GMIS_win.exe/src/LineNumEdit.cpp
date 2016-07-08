// ExeOutput.cpp: implementation of the CExeOutput class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)
#include "GMIS.h"
#include "MsgList.h"
#include "resource.h"
#include "LineNumEdit.h"
#include "SystemSetting.h"
CLineNumber::NumberItem::NumberItem(int64 ID){
	TCHAR buf[20];
	_stprintf(buf,_T("%03i"),int(ID));
	m_Text = buf;
};

void CLineNumber::NumberItem::Draw(HDC hDC, ePipeline* Pipe){
	if(m_Alias != -1){
		::DrawText(hDC,m_Text.c_str(),m_Text.size(),&GetArea(),DT_BOTTOM|DT_SINGLELINE|DT_LEFT);
	}
};
CLineNumber::NumberItem::~NumberItem(){
	int t=0;
};


CLineNumber::CLineNumber()
{
	m_State = 0;
	m_bgcol = RGB( 230, 230, 230 );
	m_fgcol = RGB( 128,128,128);
    m_format = _T("%03i");
//	BntItem*  bnt = new BntItem(10,RGB(0,0,0),RGB(192,192,192),RGB(100,255,100),RGB(128,128,128));
//	PushChild(bnt);
}

CLineNumber::~CLineNumber()
{

}


void CLineNumber::Layout(bool Redraw ){
    RECT rc;
	::GetClientRect(GetHwnd(),&rc);
    rc.top = rc.bottom-18;

//	CVSpace2* Space = m_ChildList.front();
//	Space->m_Area = rc;
};

LRESULT CLineNumber::Reaction(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_PAINT:
		return OnPaint(wParam,lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	case WM_SIZE:
		Layout(false);
		return 0;
	case WM_ERASEBKGND:
		return 1;
	default:
		return CWinSpace2::Reaction(message,wParam,lParam);	
	    break;
	}
}
LRESULT CLineNumber::OnPaint(WPARAM wParam, LPARAM lParam){

	RECT rcViewport; 
	GetClientRect(m_hWnd,&rcViewport);

	if(rcViewport.right==rcViewport.left || rcViewport.top==rcViewport.bottom){
		return 0;
	}
 	
	PAINTSTRUCT ps;				
	HDC hdc = BeginPaint(m_hWnd, &ps);

	HDC DCMem = ::CreateCompatibleDC(hdc);
	HBITMAP bmpCanvas=::CreateCompatibleBitmap(hdc, RectWidth(rcViewport),RectHeight(rcViewport));
	assert(bmpCanvas);
	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, bmpCanvas );

	FillRect(DCMem, rcViewport, m_bgcol );

	// Setting other attributes
	COLORREF crOldText =::SetTextColor(DCMem, m_fgcol );
	COLORREF crOldBk =::SetBkColor( DCMem,m_bgcol );
  //  HFONT OldFont = (HFONT)::SelectObject(DCMem, SS.m_Font12 );	

	// Output the line numbers
	for(uint32 i=0; i<m_ChildList.size(); i++){
		m_ChildList[i]->Draw(DCMem);
	}

    ::SetTextColor(DCMem,crOldText);
	COLORREF crBk = ::SetBkColor(DCMem,crOldBk);

	if(m_State & SPACE_SHOWWINBORDER){
		DrawEdge(DCMem,rcViewport,m_crWinBorder);// Border
	}
//	::SelectObject(DCMem,OldFont);

	::BitBlt(hdc, 0, 0, RectWidth(rcViewport), RectHeight(rcViewport), DCMem, 0, 0, SRCCOPY );
	::SelectObject(DCMem, OldBitmap );
	::SetBkColor(DCMem,crBk);
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);
	::EndPaint(m_hWnd, &ps);
	return 0;    
}

LRESULT CLineNumber::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	POINT point;
    point.x = ((int)(short)LOWORD(lParam));
	point.y = ((int)(short)HIWORD(lParam));
	CVSpace2* Space = Hit(point);
	if (Space)
	{
		int64 ID = Space->m_Alias;
		if(m_ChildList[0] ==Space){
			if(Space->m_State & SPACE_SELECTED){
				Space->m_State &= ~SPACE_SELECTED;
				SendParentMessage(CM_OPENOUTPUTWIN,ID,0,Space);
			}
			else{
				Space->m_State |= SPACE_SELECTED;
				SendParentMessage(CM_CLOSEOUTPUTWIN,ID,0,Space);
			}
			Invalidate();
		}else{
            SendParentMessage(CM_SELECTLINE,(WPARAM)ID,0,NULL);
		}
		return 0;
	}
    return 0;
};



LRESULT COutputEdit::OnRButtonDown(WPARAM wParam, LPARAM lParam){
	POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	POINT pos;
	::GetCursorPos(&pos);
				
	HMENU hmenu;
	HMENU subMenu;
	
	HINSTANCE hInstance = CWinSpace2::GetHinstance();
	hmenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_OUTPUT));
	if(hmenu == NULL){
		return TRUE;
	}
	
	subMenu = GetSubMenu(hmenu, 0); 				
	::SetForegroundWindow(GetHandle()); 
	::TrackPopupMenu(subMenu,0, pos.x, pos.y,0,GetHandle(),NULL);
	::DestroyMenu(hmenu); 
	::PostMessage(GetHandle(),WM_NULL, 0, 0);		

	return FALSE;
}
LRESULT  COutputEdit::OnCommand(WPARAM wParam, LPARAM lParam){
	switch(wParam){
    case ID_OUTPUT_COPY:
		{
            SendMessage(WM_COPY, 0, 0); 
			break;
		}
	case ID_OUTPUT_CLEAR:
		{
			tstring s;
			SetEditText(s);

		
			ePipeline Msg(TO_BRAIN_MSG::GUI_CLEAR_DIALOG_OUTPUT); 
			GetGUI()->SendMsgToBrainFocuse(Msg);
		}
		break;
	case ID_OUTPUT_SAVE:
		{
            SaveAsFile();
		}
	}			
	return 0;
}

BOOL COutputEdit::ProcessWindowMessage(
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		LRESULT& lResult 
		)
{
	lResult = 0;

	switch( uMsg ) {
	   case WM_PAINT:
		   DefWindowProc(uMsg,wParam,lParam);
		   SendParentMessage(CM_UPDATELINE,(WPARAM)this,0);
		   return TRUE;
	   case WM_RBUTTONDOWN:
		   lResult = OnRButtonDown(wParam,lParam);
		   return TRUE;
	   case WM_COMMAND:
		   OnCommand(wParam,lParam);
		   return TRUE;
	   case WM_DESTROY:
		   UnsubclassWindow();
	}
	return CSubclassWnd::ProcessWindowMessage(uMsg,wParam,lParam,lResult);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLineNumEdit::CLineNumEdit()
{
	m_Created = FALSE;
    m_BkgBrush = NULL;
    m_crBk = RGB(255,255,255);
    m_crText  = RGB(0,0,0);
	m_State |= SPACE_SHOWWINBORDER;
}

CLineNumEdit::~CLineNumEdit()
{

}

void CLineNumEdit::AddText(const TCHAR* Text,bool NoChangeVisibleLine/* =true*/){
	HWND EditWin = m_hEdit;

	if(!EditWin && !::IsWindow(EditWin))return ;

	int nFirstLine = (int)::SendMessage( EditWin, EM_GETFIRSTVISIBLELINE, 0, 0 );

	::SendMessage(EditWin,WM_SETREDRAW, false, 0 );

	int	 len = GetWindowTextLength(EditWin);
	::SendMessage(EditWin,EM_SETSEL, len, len);

	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwEffects = 0;
	cf.dwMask =  CFM_COLOR ;

	if (Text)
	{
		cf.crTextColor = m_crText;
		::SendMessage(EditWin,EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		::SendMessage(EditWin,EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)Text); 
		len = GetWindowTextLength(EditWin);
		::SendMessage(EditWin,EM_SETSEL, len, len);

		SendMessage(EditWin,EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)_T("\n"));
	}


	if (len > 125000)
	{
		::SendMessage(EditWin,EM_SETSEL, 0, 50000);
		::SendMessage(EditWin,EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)_T(""));

		len = GetWindowTextLength(EditWin);
		::SendMessage(EditWin,EM_SETSEL, len, len);
	}

	int	nFirstLineNow = (int)::SendMessage( EditWin, EM_GETFIRSTVISIBLELINE, 0, 0 );

	if (NoChangeVisibleLine)
	{
		::SendMessage( EditWin, EM_LINESCROLL, 0, (LPARAM) nFirstLine - nFirstLineNow);
	}else{
		::PostMessage( EditWin,WM_VSCROLL, SB_BOTTOM,0);
	} 


	::SendMessage(EditWin,WM_SETREDRAW, true, 0 );
	::InvalidateRect(EditWin,NULL,FALSE);
}

void CLineNumEdit::ClearText(){
	tstring s;
	m_EditView.SetEditText(s);
};

void CLineNumEdit::SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName )
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

LRESULT CLineNumEdit::ParentReaction(SpaceRectionMsg* SRM){
	switch(SRM->Msg)
	{
	case CM_UPDATELINE:
		DrawLineNumber(m_hEdit);
		break;
	case CM_UPDATEFONT:
		Layout();
		break;
	case CM_SELECTLINE:
		OnSelectLine(SRM->wParam);
	default:
		SendParentMessage(SRM->Msg,SRM->lParam,SRM->wParam,SRM->ChildAffected);
		break;
	}
	
	return 0;
}

LRESULT CLineNumEdit::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	switch(message){
	case WM_CTLCOLORSTATIC:
		return OnCTLColorStatic(wParam,lParam);
		break;
	case WM_GETFONT:
		{
			HFONT font = (HFONT)::SendMessage(m_hEdit, WM_GETFONT, 0, 0);
			return (LRESULT)font;
		}
		break;
	case WM_COMMAND:
		return OnCommand(wParam,lParam);
		break;
	case WM_SIZE:
		return OnSize(wParam,lParam);
		break;
	case WM_CREATE:
		return OnCreate(wParam,lParam);
	default:
		return CWinSpace2::Reaction(message,wParam,lParam);
	}
}

LRESULT CLineNumEdit::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT rcEdit;
	SetRect(&rcEdit,0,0,0,0);
	// Create the edit control
	DWORD style = WS_VISIBLE | WS_CHILD |ES_LEFT |
		ES_MULTILINE|ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_HSCROLL|WS_VSCROLL;
	
	m_hEdit =  CreateWindowEx(
		0,      // extended window style
		_T(RICHEDIT_CLASSA),  // pointer to registered class name
		_T(""), // pointer to window name
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
	assert(m_hEdit != NULL);
	if(m_hEdit==NULL)return -1;

	m_LineView.Create(GetHinstance(),_T("LineView"),WS_CHILD|WS_VISIBLE,rcEdit,GetHwnd(),102);
    m_EditView.SetEditCtrl(m_hEdit);
  
	SetTextFont(12,FALSE,FALSE,_T("Fixedsys"));
    ::SendMessage(m_hEdit,WM_SETFONT, (WPARAM)m_Font, FALSE);

	m_BkgBrush = ::CreateSolidBrush(m_crBk);   
	m_Created = true;

	return 0;	
}
LRESULT CLineNumEdit::OnSize(WPARAM wParam, LPARAM lParam){
    long nWidth  = LOWORD(lParam);  // width of client area 
	long nHeight = HIWORD(lParam);  
	if(nWidth==0 || nHeight ==0 )return 0;
	if(m_Created){
		Layout();
	}
	return 0;
}

LRESULT CLineNumEdit::OnCTLColorStatic(WPARAM wParam,LPARAM lParam){
	HDC hdc = (HDC)wParam;
	::SetTextColor(hdc,m_crText);
    ::SetBkColor(hdc,m_crBk);
 	return (LRESULT)m_BkgBrush;
}

LRESULT CLineNumEdit::OnCommand(WPARAM wParam,LPARAM lParam){
    int wNotifyCode = HIWORD(wParam); // notification code 
    int wID = LOWORD(wParam); 
   ::SendMessage(m_hEdit,wNotifyCode,0,0);
	return 0;
}


void CLineNumEdit::OnSelectLine(int line )
{

	int start =(int)::SendMessage(m_hEdit, EM_LINEINDEX, line, 0);
	int end =  (int)::SendMessage(m_hEdit, EM_LINEINDEX, line + 1, 0);
  
	::SetFocus(m_hEdit);
    ::SendMessage(m_hEdit,EM_SETSEL, start, end-1);
}

int CLineNumEdit::CalcLineNumberWidth()
{
	HDC dc = ::GetDC(m_LineView.GetHwnd());

    TCHAR buf[50];
    int n=0;
	n=_stprintf(buf, m_LineView.m_format.c_str(), m_LineView.m_ChildList.size());
	SIZE fmt;
	::GetTextExtentPoint32(dc, buf, n, &fmt);

	return fmt.cx;
}

void CLineNumEdit::DrawLineNumber(HWND EditWin) 
{
   	if(EditWin==NULL)return;
		
//	if(EditWin != m_CurViewHwnd)return;

	int topline = (int)::SendMessage(EditWin, EM_GETFIRSTVISIBLELINE, 0, 0L) ;

     RECT richEditRect;
     SendMessage( EditWin, EM_GETRECT, 0 , (LPARAM)&richEditRect );

     /*get height of font being used*/

	 HDC hdc = ::GetDC(EditWin);
     TEXTMETRIC tm;
     GetTextMetrics( hdc, &tm );
	 ::ReleaseDC(EditWin, hdc );
     

     /*use height of rich edit control  and font to get the maximum number of lines 
     the edit control can hold, */

     int iMaxNumberOfLines = (RectHeight(richEditRect)/tm.tmHeight); 
    
	 int n = iMaxNumberOfLines - (m_LineView.m_ChildList.size()-1);

	 int i;
	 for (i=0; i<n; i++)
	 {
		 CLineNumber::NumberItem* Item = new CLineNumber::NumberItem(0);
		 m_LineView.PushChild(Item);
	 }

     /*get first visible line, return value is the zero-based index of the uppermost visible line, */
     int iFirstVisibleLine = SendMessage( EditWin, EM_GETFIRSTVISIBLELINE, 0, 0 );

     /*create integers to temporarily hold the line number/char to make the line number*/
     
	 int iLineChar = 0;
     int iLineNumber = 0;
	 TCHAR buf[30];
     
     /*loop cannot run more than the max number of lines in the edit control*/
	 
	 for( i = 0; i <iMaxNumberOfLines; i++ ){
		 
	 /*return value is the character index of the line specified in the wParam parameter, 
	   or it is –1 if the specified line number is greater than the number of lines in the 
	   edit control. */
		 
		 iLineChar = SendMessage( EditWin, EM_LINEINDEX, iFirstVisibleLine+i, 0 );
		 
		 if( iLineChar == -1 )  //break loop if you've gone beyond the last line
		 {
			break; 
		 }	    
		 
		 /*otherwise output line number in the left margin*/
		 else{
			 iLineNumber = SendMessage( EditWin, EM_LINEFROMCHAR, (WPARAM)iLineChar, 0 );
			 _stprintf(buf,m_LineView.m_format.c_str(),iLineNumber);

			 CVSpace2* Space = m_LineView.m_ChildList[i+1];
			 CLineNumber::NumberItem* Item = (CLineNumber::NumberItem*)Space;
			 Item->m_Text = buf;
			 Item->m_Alias   = iLineNumber;
			 POINTL pl;
			 SendMessage( EditWin, EM_POSFROMCHAR, (WPARAM)&pl, (LPARAM)iLineChar );
			 
			 Item->m_AreaRight = 35;                      
			 Item->m_AreaLeft = 0;              
			 Item->m_AreaTop = pl.y;              
			 Item->m_AreaBottom = pl.y+tm.tmHeight+2;  			 
		 }
     }

	 for (int j=i+1; j<m_LineView.m_ChildList.size(); j++)
	 {
		 CLineNumber::NumberItem* Item = (CLineNumber::NumberItem*)m_LineView.m_ChildList[j];
		 Item->m_Alias = -1;
	 }
	 RedrawWindow(m_LineView.GetHwnd(),NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	// m_LineView.Invalidate();
}
	

void CLineNumEdit::Layout(bool Redraw /*= true*/){
	RECT rcClient;
	::GetClientRect(GetHwnd(),&rcClient);
	::InflateRect(&rcClient,-1,-1);
    long nWidth = RectWidth(rcClient);  
	long nHeight = RectHeight(rcClient);  
	int32 LineSize = 0;

	int w = CalcLineNumberWidth();
	::SetWindowPos(m_LineView.GetHwnd(),NULL,rcClient.left,rcClient.top,w,nHeight,SWP_SHOWWINDOW);
	::SetWindowPos(m_hEdit,NULL,rcClient.left+w,rcClient.top,nWidth-w,nHeight,SWP_SHOWWINDOW);
	::SendMessage(m_hEdit,WM_VSCROLL, SB_LINEUP,0); //没办法，只有这样才能通知EDIT恢复显示
	DrawLineNumber(m_hEdit);
}