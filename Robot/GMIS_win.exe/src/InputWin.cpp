// InputWin.cpp: implementation of the CInputEdit class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786) 

#include "GMIS.h"
#include "MainFrame.h"
#include "InputWin.h"
#include <sstream>
#include <malloc.h>
#include "LinkerPipe.h"
#include "GUIElementMisc.h"
#include "LinkerView.h"
#include "MsgList.h"
#include "resource.h"
#include "LogicDialog.h"

#pragma  comment(lib, "imm32.lib")

/*
from: http://www.codeguru.com/forum/showthread.php?t=446242
<rich edit control line numbering number> 
*/
void CInputEdit::DrawLineNumber(HDC hdc ){

	/*get height of rich edit so that the height of the rich edit control can be calculated*/
	
	RECT richEditRect;
	SendMessage(  EM_GETRECT, 0 , (LPARAM)&richEditRect );
	int iRectHeight = richEditRect.bottom - richEditRect.top;  //calculate height
	
	RECT rcClient;
	::GetClientRect(GetHandle(),&rcClient);
	rcClient.right = richEditRect.left;
	FillRect(hdc,rcClient,m_crLineBk);
    
	/*get height of font being used*/
	
	TEXTMETRIC tm;
	GetTextMetrics( hdc, &tm );
	
	
	/*use height of rich edit control  and font to get the maximum number of lines 
	the edit control can hold, */
	
	int iMaxNumberOfLines = (iRectHeight/tm.tmHeight); 
	
	
	/*get first visible line, return value is the zero-based index of the uppermost visible line, */
	
	int iFirstVisibleLine = SendMessage( EM_GETFIRSTVISIBLELINE, 0, 0 );
	
	
	/*create integers to temporarily hold the line number/char to make the line number*/
	
	int iLineChar = 0;
	int iLineNumber = 0;
	
	TCHAR buf[30];
	
	COLORREF crOld = ::SetTextColor(hdc,m_crLineNumber);
		
//	HFONT OldFont = (HFONT)::SelectObject(hdc,SS.m_SystemFont);	


		/*loop cannot run more than the max number of lines in the edit control*/
	
	for( int c = 0; c <= iMaxNumberOfLines; c++ ){
		
	/*return value is the character index of the line specified in the wParam parameter, 
	or it is –1 if the specified line number is greater than the number of lines in the 
		edit control. */
		
		iLineChar = SendMessage( EM_LINEINDEX, (iFirstVisibleLine + c ), 0 );
		
		if( iLineChar == -1 )  //break loop if you've gone beyond the last line
			break;    
		
		/*otherwise output line number in the left margin*/
		else{
			iLineNumber = SendMessage( EM_LINEFROMCHAR, (WPARAM)iLineChar, 0 );
			/* std::ostringstream strLineIndex;
			strLineIndex<<(iLineNumber + 1);
			*/
			_stprintf(buf,_T("%02i"),(iLineNumber));
			tstring strLineIndex = buf;
			POINTL pl;
			SendMessage(  EM_POSFROMCHAR, (WPARAM)&pl, (LPARAM)iLineChar );
			
			RECT tmpRect;
			tmpRect.right = 20;                      //right border is 55 (rich edit control left border is 70 so there is a gap of 15)
			tmpRect.left = 0;              //left border is flush with edge of window
			tmpRect.bottom = richEditRect.bottom;  //bottom is same as rich edit controls bottom
			tmpRect.top = pl.y;              //top is the y position of the characters in that line number
			
			DrawText( hdc, strLineIndex.c_str(), strLineIndex.size(), &tmpRect, DT_SINGLELINE|DT_CENTER );
		}
	}
	::SetTextColor(hdc,crOld);
//	::SelectObject(hdc,OldFont ); 

}


CInputEdit::CInputEdit()
{
   m_LeftMargin = 20;
   m_crLineNumber = RGB(128,128,128);
   m_crLineBk     = RGB(245,245,243);
   m_crTip        = RGB(0,192,0);
   m_crText       = RGB(0,0,0);
   m_bForbidSend  = false;
   m_EditBeginPos = 0;

}

CInputEdit::~CInputEdit()
{
}


