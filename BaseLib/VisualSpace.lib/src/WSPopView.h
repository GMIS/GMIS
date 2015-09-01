/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _WSPOPVIEW_H_
#define _WSPOPVIEW_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PopViewport.h : header file
//

#include "WSTitleView.h"

namespace VISUALSPACE{
	/////////////////////////////////////////////////////////////////////////////
	
#define TIMER_POPSHOW  100
	
//注意：此窗口的父空间有义务在ParentRection()中处理此消息
#define  CM_DOCKVIEW   200
#define  CM_HIDEVIEW   201
	
	class CWSAutoShowBar;
	

	//决定在父窗口的哪个方向弹出
	enum POP_DIRECTION{ POP_NULL,POP_LEFT,POP_RIGHT,POP_UP,POP_DOWN};


	class  CWSPopView : public CWSTitleView
	{
	protected:
		int32              m_StepSize;     //default = 30;
		int32              m_PopSize;      //default = 200;
		int32              m_LeftPad;      //（相对于父空间窗口偏移 ）or TopPad,  default = 0;
		int32              m_RightPad;     //or DownPad, default = 0;

		POP_DIRECTION      m_PopDirection;
		bool               m_bDragBorder;  
		bool               m_IsPop;
		// Construction
	public:
		CWSPopView();
		virtual ~CWSPopView();

		// Operations
	public:
		void Pop();
		void Hide();

        virtual LRESULT SendParentMessage(UINT Msg,int64 wParam, int64 lParam,CVSpace2* Space);

		void SetPopDirection(POP_DIRECTION d){ m_PopDirection = d;};
        POP_DIRECTION GetPopDirection(){ return m_PopDirection;};

		BOOL IsPop(){ return m_IsPop ;};
		BOOL IsDragBorder(){return m_bDragBorder;}
		
		void AsPopView(CWinSpace2* Parent);
		void AsChildView(CWinSpace2* Parent,UINT32 Style=0);
		
		void   SetIcon(HICON Icon);
		void   SetTitle(const TCHAR* Text);
		tstring GetTitle();
		
		DockboxItem* GetDockBnt();
		
		virtual void  NcLayout();	
		virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);

	protected:
		LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
		LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	};
	
} //namespace VISUALSPACE
/////////////////////////////////////////////////////////////////////////////
#endif // !defined(_WSPOPVIEW_H_)
