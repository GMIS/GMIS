/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _LISTVIEW_H__
#define _LISTVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TreeView.h"

class CListView : public CTreeView  
{
protected:
	BOOL  m_bUnitWidth;  //条目宽度是否相同,default = FALSE;
public:
	CListView();
	virtual ~CListView();


	CVSpace2* HitTestChild(CVSpace2* Parent,POINT& point);

protected:
	virtual void SizeReaction(CVSpace2* Parent,deque<CVSpace2*>::iterator& ChildIt);
    virtual void DrawItemHeader(HDC hDC, RECT& rcClient,CVSpace2* Space); 
	virtual void DrawChild(CVSpace2* Parent,HDC hDC,ePipeline* Pipe);
    virtual CVSpace2* Hit(POINT& point); 

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);

};

#endif // _LISTVIEW_H__38851603_EE95_4194_A20E_54DAAC46073D__INCLUDED_)