void CInputEdit::SetInputTip(tstring Tip){
	 if (m_InputTip == Tip)
	 {
		 return;
	 }
	 m_InputTip =  Tip;
	 
	 CHARRANGE oldcr;	
	 GetSel(oldcr);
	 
	 CHARRANGE cr;	
	 CHARFORMAT cf;
	 cf.cbSize = sizeof(CHARFORMAT);
	 cf.dwEffects = 0;
	 cf.dwMask =  CFM_COLOR ;
	 cf.crTextColor = m_crTip;
	 if (m_EditBeginPos>0)
	 {
	     cr.cpMin = 0;
		 cr.cpMax = m_EditBeginPos;
	 }else{
		 cr.cpMax=0;
		 cr.cpMin=0;

	 }
	 SetSel(cr);
//	 ::SendMessage(GetHandle(),EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	 ReplaceSel(m_InputTip);	
	 GetSel(cr);


	 int d = cr.cpMin-m_EditBeginPos;
 	 m_EditBeginPos = cr.cpMin;
	 cf.crTextColor = m_crText;
//    ::SendMessage(GetHandle(),EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

	 //恢复之前光标位置
	 oldcr.cpMin += d;
	 oldcr.cpMax += d;
	 SetSel(oldcr);
}


void  CInputEdit::SetEditText(tstring& Text){
	SendMessage(EM_HIDESELECTION, TRUE,TRUE);

	CHARRANGE cr;
	cr.cpMin =  m_EditBeginPos;
	cr.cpMax = -1;
	SetSel(cr);

    ReplaceSel(Text);
}
tstring CInputEdit::GetEditText(bool Clear){
	SendMessage(EM_HIDESELECTION, TRUE,TRUE);

	CHARRANGE cr;
	cr.cpMin =  m_EditBeginPos;
	cr.cpMax = -1;
	SetSel(cr);

	EDITSTREAM es;
	tstring  HisStr;
	es.dwCookie = (DWORD)&HisStr;
	es.pfnCallback = StreamOutCallback;
	SendMessage(EM_STREAMOUT, SF_TEXT |SFF_SELECTION|SF_UNICODE, (LPARAM)&es);   


	if (Clear)
	{
		cr.cpMin =  0;
		cr.cpMax = -1;
		SetSel(cr);
		tstring s;
		ReplaceSel(s);
		m_EditBeginPos = 0;
	}else{
		cr.cpMax = -1;
		cr.cpMin = -1;
		SetSel(cr);
	}
	CHARFORMAT cf;
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwEffects = 0;
	cf.dwMask =  CFM_COLOR ;
	cf.crTextColor = m_crText;
	::SendMessage(GetHandle(),EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

	SendMessage(EM_HIDESELECTION, FALSE,FALSE);   
	return HisStr;
}

BOOL CInputEdit::IsEditingArea(int CharPos)
{
	if(CharPos == -1){
		CHARRANGE cr;
		GetSel(cr);
		if(cr.cpMax<m_EditBeginPos)return FALSE;
		else if(cr.cpMin < m_EditBeginPos){
			cr.cpMin = m_EditBeginPos;
			SetSel(cr); //剔除无效选择字符
		}
	}
	else if(CharPos<m_EditBeginPos)return FALSE;
		
	return TRUE;
};

void CInputEdit::DeleteSelText(CHARRANGE& cr)
{
    int n = cr.cpMax - cr.cpMin;
    assert(n>0);

	ePipeline NewMsg;
	NewMsg.PushInt(cr.cpMin-m_EditBeginPos);
	tstring s;
    while(n-->0)
    {
		s+=127;
    }
	NewMsg.PushString(s);
	m_Parent->OnInputText(0, (LPARAM)&NewMsg);
};




void CInputEdit::OnMenuCopy(){
	SendMessage(WM_COPY, 0, 0); 
}
void CInputEdit::OnMenuCut(){
	CHARRANGE cr;
    GetSel(cr);
	if(cr.cpMin<m_EditBeginPos)return; //不在可编辑范围

	if (cr.cpMax != cr.cpMin)
	{
		DeleteSelText(cr);
		SendMessage(WM_CUT, 0, 0);
	}
}

void CInputEdit::OnMenuPaste(){
	if(!::IsClipboardFormatAvailable (CF_TEXT))return;
	CHARRANGE cr;
    GetSel(cr);

	if(cr.cpMin<m_EditBeginPos)return; //不在可编辑范围

	
	ePipeline  NewMsg;

	//注意：当粘贴覆盖已有选择时，大脑可以一步完成，此时NewMsg的坐标是覆盖范围
    NewMsg.PushInt(cr.cpMin-m_EditBeginPos);
//	NewMsg.PushInt64(cr.cpMax-cr.cpMin);
	 
	int32 BeginPos  = cr.cpMin;
	SendMessage(WM_PASTE, 0, 0); 

    GetSel(cr);
    assert(cr.cpMin == cr.cpMax);
	int32 EndPos = cr.cpMax;

	
    int n = EndPos - BeginPos;
	
	if(n==0)return;
	n++;
 	LPTSTR lpsz = (TCHAR*)_alloca((n+1)*sizeof(TCHAR));

	cr.cpMin = BeginPos;
	cr.cpMax = EndPos;
	SetSel(cr);

	CHARFORMAT cf;
	SendMessage(EM_GETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	cf.crTextColor = RGB(0,0,0);

	SendMessage(EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf);

	SendMessage(EM_GETSELTEXT, 0, (LPARAM)lpsz);
	lpsz[n] = _T('\0');

	NewMsg.PushString(lpsz);

	cr.cpMin = EndPos;
	cr.cpMax = EndPos;
	SetSel(cr);

	m_Parent->OnInputText(0, (LPARAM)&NewMsg);
	m_Parent->EnableInput(FALSE);
}

void CInputEdit::OnMenuSend(){
	m_bForbidSend = true;
	m_Parent->OnInputEnd();
}
void CInputEdit::OnMenuThink(){	
	ePipeline Msg(GUI_GET_THINK_RESULT);  
	
	GetGUI()->SendMsgToBrainFocuse(Msg);
}

void CInputEdit::OnMenuAnalyse(){
	ePipeline Msg(GUI_GET_ANALYSE_RESULT);
		
	GetGUI()->SendMsgToBrainFocuse(Msg);
}
void CInputEdit::OnMenuClear(){
	
	ePipeline Msg(GUI_CLEAR_THINK); 
	GetGUI()->SendMsgToBrainFocuse(Msg);
}
void CInputEdit::OnDestroy()
{
	UnsubclassWindow();
}

BOOL CInputEdit::ProcessWindowMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
{
	lResult = 0;
		
	if(uMsg == WM_PAINT){
		   lResult = DefWindowProc(uMsg,wParam,lParam);
		   HDC hdc = GetDC( GetHandle() );
		   DrawLineNumber( hdc );
		   ReleaseDC( GetHandle(), hdc );
		   return TRUE;
	}

	switch (uMsg)
	{

	case WM_KEYDOWN:
		{
			TCHAR  nChar = (TCHAR) wParam;
		    CHARRANGE cr;
	        GetSel(cr);
			int32 pos = cr.cpMin;
			
            if((wParam == _T('v')||wParam==_T('V')) && HIWORD(GetKeyState(VK_CONTROL))){
				OnMenuPaste();
				return TRUE;
			}else if((wParam == _T('x')||wParam==_T('X')) && HIWORD(GetKeyState(VK_CONTROL))){
				OnMenuCut();
				return TRUE;
			}else if((wParam == _T('z')||wParam==_T('Z')) && HIWORD(GetKeyState(VK_CONTROL))){
				return TRUE;
			}
						
			if (wParam == VK_DELETE){
				if(pos<=m_EditBeginPos)return TRUE; //不承认有输入

				CHARRANGE cr;
				GetSel(cr);
				int32 BackCharNum = max(1,cr.cpMax-cr.cpMin);
				ePipeline NewMsg;
				tstring s;
				NewMsg.PushInt(cr.cpMin-m_EditBeginPos);
				//NewMsg.PushInt64(BackCharNum);
				s.resize(BackCharNum,127);
				NewMsg.PushString(s);
				m_Parent->OnInputText(0, (LPARAM)&NewMsg);	
				return FALSE;
			}
			if (wParam == VK_BACK)
			{
				if (pos<=m_EditBeginPos)
				{
					return TRUE;
				}

				if (cr.cpMin != cr.cpMax)
				{
					DeleteSelText(cr);
				}else {
                    tstring Text;
				    Text.resize(1,127);
					ePipeline NewMsg;
					NewMsg.PushInt(cr.cpMin-1-m_EditBeginPos);
					//NewMsg.PushInt64(1);
					NewMsg.PushString(Text.c_str());
				    m_Parent->OnInputText(0, (LPARAM)&NewMsg);	
				}
				return FALSE;
			}
			
			if (! IsEditingArea(pos))
			{

				if(cr.cpMin==cr.cpMax ){ //自动默认在最后位置输入
					int32 len = GetTextLength();
					cr.cpMax = len; cr.cpMin = len;
					SetSel(cr);
					pos = len;
				}
                return TRUE; //否则不承认有输入
			}
		}
		return FALSE;

	case WM_IME_COMPOSITION:
		{
			HWND hWnd=GetHandle();
			if(lParam & GCS_RESULTSTR)
			{
				HIMC hIMC = ImmGetContext(hWnd);
				if (!hIMC)
				{
					return FALSE;	
				}
				
				int32 n = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
				
				n += sizeof(WCHAR);
				
				TCHAR buf[255];
				memset(buf, 0, 255);
				
				ImmGetCompositionString(hIMC, GCS_RESULTSTR, buf, n);
				
		        ImmReleaseContext(hWnd, hIMC);

				CHARRANGE cr;
				GetSel(cr);
				if (cr.cpMin< m_EditBeginPos)
				{
					return FALSE;
				}
				
				if (cr.cpMin != cr.cpMax)
				{
					DeleteSelText(cr);
				};
				
				int32 pos = cr.cpMin-m_EditBeginPos;
				assert(pos>=0);

				tstring text = buf;
				ePipeline NewMsg;
				NewMsg.PushInt(pos);
				//NewMsg.PushInt64(text.size());
				NewMsg.PushString(text.c_str());
				m_Parent->OnInputText(0, (LPARAM)&NewMsg);	

			}
		}
		break;
	case WM_CHAR:
		{	
		
			TCHAR  nChar = (TCHAR) wParam;

		    CHARRANGE cr;
	        GetSel(cr);
			if (cr.cpMin< m_EditBeginPos)
			{
				return FALSE;
			}

			if (cr.cpMin != cr.cpMax)
			{
				DeleteSelText(cr);
			};

			int32 pos = cr.cpMin-m_EditBeginPos;
            assert(pos>=0);

			//string s = format("WM_CHAR GetEdit Pos: %d",pos);
            //AddRTInfo(s.c_str());

			tstring Text;
			Text=nChar;
			if (nChar == VK_RETURN)
			{
				--pos;
				ePipeline NewMsg;
				NewMsg.PushInt(pos);
				//NewMsg.PushInt64(1);
				NewMsg.PushString(Text.c_str());
				m_Parent->OnInputText(0, (LPARAM)&NewMsg);	
			} 
			else if(nChar == VK_BACK ){ //DeleteSelText已经处理过cr.cpMax != cr.cpMin的情况
			    return FALSE;
				/*注意：VK_BACK实际上是按VK_DELETE来处理的
				举例：在"defi"的末尾pos=4按VK_BACK将被视为在i的pos=3按VK_DELETE
				因为此时的pos在OnkeyDown（）中已经由pos=4被改变了pos=3，所以这里不必再-1
				*/ 
				ePipeline NewMsg;
				NewMsg.PushInt(pos);
				//NewMsg.PushInt64(1);
				NewMsg.PushString(Text.c_str());
				m_Parent->OnInputText(0, (LPARAM)&NewMsg);	
			}
			else if(nChar != 0x16 && nChar != 0x18 && nChar != 0x1a && nChar !=VK_ESCAPE){
				Text = nChar;
				ePipeline NewMsg;
				NewMsg.PushInt(pos);
				//NewMsg.PushInt64(1);
				NewMsg.PushString(Text.c_str());
				m_Parent->OnInputText(0, (LPARAM)&NewMsg);	

			}
			return FALSE;
		}
	case WM_RBUTTONDOWN:
		{

			POINT pos;
			::GetCursorPos(&pos);

			HMENU hmenu;
			HMENU subMenu;
		
			HINSTANCE hInstance = CWinSpace2::GetHinstance();
			hmenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_RICHEDITMENU));
			if(hmenu == NULL){
				int32 n = ::GetLastError();
				return TRUE;
			}
				
			subMenu = GetSubMenu(hmenu, 0); 
			if(m_bForbidSend){
				EnableMenuItem(subMenu,ID_MENU_SEND,FALSE);
			}
			::SetForegroundWindow(GetHandle()); 
			::TrackPopupMenu(subMenu,0, pos.x, pos.y,0,GetHandle(),NULL);
			::DestroyMenu(hmenu); 
			PostMessage(WM_NULL, 0, 0);

            return FALSE;

		}
	case WM_INITMENU:
		{
			::SetCursor(::LoadCursor(NULL,IDC_ARROW));
			break;
		}
	case WM_COMMAND:
		{
			switch(wParam)
			{
			case ID_MENU_COPY:
				OnMenuCopy();
				break;
			case ID_MENU_CUT:
				OnMenuCut();
				break;
			case ID_MENU_PASTE:
				OnMenuPaste();
				break;
			case ID_MENU_SEND:
				OnMenuSend();
				break;
			case ID_MENU_ANALYSE:
				OnMenuAnalyse();
				break;
			case ID_MENU_THINK:
				OnMenuThink();
				break;
			case ID_CLEAR_THINK:
				OnMenuClear();
				break;
			}
		}
		return FALSE;
	case WM_ENABLE:
		{
		BOOL fEnabled = (BOOL) wParam;
		if(fEnabled){
			::SetFocus(GetHandle());
		}
		}
		break;
	case WM_DESTROY:
			OnDestroy();
			return FALSE;
	}

	return CSubclassWnd::ProcessWindowMessage( uMsg, wParam, lParam, lResult );
}

