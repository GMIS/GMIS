// SplitterView.h: interface for the CSplitterView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPLITTERVIEW_H__4668F612_2A51_4CCA_8874_DA4F34245D56__INCLUDED_)
#define AFX_SPLITTERVIEW_H__4668F612_2A51_4CCA_8874_DA4F34245D56__INCLUDED_

#include "WinSpace2.h"
#include "Splitter.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace	VISUALSPACE;

class CSplitterView : public CWinSpace2  
{
	CWinSpace2*         m_Parent;
	SplitterStruct      m_Splitter;
	CVSpace2*           m_SpaceSelected;
public:
	CSplitterView(CWinSpace2* Parent,BOOL IsVertical);
	virtual ~CSplitterView();

	void AddView(CWinSpace2* View,DWORD Style=WS_CHILD|WS_VISIBLE);
	void DelView(int32 ViewID);

protected:
	virtual void  Layout(bool Redraw = true);

	virtual LRESULT ParentReaction(SpaceRectionMsg* SRM);
	virtual LRESULT ChildReaction(SpaceRectionMsg* SRM);

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(AFX_SPLITTERVIEW_H__4668F612_2A51_4CCA_8874_DA4F34245D56__INCLUDED_)
