/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _WSSCROLLVIEW_H__
#define _WSSCROLLVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScrollViewport.h : header file
//
#include "VisibleSpace.h"
#include "GUIElementMisc.h"


#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL       0x020A
#endif

#define  MINTHUMB 8
#define  TIMER_SCROLL_INCR    998
#define  TIMER_SCROLL_DECR    999
#define  SCROLL_MOVE_UNIT     16    //每次按下Arrow移动16

namespace VISUALSPACE{
	
/*
设计思想：
	如果采用系统自带scrollbar则使用自己实现的。

	我们用WinSpace.m_Area表达了Viewport的大小，MainSpace.m_Size里保存着整个文档的大小。
	当m_Size超过m_Area时，会自动设置相应的ScrollBar。

	每一个ScrollBar也是一个Space,它包含了三个Child Space：pre Arrow, Last Arrow和Thumb。在ScorllBar
	的Area里保存着它的显示位置。我们规定它的Size.cx=pageSize， Size.cy = ScorllPos。ScrollBar根
	据m_MainSpace.Size(min=0,max=size.cx或size.cy)调整自己的显示位置。 
  
	使用：
	在Layout()中设置WinSpace的Area和size,然后调用SetScrollbar()即可
*/
	
	enum  SCROLL_TYPE{ H_SCROLL, V_SCROLL};
	
	class  CWSScrollView : public CWinSpace2
	{
	protected:
		bool          m_UseSystemScrollbar;   //default = false;
		CVSpace2       m_VScrollbar;
		CVSpace2       m_HScrollbar;
	protected:
		int32         m_ScrollMoveUnit;     //default=SCROLL_MOVE_UNIT
		int32         m_ScrollWidth;        // default = 16
		SCROLL_TYPE   m_ScrollBarDraged;    // 当前正在DRAG哪个ScrollBar 
		int32         m_ThumbDragPos;       // -1 表示未使用ScrollBar 
		CVSpace2*      m_ScrollSelected;
		CVSpace2*      m_ScrollFocused; 
		
		void LayoutVScrollBar();
		void LayoutHScrollBar();     
		
		int32 GetVScrollPos();
		int32 GetHScrollPos();
		void  SetVScrollPos(int32 Pos,bool Redraw);
		void  SetHScrollPos(int32 Pos,bool Redraw);
		
		//这会在OnPaint()里自动完成，用户如果不实现自己的OnPaint则不需要自己调用
		virtual void  DrawScrollbar(HDC hDC);
		// Construction
	public:
		CWSScrollView();
		virtual ~CWSScrollView();

		virtual void LayoutScrollBar(); 
		virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
		virtual CVSpace2*  Hit(POINT& point);	
	protected:
		LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
		LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
		LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
		LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
		LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
		LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
		LRESULT OnHScroll(WPARAM wParam, LPARAM lParam);
		LRESULT OnVScroll(WPARAM wParam, LPARAM lParam);

	};
	
} //namespace VISUALSPACE
/////////////////////////////////////////////////////////////////////////////
#endif // _WSSCROLLVIEW_H__
