// OptionView.h: interface for the COptionView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONVIEW_H__AF42315B_FA42_43A4_B179_A877B1F69B50__INCLUDED_)
#define AFX_OPTIONVIEW_H__AF42315B_FA42_43A4_B179_A877B1F69B50__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WSTitleView.h"
#include "WSFormView.h"

#define PLAY_TYPE        1
#define PLAY_FACTOR      2
#define PLAY_TIME        3
#define PLAY_SELECTROOM  4
#define PLAY_DEBUG       5
#define PLAY_LINGFEN     6

#define NEW_OPTION_RESULT  40001

using namespace VISUALSPACE;

class COptionView : public CWSTitleView  
{
    class CMyOptionView: public CWSFormView
	{
	public:
		CMyOptionView();
		~CMyOptionView();
	
	protected:
		virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);

	};
public:
	CMyOptionView    m_OptionView;
public:
	COptionView();
	virtual ~COptionView();
	
protected:
	
	virtual void  Layout(bool Redraw = true);
	virtual LRESULT ParentReaction(SpaceRectionMsg* SRM);
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
};

#endif // !defined(AFX_OPTIONVIEW_H__AF42315B_FA42_43A4_B179_A877B1F69B50__INCLUDED_)
