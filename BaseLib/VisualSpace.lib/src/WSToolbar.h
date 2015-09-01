/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _WSTOOLBAR_H__
#define _WSTOOLBAR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VisibleSpace.h"
#include "WinSpace2.h"

#define  TOOL_DEFAULT_HEIGHT 24

namespace VISUALSPACE{
	
	
	class  CWSToolbar : public CWinSpace2
	{
	protected:
	bool   m_bVertical;   //default = false;
// Construction
public:
	CWSToolbar();
	virtual ~CWSToolbar();
public:
 
	void AlignVertical(){ m_bVertical = true;};
	
	virtual void    Layout(bool Redraw = true);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint();

};

} //namespace VISUALSPACE
/////////////////////////////////////////////////////////////////////////////
#endif // !defined(_WSTOOLBAR_H__)