///////////////////////////////////////////////////////

CInputWin::Bnt::Bnt(int64 ID,TCHAR* Name)
:CVSpace2(ID),m_Name(Name){
	
};

CInputWin::Bnt::~Bnt()
{
	
};

void CInputWin::Bnt::Draw(HDC hDC, ePipeline* Pipe/* =NULL */)
{
    COLORREF crText = RGB(0,0,0);
	
	DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;	
	
	//	HFONT OldFont = (HFONT)::SelectObject(hDC,SS.m_SystemFont);	
	
	RECT rc = GetArea();
	
	if(m_State & SPACE_SELECTED){
		FillRect(hDC,rc,RGB(192,220,255));
		//DrawEdge(hDC,rc,RGB(128,128,128));
		COLORREF Oldcr = ::SetTextColor(hDC,crText);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
		::DrawEdge(hDC,&rc,BDR_SUNKENOUTER,BF_RECT);
		//Draw3DEdge(hDC,&rc);
	}
    else if(m_State & SPACE_WARNING || m_State & SPACE_FOCUSED){	
		FillRect(hDC,rc,RGB(192,220,255));
		DrawEdge(hDC,rc,RGB(196,196,196));
		COLORREF Oldcr = ::SetTextColor(hDC,crText);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else {
		DrawEdge(hDC,rc,RGB(196,196,196));
		COLORREF Oldcr = ::SetTextColor(hDC,crText);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}	
	
	//	::SelectObject(hDC,OldFont ); 
};

CInputWin::Toolbar::Toolbar(HANDLE Image){
	m_Image = Image;
	
//	Bnt* BntDebug  = new Bnt(BNT_DEBUG,_T("Debug"));
	Bnt* BntSend   = new Bnt(BNT_SEND,_T("Send"));
	
	
//	PushChild(BntDebug);
	PushChild(BntSend);
}

CInputWin::Toolbar::~Toolbar(){};

void CInputWin::Toolbar::Layout(bool Redraw){
	RECT rc = GetArea();
	
	Bnt* BntSend = (Bnt*)m_ChildList.back(); 
    BntSend->SetArea(rc.left+1,rc.bottom-40,rc.right-1,rc.bottom);
    
	int32 top = rc.top; 
	int32 bottom  = rc.bottom-40;
	
    for (int i=0; i<m_ChildList.size()-1 && top<bottom;i++)
    {
		CVSpace2* Bnt = (CVSpace2*)m_ChildList[i];
		
		Bnt->SetArea(rc.left,top,rc.right,top+20);
		top += 19;
    }
}

void CInputWin::Toolbar::Draw(HDC hDC, ePipeline* Pipe){
    FillTile(hDC,m_Image,GetArea());
	CVSpace2::Draw(hDC);
	RECT rc = GetArea();
	rc.right = rc.left+1;
	FillRect(hDC,rc,RGB(192,192,192));
	rc.right = m_AreaRight;
	rc.left  = rc.right-1;
	FillRect(hDC,rc,RGB(192,192,192));
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInputWin::CInputWin()
{

	m_IOType   = IO_TASK;
	m_crText =  RGB(255,255,255);  
	m_SkipEnter = TRUE;
	m_BkgBrush = NULL;

	m_SendTimeStamp = 1;

	m_State |= SPACE_SHOWWINBORDER;

	m_Edit.m_Parent = this;

	CInputWin::Toolbar* bar = new Toolbar;
	m_ChildList.push_back(bar);
	m_Send = NULL;
}

CInputWin::~CInputWin(){
	if (m_BkgBrush)
	{
		DeleteObject(m_BkgBrush);
	}
}
	

void  CInputWin::OnInputText(WPARAM wParam,LPARAM lParam){
    ePipeline* Text = (ePipeline*)lParam;
	Text->SetID(GUI_IO_INPUTING);
	
	tstring& s = *(tstring*)Text->GetData(1);

	bool bEnter = s.size() == 1 && s[0] == VK_RETURN;
	
	int64 EventID = m_SendTimeStamp;
	m_SendTimeStamp  = AbstractSpace::CreateTimeStamp();
    Text->PushInt(m_SendTimeStamp);  //预先把下一个信息的时间ID发送，以便接受方检验信息顺序
	
	GetGUI()->SendMsgToBrainFocuse(*Text,EventID);

	if(!m_SkipEnter && bEnter){		
		OnInputEnd();
		return ;
	}
}
	

void CInputWin::OnInputEnd(){
	m_IOType = IO_WAIT;

	EnableInput(FALSE); //锁定输出窗口
	
	int Pos = GetWindowTextLength(m_Edit.GetHandle());

	ePipeline Text(GUI_IO_INPUTED);
	Text.PushInt(Pos);
	Text.PushString(m_Edit.GetEditText(false));
	
	GetGUI()->SendMsgToBrainFocuse(Text);

}

void CInputWin::EnableInput(BOOL Enable){
	::SendMessage(m_Edit.GetHandle(), EM_SETREADONLY,(WPARAM)!Enable,0); //锁定输出窗口
	::EnableWindow(m_Edit.GetHandle(),Enable);
	::SetFocus(m_Edit.GetHandle());
}

void CInputWin::EnableSendBnt(BOOL Enable){
	Toolbar* toolbar =  (Toolbar*)m_ChildList[0];
    CVSpace2* Space = toolbar->FindSpace(BNT_SEND);
	if(Space->m_State & SPACE_DISABLE){
		if(Enable)Space->m_State &=~SPACE_DISABLE;
	}else {
		if(!Enable)Space->m_State &= SPACE_DISABLE;
	}
	Invalidate();
}


void CInputWin::ClearEdit(const TCHAR* Tip){
    tstring text;
	if (Tip)
	{
		text = Tip;
	}
	m_Edit.SetEditText(text);
}

void CInputWin::ContinueEdit(const TCHAR* Text){
	//EnableInput(TRUE);
	if (Text)
	{
		tstring s = Text;
		m_Edit.SetEditText(s);
	}
}

void CInputWin::SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName )
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


void CInputWin::Layout(bool Redraw){
	RECT rcViewport; 
	GetClientRect(m_hWnd,&rcViewport);
	::InflateRect(&rcViewport,-1,-1);

	Toolbar* bar = (Toolbar* )m_ChildList[0];
	RECT rc = rcViewport;
	rc.left = rc.right-45;
	bar->SetArea(rc);
	bar->Layout();
	rcViewport.right -=45;

	Invalidate();

	::MoveWindow(m_Edit.GetHandle(),rcViewport.left,rcViewport.top,RectWidth(rcViewport),RectHeight(rcViewport),TRUE);
}

LRESULT CInputWin::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
   switch(message)
   {
   case WM_ERASEBKGND:
	   return TRUE;
   case WM_PAINT:
	   return OnPaint(wParam,lParam);
   case WM_SIZE:
	   return OnSize(wParam,lParam);
   case WM_CTLCOLOREDIT:
	   return OnCTLColor(wParam,lParam);
   case WM_LBUTTONDOWN:
	   return OnLButtonDown(wParam,lParam);
   case WM_LBUTTONUP:
		return OnLButtonUp(wParam,lParam);
   case WM_CREATE:   
	   return OnCreate(wParam,lParam);
   default:
	   return CWinSpace2::Reaction(message,wParam,lParam);
   }
}

