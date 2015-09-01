/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _TITLEVIEW_H__
#define _TITLEVIEW_H__


#include "PhysicSpace.h"
#include "WSTitleView.h"

using namespace VISUALSPACE;


#define  TITLE_VIEW   12    

class CTitleView : public CWSTitleView  
{
protected:
	Nerve*   m_Nerve;
public:
	CTitleView();
	virtual ~CTitleView();
    void SetID(int64 ID);
	void SetNerve(Nerve*  N){ m_Nerve = N;};
	void SetTitle(const TCHAR* Text);
	void SetIcon(HICON Icon);

protected:
    virtual void    NcLayout();
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
};


#endif // !defined(_BRAINVIEWCHILD_H__)
