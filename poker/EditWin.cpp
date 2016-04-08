// EditWin.cpp: implementation of the CEditWin class.
//
//////////////////////////////////////////////////////////////////////
#include "WinSpace2.h"

#include "EditWin.h"

using namespace VISUALSPACE;

_AFX_RICHEDITEX_STATE::_AFX_RICHEDITEX_STATE()
{
    m_hInstRichEdit20 = NULL ;
}

_AFX_RICHEDITEX_STATE::~_AFX_RICHEDITEX_STATE()
{
    if (m_hInstRichEdit20 != NULL)
        ::FreeLibrary(m_hInstRichEdit20) ;
}

_AFX_RICHEDITEX_STATE _afxRichEditStateEx;

BOOL PASCAL AfxInitRichEditEx()
{
    _AFX_RICHEDITEX_STATE *l_pState = &_afxRichEditStateEx;
    
    if (l_pState->m_hInstRichEdit20 == NULL)
        l_pState->m_hInstRichEdit20 = ::LoadLibraryA("RICHED20.DLL");
    
    return l_pState->m_hInstRichEdit20 != NULL ;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEditWin::CEditWin()
{

}

CEditWin::~CEditWin()
{

}

void CEditWin::SetEditCtrl(HWND hEdit){
	if( hEdit != GetHandle() && GetHandle() != NULL )
		UnsubclassWindow();
	SubclassWindow( hEdit,FALSE ); 
}
 
LRESULT CEditWin::SendParentMessage(UINT Msg,int64 wParam, int64 lParam){
	HWND parent = GetParent(GetHandle());
	if(parent==NULL)return 0;
	SpaceRectionMsg SRM;
	
	SRM.Msg = Msg;
	SRM.wParam = wParam;
	SRM.lParam = lParam;
	SRM.ChildAffected = NULL;
	SRM.WinSpace = NULL;
	return ::SendMessage(parent,WM_PARENTRECTION,(WPARAM)&SRM,0);
}


void  CEditWin::SetEditText(tstring& Text){
	SendMessage(EM_HIDESELECTION, TRUE,TRUE);

	CHARRANGE cr;
	cr.cpMin =  0;
	cr.cpMax = -1;
	SetSel(cr);

    ReplaceSel(Text);
}
tstring CEditWin::GetEditText(bool Clear){
	SendMessage(EM_HIDESELECTION, TRUE,TRUE);

	CHARRANGE cr;
	cr.cpMin =  0;
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
	}else{
		cr.cpMax = -1;
		cr.cpMin = -1;
		SetSel(cr);
	}
	SendMessage(EM_HIDESELECTION, FALSE,FALSE);   
	return HisStr;
}

void CEditWin::SetCaretPos(int line,int pos)
{
  	int CharPos;

	if(line ==-1 && pos ==-1)
	{
		CharPos = -1;
	}
	else{		
		int LinePos = (int)SendMessage(EM_LINEINDEX, line, 0);
		if(pos == -1)
		{
			CharPos = LinePos + (int)SendMessage(EM_LINELENGTH, line, 0);
		}
		else{
			CharPos = LinePos + pos;
		}
	}
    CHARRANGE cr;
	cr.cpMax = CharPos;
	cr.cpMin = CharPos;
	SetSel(cr);
	tstring s;
	ReplaceSel(s);
	
};


void CEditWin::GetCaretPos(int& nLine, int& nPos)
{
	CHARRANGE cr;
	GetSel(cr);
	nLine = (long)SendMessage(EM_EXLINEFROMCHAR, 0, cr.cpMax);
	nPos = cr.cpMax  - (int)SendMessage(EM_LINEINDEX, nLine, 0);
}

	
DWORD CALLBACK CEditWin::StreamInCallback(DWORD dwCookie, 
												LPBYTE pbBuff, LONG cb, LONG *pcb)
{
   
    tstring&  Text = *(tstring*)(dwCookie);
	int n = cb/sizeof(TCHAR);
	n = min(n,Text.size());
	_tcsncpy((TCHAR*) pbBuff, Text.c_str() , n );
    Text.erase(0,n); 
	*pcb = n*sizeof(TCHAR);

	return 0;
}
DWORD CALLBACK CEditWin::StreamOutCallback(DWORD dwCookie, 
												 LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	int n = cb/sizeof(TCHAR);
	TCHAR* buf = new TCHAR[n + 1 ];

	_tcsncpy(buf, (TCHAR*) pbBuff,n);

	buf[n]=_T('\0');

	tstring &  Text = *(tstring *)(dwCookie);
	Text += buf;
	*pcb = n*sizeof(TCHAR);
	delete[] buf;
	return 0;
}