LRESULT CInputWin::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT rcEdit;
	::GetClientRect(m_hWnd,&rcEdit);
	// Create the edit control
	DWORD style = WS_VISIBLE | WS_CHILD |ES_LEFT |
		ES_MULTILINE|ES_WANTRETURN|ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_HSCROLL|WS_VSCROLL;//|WS_BORDER;//|WS_EX_STATICEDGE;
	
	HWND hEdit =  CreateWindowEx(
		0,//WS_EX_STATICEDGE,      // extended window style
		RICHEDIT_CLASS,  // pointer to registered class name
		NULL, // pointer to window name
		style,        // window style
		rcEdit.left+1,                // horizontal position of window
		rcEdit.top,                // vertical position of window
		RectWidth(rcEdit),           // window width
		RectHeight(rcEdit),          // window height
		m_hWnd,      // handle to parent or owner window
		(HMENU)100,          // handle to menu, or child-window identifier
		GetHinstance(),  // handle to application instance
		NULL       // pointer to window-creation data
		);
	assert(hEdit != NULL);
	if(hEdit==NULL)return -1;
	
//	HFONT Font = (HFONT)::SendMessage(m_hWnd,WM_GETFONT, 0, 0);
//	HFONT Font = (HFONT)GetStockObject(DEVICE_DEFAULT_FONT); 
 	
	m_Edit.SetEditCtrl(hEdit);
	SendMessage( hEdit, EM_SETMARGINS, EC_LEFTMARGIN, m_Edit.m_LeftMargin );

	SetTextFont(12,FALSE,FALSE,_T("Fixedsys"));
    ::SendMessage(hEdit,WM_SETFONT, (WPARAM)m_Font, FALSE);
    
   	m_BkgBrush = ::CreateSolidBrush(RGB(0,0,0));
 
	return 0;
}
LRESULT CInputWin::OnSize(WPARAM wParam, LPARAM lParam){
	if(m_Edit.GetHandle()){
		Layout();
	}
	return 0;
}

