/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef  _WSLISTVIEW_H_
#define  _WSLISTVIEW_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WSTreeView.h"

namespace VISUALSPACE{
	
	
	/////////////////////////////////////////////////////////////////////////////
	// CListViewport window
	
	class  CWSListView : public CWSTreeView
{
public:
	class  CListItem : public CWSTreeView::CTreeItem{
		
	public:
		CListItem(){};
		CListItem(int32 ID,const TCHAR* text):
		CTreeItem(ID,text)
		{
		};
		virtual ~CListItem(){};
		
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
		
	};

// Construction
public:
	CWSListView();
	virtual ~CWSListView();

// Attributes
public:
	CListItem*     AddItem(const TCHAR* text,bool Redraw=true);
	int32         GetItemCount(){ return m_ChildList.size();};
	//画编号
	virtual void  DrawItemHeader(HDC hDC, CVSpace2* Space);  
};

} //namespace VISUALSPACE
/////////////////////////////////////////////////////////////////////////////

#endif // _WSLISTVIEW_H_
