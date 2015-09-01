/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _TREEVIEW_H__
#define _TREEVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "WSScrollView.h"
#include "WSTreeView.h"
#include <deque>
using namespace std;

using namespace VISUALSPACE;

class CTreeView : public CWSScrollView  
{
protected:	
	// m_HeaderWidth>0 将在Space左边偏移Draw TreeBox    
	uint32		       m_HeaderWidth; 
	COLORREF		   m_crConnectingLine;
    HPEN               m_LinePen;

	bool               m_bShowLine;
	int32              m_Padding;
    CVSpace2*          m_SpaceSelected;

protected:

	virtual void      SizeReaction(CVSpace2* Parent,deque<CVSpace2*>::iterator& ChildIt);
	CVSpace2*         BinaryHit(POINT& p, deque<CVSpace2*>& List,int32 begin, int32 n);

	virtual void      DrawItemHeader(HDC hDC, RECT& rcClient,CVSpace2* Space); //显示tree open状态及连接线 
    virtual void      DrawItem(HDC hDC, RECT& rcClient,CVSpace2* Space);

	virtual RECT      GetHeaderBox(CVSpace2* Space); //根据Space->Area计算TreeBox坐标	

public:

    void              SetLineColor(COLORREF color,int PenType=PS_SOLID);
	SIZE              CalcuTextSize(const TCHAR* Text,HFONT Font);

    virtual void      Draw(HDC hDC,ePipeline* Pipe=NULL);
	virtual void      Layout(bool Redraw =true);


    virtual CVSpace2* Hit(POINT& point); //TreeViewport只处理HotList里Item

	virtual	void	  ToggleSpace(CVSpace2* Space);
    void              EnsureVisible(CVSpace2* Space, bool FirstVisible);	  	

	LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);

// Construction
public:
	CTreeView();
	virtual ~CTreeView();

    void Insert(CVSpace2* Parent,CVSpace2* Child,int32 InsertPos =-1);
	CVSpace2* Delete(CVSpace2* Child);
	
protected:
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDBCLK(WPARAM wParam, LPARAM lParam);//测试插入或删除 
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
};

#endif // _TREEVIEW_H__