LRESULT CInputWin::OnCTLColor(WPARAM wParam,LPARAM lParam){
	HDC hdc = (HDC) wParam;
	::SetTextColor(hdc,m_crText);
 	return (LRESULT)m_BkgBrush;
}

LRESULT CInputWin::OnLButtonDown(WPARAM wParam,LPARAM lParam){
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
    CVSpace2 * Space = Hit(point);	
	
	if(Space){
		int64 ID = Space->m_Alias;
		if(ID == BNT_SEND){
			if(Space->m_State == SPACE_DISABLE)return 0;
            Space->m_State |= SPACE_SELECTED;
			::SetCapture(m_hWnd);
			m_Send = Space;
		}
	}
	Invalidate();
	
	return 0;		
}
LRESULT CInputWin::OnLButtonUp(WPARAM wParam, LPARAM lParam) 
{
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
    CVSpace2 * Space = Hit(point);	
	
    if(Space && Space->m_Alias== BNT_SEND){
		::ReleaseCapture();
		OnInputEnd();
		Space->m_State |= SPACE_DISABLE;
        Space->m_State &=~ SPACE_SELECTED;   
	}else if(m_Send){			
		//鼠标捕获后可能在bnt之外放开	
		::ReleaseCapture();
		m_Send->m_State &=~ SPACE_SELECTED;   
		m_Send = NULL;
	}
	
	Invalidate();
	
    return 0;	
}

LRESULT CInputWin::OnPaint(WPARAM wParam, LPARAM lParam) 
{
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
		RECT rc = rcViewport;
		rc.right -=45;
		//Draw3DEdge(hdc,rcViewport);
		DrawEdge(DCMem,rcViewport,m_crWinBorder);// Border
	}	
	::BitBlt(hdc, 0, 0, RectWidth(rcViewport), RectHeight(rcViewport), DCMem, 0, 0, SRCCOPY );
	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);

	return 0;
}