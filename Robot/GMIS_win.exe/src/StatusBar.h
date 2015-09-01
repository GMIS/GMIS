/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _STATUSBAR_H__
#define _STATUSBAR_H__


#include "PhysicSpace.h"
#include "GuiElementMisc.h"

#define   SPACE_USING  0x08000000

#define  STATUS_SETTEXT        1
#define  STATUS_SETPROGRESSPOS 2
#define  STATUS_LIGHTLAMP      3


#define   BNT_PERFORMANCE_VIEW      1001  
#define   BNT_OPTION_VIEW           1002
#define   BNT_SPACE_VIEW            1003
#define   BNT_LOG_VIEW              1004

using namespace VISUALSPACE;

enum LIGHTLAMP{ IN_LAMP, OUT_LAMP};

class CStatusBar : public CWinSpace2  
{
public:
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

	class NetLamp : public CVSpace2
	{
	public:
		BOOL      m_bLight;
		COLORREF  m_crLight;
		COLORREF  m_crDark;
    public:
		NetLamp(COLORREF crLight, COLORREF crDark);
		virtual ~NetLamp(){};
		virtual void Draw(HDC hDC,ePipeline* Pipe = NULL);
	};

protected:
    HFONT             m_Font;
	COLORREF          m_crText; 
   	tstring           m_Text;

	ProgressItem      m_ProgressBar;


	NetLamp           m_OutputLamp;
	NetLamp           m_InputLamp;

	ControlBnt        m_PerformanceViewBnt;
	ControlBnt        m_OptionViewBnt;
	ControlBnt        m_SpaceBnt;
	ControlBnt        m_LogViewBnt;

	CVSpace2*         m_SpaceSelected;
public:
	CStatusBar();
	virtual ~CStatusBar();

	void   LightLamp(LIGHTLAMP Lamp,BOOL bLight); 

	void   SetTip(tstring Text);
	void   SetProgressPer(int32 Per);
    void   SetProgressAlpha(uint8 Alpha);
	void   SetProgressColor(COLORREF cr);

	void   SetBntState(int32 BntID,uint32 State);

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

#endif // !defined(_STATUSBAR_H__)
