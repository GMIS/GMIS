// AddressBar.h: interface for the CAddressBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADDRESSBAR_H__83B663F9_77F1_4689_BA50_1B2785FB3AC9__INCLUDED_)
#define AFX_ADDRESSBAR_H__83B663F9_77F1_4689_BA50_1B2785FB3AC9__INCLUDED_


#include "TheorySpace.h"
#include "GUIElementMisc.h"
#include "WinSpace2.h"
#include "EditWin.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace VISUALSPACE;

#define CM_CONNECT    121
#define CM_DISCONNECT 122

class CAddressBar : public CWinSpace2  
{
public:
	class CAddressEdit : public CEditWin 
	{
	protected:
		virtual BOOL ProcessWindowMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );
	};

	class CBnt : public ButtonItem
	{	
		
	public:
		CBnt(int64 ID,tstring text);
		virtual ~CBnt();
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
	};

	//工具栏分割线
	class SeparatorLine : public CVSpace2
	{
	public:
		BOOL      m_bVerDraw;     //default = TRUE;
		int       m_NumLine;      //default = 1
		COLORREF  m_crDark;       //default RGB(13,13,13)
		COLORREF  m_crLight;      //default
	public:
		SeparatorLine(int32 ID,int32 nline = 1);
		virtual ~SeparatorLine(){};
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
	};

public:
	
    BOOL              m_bConnected;
	//CAddressEdit      m_AddressEdit;
	HWND              m_AddressEdit;

	CBnt              m_ConnectBnt;

	//AddressBar请求的对话地址
	int64             m_ServerSourceID;
	int64             m_ServerDialogID;
public:
	CAddressBar();
	virtual ~CAddressBar();

	void  SetText(tstring text);
	void  SetConnectState(BOOL success);	
	virtual  void  Layout(bool Redraw = true);

protected:
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
protected:
	LRESULT OnPaint();
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam,LPARAM lParam);

};

#endif // !defined(AFX_ADDRESSBAR_H__83B663F9_77F1_4689_BA50_1B2785FB3AC9__INCLUDED_)
