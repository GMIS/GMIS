// WebHost.h: interface for the CWebHost class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WEBHOST_H__897C5A9B_C950_4E1A_B985_25B59C01A081__INCLUDED_)
#define AFX_WEBHOST_H__897C5A9B_C950_4E1A_B985_25B59C01A081__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TheorySpace.h"
#include "SubclassWnd.h"
#include "WinSpace2.h"
#include "WebView.h"

using namespace VISUALSPACE;


HBITMAP CaptureImageFromBmp(HWND hWnd,HBITMAP srcBmp,int x, int y,int w, int h);
HBITMAP CaptureImageFromWin(HWND hWnd,int x, int y, int w, int h);
HBITMAP CaptureImageFromWeb(IWebBrowser2* WebBrowSer,HWND hWndFrame,int32 w=0,int32 h=0);//缺省整个页面
BOOL    SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName); 


class CMainFrame;

class CWebHost:public CSubclassWnd  
{
protected:
	CMainFrame*		m_MainFrame;
	BOOL			m_bSaveBmp;
public:
	tstring         m_TempDir;

//	POINT           m_ScrollPos;
//	POINT           m_FramePos;

	POINT           m_OffsetPos;


	CWebView        m_WebView;
public:
	CWebHost(CMainFrame* MainFrame);
	virtual ~CWebHost();


	void ScrollTo(int32 dx,int32 dy);

	void Click(POINT& p);
	void Click(ePipeline* Ob);

	void AuotFillForm(tstring UserName,tstring Password);

	HBITMAP ScreenShot(BOOL bSaveFile=TRUE,int32 w=0,int32 h=0);

	BOOL SubclassWebWnd();

	LRESULT SendParentMessage(UINT Msg,int64 wParam, int64 lParam);

	void Close(){
		UnsubclassWindow();
	}


	void StartSelectObject();
	BOOL GetSelectedObject(int64 ID,CObjectData& ObjectData,BOOL bSaveFile);
	void EndSelectObject();

	//返回当前的页面的Scrollbar Pos
	void GetWebScrollPos(POINT& p);

	tstring GetUrl();

	IHTMLElement2* FindGameFrame();

	//返回游戏Frame相对于窗口的位置坐标（无需在关心Scroll Pos)
	void GetFramePos(POINT& p);

	HBITMAP CaptureFrameFromWeb(int32 w,int32 h);

protected:
	virtual BOOL ProcessWindowMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult );
};

#endif // !defined(AFX_WEBHOST_H__897C5A9B_C950_4E1A_B985_25B59C01A081__INCLUDED_)
