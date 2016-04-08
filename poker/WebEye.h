// WebEye.h: interface for the CWebEye class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WEBEYE_H__533A2DBE_A6D6_433A_B54D_51E8DB3EC4A7__INCLUDED_)
#define AFX_WEBEYE_H__533A2DBE_A6D6_433A_B54D_51E8DB3EC4A7__INCLUDED_

#include "WinSpace2.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace VISUALSPACE;

class CWebEye : public CWinSpace2  
{
public:
	POINTS  m_P1;  //begin drag point
	POINTS  m_P2;  //end drag point

	HBRUSH m_HalfBrush; 
	BOOL   m_bDrag;
public:
	CWebEye();
	virtual ~CWebEye();

protected:
	virtual void    Draw(HDC hDC,ePipeline* Pipe = NULL);
	
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(AFX_WEBEYE_H__533A2DBE_A6D6_433A_B54D_51E8DB3EC4A7__INCLUDED_)
