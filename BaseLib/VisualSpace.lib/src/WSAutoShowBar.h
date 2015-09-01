/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _WSAUTOSHOWBAR_H__
#define _WSAUTOSHOWBAR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// AutoShowViewport.h : header file
//
#include "VisibleSpace.h"
#include "WSToolbar.h"
#include "WSPopView.h"

namespace VISUALSPACE{
	
	/////////////////////////////////////////////////////////////////////////////
	// CWSAutoShowBar window
	enum DOCK_SIDE{ DOCK_NULL,DOCK_LEFT,DOCK_RIGHT,DOCK_TOP,DOCK_BOTTOM};


class  CWSAutoShowBar : public CWSToolbar
{
public:
    DOCK_SIDE      m_DockSide;
	int32          m_MaxItemWidth; //default=40  对于垂直bar为高度
// Construction
public:
	CWSAutoShowBar();
	virtual ~CWSAutoShowBar();

// Attributes
public:
    virtual void FocuseChanged(CVSpace2* OldSpace,POINT& point); //NewSpace = m_FocuseSpace; 
	void         SetDockSide(DOCK_SIDE side){ m_DockSide = side;};
	DOCK_SIDE    GetDockSide(){return m_DockSide;};

	void AddItem(ButtonItem* Item,CWSPopView* PopWin,bool Redraw=false);

	ButtonItem* GetItem(int64 CmdID);
	int32        GetItemCount(){ return m_ChildList.size();}; 
		
	virtual LRESULT ParentRecttion(SpaceRectionMsg* SRM);

	virtual void    Layout(bool Redraw = true);
};

} //namespace VISUALSPACE
/////////////////////////////////////////////////////////////////////////////

#endif // _WSAUTOSHOWBAR_H__
