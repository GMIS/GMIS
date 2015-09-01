/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _INPUTWIN_H__
#define _INPUTWIN_H__

#include "PhysicSpace.h"
#include "VisibleSpace.h"
#include "WinSpace2.h"
#include <string>
#include "EditWin.h"

#define  CM_SHOWTHINKRESULT   15
#define  CM_SHOWANALYSERESULT 16 

#define BNT_SEND   201

using namespace VISUALSPACE;

class CInputWin;

class CInputEdit : public CEditWin 
{
public:
	CInputWin*  m_Parent;

	int32      m_LeftMargin;
	COLORREF   m_crLineNumber;
	COLORREF   m_crLineBk;
	bool       m_bForbidSend;

	COLORREF   m_crText;

	COLORREF   m_crTip;
	tstring    m_InputTip;


	int32      m_EditBeginPos;  //锁定的字符位置，禁止编辑反馈提示信息

	//在OnKeyDown()中调用CanBackspace()后当前字符位置改变则在OnChar()中调用CanBackspace()
	//将得到不一样的结果,所以在这里用一个变量来保存第一次CanBackspace的结果在OnChar中直接用
	BOOL       m_CanBackspace;  


public:  
	CInputEdit();
	virtual ~CInputEdit();

	void   SetInputTip(tstring tip);
	
	void   SetEditText(tstring& Text);
	tstring GetEditText(bool Clear);
	
public:
		
	BOOL IsEditingArea(int CharPos=-1);// -1表示当前光标位置

	void DeleteSelText(CHARRANGE& cr);

	void DrawLineNumber( HDC hdc );
protected:
	void OnDestroy();
	void OnMenuCopy();
	void OnMenuCut();
	void OnMenuPaste();
	void OnMenuSend();
    void OnMenuThink();
	void OnMenuAnalyse();
	void OnMenuClear();

	virtual BOOL ProcessWindowMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );
};



enum IOTYPE{ IO_WAIT,IO_TASK,IO_THINK,IO_CHAT};

class CInputWin : public CWinSpace2
{
	friend class CInputEdit;

public:
	tstring      m_Name;	

	IOTYPE       m_IOType;

	COLORREF     m_crText;
	HBRUSH       m_BkgBrush;

	BOOL         m_SkipEnter;    //是否Enter即执行

	CInputEdit   m_Edit;

    HFONT        m_Font;
	
	int64        m_SendTimeStamp;

  
public:
	class Bnt : public CVSpace2{
    public:
		tstring  m_Name;
	public:
		Bnt(int64 ID,TCHAR* Name);
		~Bnt();
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
	};
	
	class Toolbar : public CVSpace2{
	public:
		HANDLE          m_Image;
	public:
		Toolbar(HANDLE Image=NULL);
		~Toolbar();
		
		virtual void Layout(bool Redraw=false);
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);    
	};
	
	CVSpace2*  m_Send;
public:
	CInputWin();
	virtual ~CInputWin();

	void EnableSendBnt(BOOL Enable);

	void EnableInput(BOOL Enable);

	void SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName );

	void ClearEdit(const TCHAR* Tip=NULL);  //如果tip!=NULL设置tip为输入窗口当前文本
	void ContinueEdit(const TCHAR* Text=NULL); //如果text有效则设置为输入窗口当前文本

protected:
    void PushSendQueue(ePipeline& Text);

    void OnInputEnd(); 
	void OnInputText(WPARAM wParam,LPARAM lParam);
	LRESULT OnSetProgress(WPARAM wParam,LPARAM lParam);
	virtual void Layout(bool Redraw=false);

    virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
protected:
	LRESULT OnCreate(WPARAM wParam,LPARAM lParam);
	LRESULT OnSize(WPARAM wParam,LPARAM lParam);
	LRESULT OnPaint( WPARAM wParam, LPARAM lParam);
	LRESULT OnCTLColor(WPARAM wParam,LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);

};

#endif // !defined(_INPUTWIN_H__)
