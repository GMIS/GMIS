/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _NAVIGATIONBAR_H__
#define _NAVIGATIONBAR_H__


#include "VisibleSpace.h"
#include "WSTitleView.h"

using namespace VISUALSPACE;	

BOOL RegisterToolClass(HINSTANCE hInstance);

#define  ID_TURN       100
#define  ID_GOAHREAD   101
#define  ID_GOBACK     102

#define  CM_WORLDTRUN  200
#define  CM_WORLDMOVE  201

#define  ID_TIMER_MOVE  1128

class TurnItem: public CVSpace2{

public:
    TurnItem(int32 ID);
	virtual ~TurnItem();
	virtual void Draw(HDC hDC,ePipeline* Pipe /* = NULL */);
};

class ForwardItem: public CVSpace2{
public:
    ForwardItem(int32 ID);
	virtual ~ForwardItem();
	virtual void Draw(HDC hDC,ePipeline* Pipe /* = NULL */);
};

class BackItem: public CVSpace2{
public:
    BackItem(int32 ID);
	virtual ~BackItem();
	virtual void Draw(HDC hDC,ePipeline* Pipe /* = NULL */);
};


class CNavigationBar : public CWinSpace2  
{
public:		
    BOOL Create(DWORD dwStyle, const RECT& rect, HWND pParentWnd);

public:
	int32  m_x;     //在父窗口的位置坐标（中心点）
	int32  m_y;

	bool        m_IsDock;
	CVSpace2*   m_SpaceSelected;
	POINT       m_OldMousePos;
		
	int32       m_MoveAcceleration; //default = 2;
	int32       m_MoveSpeed;        //init = 0;
	int32       m_MoveTime;
public:
	CNavigationBar();
	virtual ~CNavigationBar();
  
	void MoveTo(int32 x, int32 y);

protected:
	virtual void    Layout(bool Redraw = true);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(_NAVIGATIONBAR_H__)
