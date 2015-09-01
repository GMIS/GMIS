/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _EXEOUTPUT_H__
#define _EXEOUTPUT_H__


#include "EditWin.h"
#include "WinSpace2.h"

using namespace VISUALSPACE;


#define  CM_OPENOUTPUTWIN   12
#define  CM_CLOSEOUTPUTWIN  13


#define  CM_UPDATELINE  100
#define  CM_UPDATEFONT  200
#define  CM_SELECTLINE  210


class COutputEdit : public CEditWin  
{
public:
	COutputEdit(){};
	virtual ~COutputEdit(){};
	
	// Operations
protected:
    LRESULT OnRButtonDown(WPARAM wParam, LPARAM lParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	void    OnDestroy();
	
	virtual BOOL ProcessWindowMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );
	
};

class CLineNumber : public CWinSpace2  
{
public:
	class NumberItem: public CVSpace2{
    public:
		tstring m_Text;
    public:
		NumberItem(int64 ID);
		virtual ~NumberItem();
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
	};
	
	public:
		CLineNumber();
		virtual ~CLineNumber();
	protected:
		virtual void Layout(bool Redraw /* = true */);
		virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
		LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
		
	public:
		// Attributes
		COLORREF			m_fgcol;
		COLORREF			m_bgcol;
		tstring				m_format;
};



class CLineNumEdit : public CWinSpace2  
{
protected:
	BOOL			 m_Created;
	COLORREF	     m_crBk;
	COLORREF         m_crText;
	HBRUSH           m_BkgBrush;
	HFONT            m_Font;

	CLineNumber      m_LineView; 
	HWND             m_hEdit;
	COutputEdit      m_EditView;

public:
	CLineNumEdit();
	virtual ~CLineNumEdit();

	void SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName );

	void AddText(const TCHAR* Text,bool NoChangeVisibleLine =true);
	void ClearText();
protected:
	void OnSelectLine(int line);
    void DrawLineNumber(HWND EditWin);
	int  CalcLineNumberWidth();
	
	virtual void Layout(bool Redraw = true);
    virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT ParentReaction(SpaceRectionMsg* SRM);

	LRESULT OnCreate(WPARAM wParam,LPARAM lParam);
	LRESULT OnSize(WPARAM wParam,LPARAM lParam);
	LRESULT OnCTLColorStatic(WPARAM wParam,LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam,LPARAM lParam);
	
};

#endif // !defined(_EXEOUTPUT_H__)
