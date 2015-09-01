/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _EDITWIN_H__
#define _EDITWIN_H__

#include "SubclassWnd.h"
#include <richedit.h>
#include "AbstractSpace.h"


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

	void SetEditCtrl(HWND hEdit);
	
	HWND GetEditCtrl() const{
		return GetHandle();
	}

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


	void SaveAsFile();
	 
	static DWORD CALLBACK SaveFileCallback(DWORD dwCookie, 
		   LPBYTE pbBuff, LONG cb, LONG *pcb);
	static DWORD CALLBACK StreamOutCallback(DWORD dwCookie, 
	    LPBYTE pbBuff, LONG cb, LONG *pcb);
	static DWORD CALLBACK StreamInCallback(DWORD dwCookie, 
		LPBYTE pbBuff, LONG cb, LONG *pcb);

};

#endif // !defined(AFX_EDITWIN_H__E34881BE_B1F7_4848_A03E_C7FA38531495__INCLUDED_)
