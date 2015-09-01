/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _OPTIONVIEW_H__
#define _OPTIONVIEW_H__


#include "WSFrameView.h"
#include "WSFormView.h"

using namespace VISUALSPACE;

#define BNT_OK      198
#define BNT_CANCLE  199

class CFooterBar: public CWinSpace2
{
public:
	class ControlBnt : public ButtonItem
	{	
	public:
		ControlBnt(int64 ID,const TCHAR* Name)
			:ButtonItem(ID,Name,NULL,false){
		};
		virtual ~ControlBnt(){};
		virtual void Draw(HDC hDC,ePipeline* Pipe=NULL);
	};
public:
	
	ControlBnt        m_OKBnt;
	ControlBnt        m_CancleBnt;
	
	CVSpace2*         m_SpaceSelected;

public:
	CFooterBar();
	virtual ~CFooterBar();
	virtual  void    Layout(bool Redraw = true);
protected:
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	
protected:
	LRESULT OnPaint();
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);

};

class COptionView : public CWSFrameView  
{
public:
	CWSFormView    m_OptionView;
	CFooterBar     m_FooterBar;
public:
	COptionView();
	virtual ~COptionView();

protected:

	virtual void  Layout(bool Redraw = true);
    virtual LRESULT  ParentReaction(SpaceRectionMsg* SRM);
	
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	
};

#endif // !defined(_OPTIONVIEW_H__)
