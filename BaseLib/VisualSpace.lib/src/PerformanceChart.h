/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _PERFORMANCECHART_H__
#define _PERFORMANCECHART_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WinSpace2.h"

using namespace VISUALSPACE;

#define ADD_LINEDATA         1000


//横轴为次（对应每一个像素点），数轴为每次的值

class CPerformanceChart : public CWinSpace2  
{
public:
	class  Line : public CVSpace2{
	public:		
		HPEN         m_LinePen;
		ePipeline    m_LineValue;
	public:
		Line(int64 ID,CVSpace2* Parent);
		virtual ~Line();
		
		void SetPen(COLORREF cr);

		virtual void Draw(HDC hDC,ePipeline* Pipe);
		
	};	

	class  Chart : public CVSpace2{
	public:
		int32    m_MinValue; 
		int32    m_MaxValue; 
		
        tstring  m_Title;

		HPEN     m_LinePen;  
		COLORREF m_crBk;
		COLORREF m_crText;
		int32    m_Pading;
		int32    m_xgap;  //横轴间距，default= 4;
	public:
		Chart();
		virtual ~Chart();
		
		Line* GetLine(int64 ID);

		void SetPen(COLORREF crPen);

		void AddLine(int64 ID,COLORREF crPen);
	
		void AppendLineData(int64 ID,int32 Value);
		void AppendLineData(int64 ID,ePipeline& ValueList);

		virtual void Draw(HDC hDC,ePipeline* Pipe);		
	};	

protected:
	Chart* GetChart();
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);

public:
	CPerformanceChart();
	virtual ~CPerformanceChart();

	void SetTitle(tstring s);
	void InitRang(int low,int32 high);
	void SetXGap(int32 gap);

	void AddLine(int64 ID, COLORREF cr);

	void AppendLineData(int64 ID,int32 Value);
	void AppendLineData(int64 ID,ePipeline& ValueList);

};

#endif // _PERFORMANCECHART_H__