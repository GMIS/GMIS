/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _PERFORMANCEVIEW_H__
#define _PERFORMANCEVIEW_H__


#include "PerformanceChart.h"
#include "GUIMsgDefine.h"


class CPerformanceView : public CWinSpace2  
{
public:
	CPerformanceChart    m_ThreadNumView;
	CPerformanceChart    m_NerveMsgNumView;
public:
	CPerformanceView();
	virtual ~CPerformanceView();
protected:
	//virtual void    FocuseChanged(CVSpace2* OldSpace,POINT& point){};
	virtual void    Layout(bool Redraw = true);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(_PERFORMANCEVIEW_H__)
