/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _MAINFRAME_H__
#define _MAINFRAME_H__

#include "LogView.h"
#include "StatusTip.h"
#include "WSFrameView.h"


#define  TIMER_HIDEWIN 99

struct SPACE_ACCOUNT{
	tstring Name;
	tstring Password;
	tstring Confirm;
	tstring CrypText;
	tstring LocalName;
	uint32  OuterIP;
};


class CMainFrame : public CWSFrameView   
{
	BOOL				m_Created;
public:

	CLogView			m_LogView;
	CStatusTip			m_Status;	
	int					m_SecondHide;   //defualt = 10; 
	int					m_Second;

//Construction/Destruction
//////////////////////////////////////////////////////////
public:
	CMainFrame();
	virtual ~CMainFrame();
		
	void AddRTInfo(const TCHAR* Text);
	void SetStatus(const TCHAR* Text);

protected:
	
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void  Layout(bool Redraw=true);

	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
};


CMainFrame*  GetGUI();

#endif // !defined(_MAINFRAME_H__)
