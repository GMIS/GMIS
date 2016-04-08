// EditWin.h: interface for the CEditWin class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITWIN_H__E34881BE_B1F7_4848_A03E_C7FA38531495__INCLUDED_)
#define AFX_EDITWIN_H__E34881BE_B1F7_4848_A03E_C7FA38531495__INCLUDED_

#include "SubclassWnd.h"
#include <richedit.h>
#include "AbstractSpace.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class _AFX_RICHEDITEX_STATE
{
public:
					_AFX_RICHEDITEX_STATE();
    virtual			~_AFX_RICHEDITEX_STATE();

    HINSTANCE		m_hInstRichEdit20 ;
};

BOOL PASCAL AfxInitRichEditEx();


class CEditWin : public CSubclassWnd  
{
public:
	CEditWin();
	virtual ~CEditWin();

	void    SetEditCtrl(HWND hEdit);

	LRESULT SendParentMessage(UINT Msg,int64 wParam, int64 lParam);

	void   SetEditText(tstring& Text);
	tstring GetEditText(bool Clear);

	BOOL SetReadOnly(BOOL bReadOnly = TRUE){
		return (BOOL)SendMessage(EM_SETREADONLY, bReadOnly, 0L);
	}
	void GetSel(CHARRANGE &cr){
		SendMessage(EM_EXGETSEL, 0, (LPARAM)&cr);
	}
	void SetSel(CHARRANGE &cr){
		SendMessage(EM_EXSETSEL, 0, (LPARAM)&cr);
	}
	void ReplaceSel(tstring& s){
		SendMessage(EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)s.c_str()); 	
	}
    long GetTextLength(){
		return (long)SendMessage(WM_GETTEXTLENGTH, NULL, NULL); 
	}

	void SetCaretPos(int line=-1,int pos=-1); //缺省为最后位置
	void GetCaretPos(int& nLine, int& nPos);

	static DWORD CALLBACK StreamOutCallback(DWORD dwCookie, 
	    LPBYTE pbBuff, LONG cb, LONG *pcb);
	static DWORD CALLBACK StreamInCallback(DWORD dwCookie, 
		LPBYTE pbBuff, LONG cb, LONG *pcb);

};

#endif // !defined(AFX_EDITWIN_H__E34881BE_B1F7_4848_A03E_C7FA38531495__INCLUDED_)
