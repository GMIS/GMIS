/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _WSTREEVIEW_H__
#define _WSTREEVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "WSScrollView.h"
#include <deque>
using namespace std;

#define	SPACE_OPEN	  	  0x00000100
#define	SPACE_ONLYREAD	  0x00000200
#define SPACE_NOTOOLABR   0x00000400
namespace VISUALSPACE{

/////////////////////////////////////////////////////////////////////////////
// CWSTreeView window

class  CWSTreeView : public CWSScrollView
{

public:
	class  CTreeItem : public CVSpace2{
	public:
		tstring m_Text;
	public:
		CTreeItem(){};
		CTreeItem(int64 ID,const TCHAR* text);
		virtual ~CTreeItem(){};
		
		virtual void Draw(HDC hDC,ePipeline* Pipe);
		
	};
	class CToolbar: public CVSpace2{
	public:
		CVSpace2*   m_Owner;
		int32       m_Height;     //default= 25；
		COLORREF    m_crToolBgk;  //default= RGB(160,160,160);
	public:
		CToolbar();
		virtual ~CToolbar();
		
		virtual  void  Layout(bool Redraw = true); //根据Owner->m_Area
		virtual void Draw(HDC hDC, ePipeline* Pipe=NULL);
		
	};
protected:	
	// m_HeaderWidth>0 将在Space左边偏移Draw TreeBox    
	uint32		       m_HeaderWidth; 
    HFONT              m_Font;
	COLORREF           m_crText;
	COLORREF		   m_crConnectingLine;
    HPEN               m_LinePen;

	bool               m_bShowLine;
	int32              m_Padding;
    CVSpace2*          m_SpaceSelected;

	deque<CVSpace2*>   m_HotSpaceList; //只绘制此表的Item,由Layout负责填充

	CToolbar		   m_Toolbar;
	CVSpace2*		   m_ToobarItemSelected;

public:
	virtual RECT      GetHeaderBox(CVSpace2* Space); //根据Space->Area计算TreeBox坐标	
	virtual void      DrawItemHeader(HDC hDC, CVSpace2* Space); //显示tree open状态及连接线 

	void SetTextFont( LONG nHeight, BOOL bBold, BOOL bItalic, const TCHAR* FontName );
    void SetLineColor(COLORREF color,int PenType=PS_SOLID);

    virtual void      Draw(HDC hDC,ePipeline* Pipe=NULL);
	virtual void      Layout(bool Redraw =true);
 	virtual SIZE      LayoutRecursive(CVSpace2* Space,int x, int y, const RECT& rcViewport);		
    virtual CVSpace2* Hit(POINT& point); //TreeViewport只处理HotList里Item

	virtual	void	  ToggleSpace(CVSpace2* Space);
    void              EnsureVisible(CVSpace2* Space, bool FirstVisible);	  	

	LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	SIZE    CalcuTextSize(const TCHAR* Text);

	virtual LRESULT ToolbarReaction(ButtonItem* Bnt);

public:
	CWSTreeView();
	virtual ~CWSTreeView();

	CTreeItem*  Push(CTreeItem* Item,int32 Pos=-1){ 
		PushChild(Item,Pos);
		SIZE s = CalcuTextSize(Item->m_Text.c_str());    	
	    Item->SetArea(0,0,s.cx+8,s.cy+8);
		return Item;
	};
protected:
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
};

};// namespace VISUALSPACE
/////////////////////////////////////////////////////////////////////////////

#endif // !defined(_WSTREEVIEW_H__)
