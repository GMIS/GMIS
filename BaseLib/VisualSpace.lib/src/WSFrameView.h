/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _WSFRAMEVIEW_H__
#define _WSFRAMEVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FrameSpace.h : header file
//

#include "WSTitleView.h"

namespace VISUALSPACE{
	
    //NOTE: 如果作为MainFrame 必须自己在适当时机调用::PostQuitMessage(0)以结束程序;
	/////////////////////////////////////////////////////////////////////////////
	// CWSFrameView window
	
	class  CWSFrameView : public CWSTitleView
	{
		// Construction
	public:
		CWSFrameView();
		virtual ~CWSFrameView();
		void SetIcon(HICON Icon);
		void SetTitle(const TCHAR* Text);
		tstring  GetTitle();
	protected:
		virtual void    NcLayout();
		virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
		
	protected:
		LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
		LRESULT OnLButtonDblClk(WPARAM wParam, LPARAM lParam);
		LRESULT OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam);
	};
	
}; //namespace VISUALSPACE
/////////////////////////////////////////////////////////////////////////////
#endif // _WSFRAMEVIEW_H__
