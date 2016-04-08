// StatusBar.h: interface for the CStatusBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_StatusBar_H__F89FFC60_72A4_4FFB_BD6E_15E46087AA5A__INCLUDED_)
#define AFX_StatusBar_H__F89FFC60_72A4_4FFB_BD6E_15E46087AA5A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TheorySpace.h"
#include "GuiElementMisc.h"

#define   SPACE_USING  0x08000000

#define  STATUS_SETTEXT        1
#define  STATUS_SETPROGRESSPOS 2


#define   BNT_OCRVIEW        1000

#define   BNT_TASK_RUN       1001  
#define   BNT_TASK_STOP      1002
#define   BNT_TASK_PAUSE     1003
#define   BNT_CON_ROBOT      1004
#define   BNT_TASK_BISAI     1005
#define   BNT_TASK_DEBUG     1006
#define   BNT_TASK_SHUPENG   1007
#define   BNT_TASK_JIAOZHENG 2008
#define   BNT_TASK_OPTION    2009

using namespace VISUALSPACE;

class CStatusBar : public CWinSpace2  
{
	class ProgressItem:public CVSpace2
	{
	public:
		uint8     m_Alpha;  //半透明 default == 255 不透明
		COLORREF  m_crBorder;
		COLORREF  m_crThumb;
   	    tstring   m_Text;
		int32     m_Per; //百分比	[0，100]	
	public:
		ProgressItem();
		virtual ~ProgressItem(){};
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
		
	};

	//双态按钮
	class ControlBnt : public ButtonItem
	{	
	public:
		ControlBnt(int64 ID,const TCHAR* Name)
			:ButtonItem(ID,Name,NULL,false){
		};
		virtual ~ControlBnt(){};
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
	};

protected:
    HFONT             m_Font;
	COLORREF          m_crText; 
   	tstring           m_Text;

	ProgressItem      m_ProgressBar;


	ControlBnt        m_RunBnt;
	ControlBnt        m_PauseBnt;
	ControlBnt        m_StopBnt;

	ControlBnt        m_OptionBnt;

	ControlBnt        m_DebugBnt;
	ControlBnt        m_BisaiBnt;
    ControlBnt        m_ShuaBnt; 
	ControlBnt        m_Jiaozheng;

	CVSpace2*         m_SpaceSelected;
public:
	CStatusBar();
	virtual ~CStatusBar();

	void   SetTip(tstring Text);
	void   SetProgressPer(int32 Per);
    void   SetProgressAlpha(uint8 Alpha);
	void   SetProgressColor(COLORREF cr);

	void   SetBntState(int32 BntID,uint32 State);
	void   StopTask();
    void   CloseOptionView();

	virtual  void    Layout(bool Redraw = true);
	void SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName );
    void SetTextColor(COLORREF color){ m_crText = color;};

protected:
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
    virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);

protected:
	LRESULT OnPaint();
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);

};

#endif // !defined(AFX_StatusBar_H__F89FFC60_72A4_4FFB_BD6E_15E46087AA5A__INCLUDED_)
