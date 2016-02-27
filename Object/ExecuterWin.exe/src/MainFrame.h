/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/
#ifndef _MAINFRAME_H__
#define _MAINFRAME_H__

#include "space.h"
#include "VisibleSpace.h"
#include "EditLog.h"
#include "editlog_stream.h"
#include "WSFrameView.h"

using namespace VISUALSPACE;

class CMainFrame : public CWSFrameView   
{		
protected: //GUI

	BOOL          m_Created;
	HWND          m_hEdit;
	CEditLog      m_AppLog;
	
	// ANSI and UNICODE stream buffers for the EditLogger
	std::editstreambuf	m_EditStrBuf;
	std::weditstreambuf m_EditStrBufW;
	
	// Used to save the previos values for cout and wcout
	std::basic_streambuf<char>*		m_OldBuf;
	std::basic_streambuf<wchar_t>*	m_OldBufW;
	
	HBRUSH                          m_BKBrush;
	
public:
	CMainFrame();
	virtual ~CMainFrame();
	
protected:

	virtual void Layout(bool Redraw  = true );
	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(WPARAM wParam, LPARAM lParam);

};

#endif // _MAINFRAME_H__