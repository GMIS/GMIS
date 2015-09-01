/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _WSTITLEVIEW_H__
#define _WSTITLEVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VisibleSpace.h"
#include "GUIElementMisc.h"

// CWSTitleView取消了系统缺省的非客户区，即在处理WM_NCCALCSIZE消息时让rcWin = rcClient;
// 然后在窗口客户区画四个缺省的边，其中m_NcBorderList[0]较大为Title
// m_NcBorderList[1] = Left  m_NcBorderList[2]=right m_NcBorderList[3] = bottom
// 这个四个边内的面积分配给m_MainSpace,最好自己就别设置WS_CAPTION
//     ___________________ 
//    |___________________|  
//    ||                 ||         
//    ||                 || 
//    ||                 ||        
//    ||_________________||    
//    ---------------------  
//
//使用方法:   
//    
//    在重载的Layout()里先调用NcLayout()得到正确的m_MainSapce坐标。其他与
//    CViewport的使用一样。
//
//    用户可以在任何NcBorderItem的ChildList加入自己的CSpac2的类实例。
//    比如在Title加入close minbox或maxbox,然后重载NcLayout负责为加入的CSpace规
//    划坐标，坐标基于Client;
//     
//    NcDrawCtrl()只负责绘制四个NcBorderItem。NcBorderItem会Draw()里处理其可能有的Child。
//
//    同样NcHit()只依次调用NcBoderItem->HitTest()。NcBorderItem会处理其可能有的Child。
//
//    如果要改变缺省的NcBorder，用户可以先delete选定的一个再加入自己的NcBorderItem
//
//    建议任何给NcBorderItem及其child唯一的识别号，CWSTitleView已经设置四个边ID分别为
//    -1\-2\-3\-4 
/////////////////////////////////////////////////////////////////////////////
// CWSTitleView window

#define TIMER_NCMOUSELEAVE 887   //类似TIMER_MOUSELEAVE

namespace VISUALSPACE{

class  CWSTitleView : public CWinSpace2
{
protected:
    COLORREF          m_crClientBorder;       //default = RGB(0,0,0)     
	int32             m_TitleHeight;        //defualt = 24
	int32             m_NcBoderWidth;       //defualt = 4
    CVSpace2*         m_NcBorderList[4];                         
	
	CVSpace2*         m_NcSpaceFocused;

	RECT              m_rcClient;

	//为四个边分配坐标并把剩下面积的坐标给m_rcClient
	//注意：m_Area基于父空间的坐标，CWinSpace2给自己子空间分配的坐标应该基于自己的绘制区
  	virtual void       NcLayout();    
    virtual void       NcDraw( HDC hDC); 
    virtual CVSpace2*  NcHit(int32 x, int32 y); 
    void  DeleteAllBorder();

	CVSpace2* GetTitleBorder(){ return m_NcBorderList[0];};
	CVSpace2* GetLeftBorder(){  return m_NcBorderList[1];};
	CVSpace2* GetRightBorder(){ return m_NcBorderList[2];};
	CVSpace2* GetBottomBorder(){ return m_NcBorderList[3];};

	//设置新的返回旧的
	CVSpace2*  SetTitleBorder(CVSpace2* NewTitle);
	CVSpace2*  SetLeftBorder(CVSpace2* NewLeft);
	CVSpace2*  SetRightBorder(CVSpace2* NewRight);
	CVSpace2*  SetBottomBorder(CVSpace2* NewBottom);

// Construction
public:
	CWSTitleView();
	virtual ~CWSTitleView();

	virtual void Layout(bool Redraw = true){ NcLayout(); if(Redraw)Invalidate();};
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);

protected: //Reaction处理了以下消息
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnNcActivate(WPARAM wParam, LPARAM lParam);
	LRESULT OnNcCalcSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
};

} //namespace VISUALSPACE

/////////////////////////////////////////////////////////////////////////////
#endif // (!_WSTITLEVIEW_H__)
