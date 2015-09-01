/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _STATUSTIP_H__
#define _STATUSTIP_H__



#include "PhysicSpace.h"
#include "GuiElementMisc.h"

#define  STATUS_SETTEXT        1
#define  STATUS_SETPROGRESSPOS 2

using namespace VISUALSPACE;

class CStatusTip : public CWinSpace2  
{
	class ProgressItem:public CVSpace2
	{
	public:
		uint8     m_Alpha;  // default == 255 
		COLORREF  m_crBorder;
		COLORREF  m_crThumb;
   	    tstring   m_Text;
		int32     m_Per; //	[0，100]	
	public:
		ProgressItem();
		virtual ~ProgressItem(){};
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
		
	};
	
	class ControlBnt : public ButtonItem
	{	
		
	public:
		ControlBnt(int64 ID,const TCHAR* text)
			:ButtonItem(ID,text,NULL,false){
		};
		virtual ~ControlBnt(){};
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
	};

protected:
    HFONT             m_Font;
	COLORREF          m_crText; 
   	tstring           m_Text;
	ProgressItem      m_ProgressBar;

public:
	CStatusTip();
	virtual ~CStatusTip();

	void   SetTip(tstring Text);
	void   SetProgressPer(int32 Per);
    void   SetProgressAlpha(uint8 Alpha);
	void   SetProgressColor(COLORREF cr);


	virtual  void    Layout(bool Redraw = true);
	void SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* csFaceName );
    void SetTextColor(COLORREF color){ m_crText = color;};

protected:
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
    virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);

protected:
	LRESULT OnPaint();
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);

};

#endif // !defined(AFX_STATUSTIP_H__F89FFC60_72A4_4FFB_BD6E_15E46087AA5A__INCLUDED_)
