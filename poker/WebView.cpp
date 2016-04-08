// WebView.cpp: implementation of the CWebView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)


#include "MainFrame.h"
#include "WebView.h"
#include <exdispid.h>  /* DISP_xxxx constants */
#include "WebHost.h"
#include <MsHtmdid.h>
#include <comutil.h>

#pragma comment(lib, "comsupp.lib") 

CWebBrowserEventSink::CWebBrowserEventSink()
:m_Frame(NULL),m_View(NULL)
{};

CWebBrowserEventSink::~CWebBrowserEventSink(){};


HRESULT STDMETHODCALLTYPE CWebBrowserEventSink::QueryInterface( 	/* [in] */ REFIID riid,
													/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (IsEqualIID(riid, IID_IUnknown))
	{
		*ppvObject = (IUnknown *) this;
	}
	else if (IsEqualIID(riid, IID_IDispatch))
	{
		*ppvObject = (IDispatch *) this;
	}
	// For other types of objects the browser wants, just report that we don't have any such objects.
	// NOTE: If you want to add additional functionality to your browser hosting, you may need to
	// provide some more objects here. You'll have to investigate what the browser is asking for
	// (ie, what REFIID it is passing).
	else
	{
		*ppvObject = 0;
		return(E_NOINTERFACE);
	}

	return(S_OK);
}

ULONG STDMETHODCALLTYPE CWebBrowserEventSink::AddRef( void)
{
	return 1;
}

ULONG STDMETHODCALLTYPE CWebBrowserEventSink::Release( void)
{
	return 0;
}

HRESULT STDMETHODCALLTYPE CWebBrowserEventSink::GetTypeInfoCount(
													  /* [out] */ UINT __RPC_FAR *pctinfo){
													  *pctinfo = 0;
return S_OK;
};

HRESULT STDMETHODCALLTYPE CWebBrowserEventSink::GetTypeInfo(
												 /* [in] */ UINT iTInfo,
												 /* [in] */ LCID lcid,
												 /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo){
												 *ppTInfo = NULL;
return TYPE_E_ELEMENTNOTFOUND;
};

HRESULT STDMETHODCALLTYPE CWebBrowserEventSink::GetIDsOfNames(
												   /* [in] */ REFIID riid,
												   /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
												   /* [in] */ UINT cNames,
												   /* [in] */ LCID lcid,
												   /* [size_is][out] */ DISPID __RPC_FAR *rgDispId){
												   *rgDispId = DISPID_UNKNOWN;
return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE CWebBrowserEventSink::Invoke(
											/* [in] */ DISPID dispIdMember,
											/* [in] */ REFIID riid,
											/* [in] */ LCID lcid,
											/* [in] */ WORD wFlags,
											/* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
											/* [out] */ VARIANT __RPC_FAR *pVarResult,
											/* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
											/* [out] */ UINT __RPC_FAR *puArgErr){
											if(dispIdMember == DISPID_PROGRESSCHANGE)
											{
												long progress = pDispParams->rgvarg[0].lVal;
												long progress_max = pDispParams->rgvarg[1].lVal;

												if(progress < 0  ||  progress_max < 0) {

													if(m_Frame==NULL)return 0;
													assert(m_View!=NULL);
													m_View->InitDocumentEvent();

												}
											}else if (dispIdMember == DISPID_DOCUMENTCOMPLETE)
											{
													SpaceRectionMsg SRM;

													SRM.Msg = CM_WEB_COMPLETED;
													SRM.wParam = 0;
													SRM.lParam = 0;
													SRM.ChildAffected = NULL;
													SRM.WinSpace = NULL;
													return ::SendMessage(m_Frame,WM_PARENTRECTION,(WPARAM)&SRM,0);
					
											}
											else if (dispIdMember == DISPID_BEFORENAVIGATE2)
											{
												if(m_Frame==NULL)return 0;
												SpaceRectionMsg SRM;

												SRM.Msg = CM_WEB_BEGIN;
												SRM.wParam = 0;
												SRM.lParam = 0;
												SRM.ChildAffected = NULL;
												SRM.WinSpace = NULL;
												return ::SendMessage(m_Frame,WM_PARENTRECTION,(WPARAM)&SRM,0);

											}else if (dispIdMember == DISPID_NEWWINDOW3)
											{
												VARIANT& arg1 = pDispParams->rgvarg[3];
												*arg1.pboolVal  = VARIANT_TRUE;                    

												VARIANTARG&  varMyURL = pDispParams->rgvarg[0];
												m_URL = varMyURL.bstrVal;


												if(m_Frame==NULL)return 0;
												SpaceRectionMsg SRM;

												SRM.Msg = CM_WEB_NEWWINDOW;
												SRM.wParam = (int64)&m_URL;
												SRM.lParam = 0;
												SRM.ChildAffected = NULL;
												SRM.WinSpace = NULL;
												::SendMessage(m_Frame,WM_PARENTRECTION,(WPARAM)&SRM,0);
											}
											else if (dispIdMember == DISPID_NEWWINDOW2)
											{

												/*
												VARIANT *pvCancel = pDispParams->rgvarg[0].pvarVal;
												pvCancel->boolVal=VARIANT_TRUE;
												*/
												*pDispParams->rgvarg[0].pboolVal = TRUE;
											
												if(m_Frame==NULL)return 0;
												SpaceRectionMsg SRM;

												if (m_URL.size()==0)
												{
													return 0;
												}
												SRM.Msg = CM_WEB_NEWWINDOW;
												SRM.wParam = (int64)&m_URL;
												SRM.lParam = 0;
												SRM.ChildAffected = NULL;
												SRM.WinSpace = NULL;
												::SendMessage(m_Frame,WM_PARENTRECTION,(WPARAM)&SRM,0);
											}
											else if (dispIdMember == DISPID_STATUSTEXTCHANGE)
											{
												_bstr_t  bs = pDispParams->rgvarg[0].bstrVal;
												
												m_URL = bs;
												
											}

											return S_OK;
};

HRESULT STDMETHODCALLTYPE CHtmlDocumentEventSink::QueryInterface( 	/* [in] */ REFIID riid,
													/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	if (IsEqualIID(riid, IID_IUnknown))
	{
		*ppvObject = (IUnknown *) this;
	}
	else if (IsEqualIID(riid, IID_IDispatch))
	{
		*ppvObject = (IDispatch *) this;
	}
	// For other types of objects the browser wants, just report that we don't have any such objects.
	// NOTE: If you want to add additional functionality to your browser hosting, you may need to
	// provide some more objects here. You'll have to investigate what the browser is asking for
	// (ie, what REFIID it is passing).
	else
	{
		*ppvObject = 0;
		return(E_NOINTERFACE);
	}

	return(S_OK);
}

ULONG STDMETHODCALLTYPE CHtmlDocumentEventSink::AddRef( void)
{
	return 1;
}

ULONG STDMETHODCALLTYPE CHtmlDocumentEventSink::Release( void)
{
	return 0;
}

HRESULT STDMETHODCALLTYPE CHtmlDocumentEventSink::GetTypeInfoCount(
													  /* [out] */ UINT __RPC_FAR *pctinfo){
													  *pctinfo = 0;
return S_OK;
};

HRESULT STDMETHODCALLTYPE CHtmlDocumentEventSink::GetTypeInfo(
												 /* [in] */ UINT iTInfo,
												 /* [in] */ LCID lcid,
												 /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo){
												 *ppTInfo = NULL;
return TYPE_E_ELEMENTNOTFOUND;
};

HRESULT STDMETHODCALLTYPE CHtmlDocumentEventSink::GetIDsOfNames(
												   /* [in] */ REFIID riid,
												   /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
												   /* [in] */ UINT cNames,
												   /* [in] */ LCID lcid,
												   /* [size_is][out] */ DISPID __RPC_FAR *rgDispId){
												   *rgDispId = DISPID_UNKNOWN;
return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE CHtmlDocumentEventSink::Invoke(
											/* [in] */ DISPID dispIdMember,
											/* [in] */ REFIID riid,
											/* [in] */ LCID lcid,
											/* [in] */ WORD wFlags,
											/* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
											/* [out] */ VARIANT __RPC_FAR *pVarResult,
											/* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
											/* [out] */ UINT __RPC_FAR *puArgErr){

											if(dispIdMember == DISPID_EVMETH_ONMOUSEDOWN){
												POINT p;
												p.x=0; p.y=0;

											}

											return S_OK;
};
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWebView::CWebView()
{
	m_DocumentEventSink.m_View = this;
	m_State=0;
	m_bDocEventInited= FALSE;

	m_bDrag=FALSE;
	m_BkgImag = NULL;

	m_HalfBrush = NULL;
    int16 Pattern[8];
	for (int i = 0; i < 8; i++)Pattern[i] = (int16)(0x5555 << (i & 1));
	HBITMAP Bitmap = ::CreateBitmap(4, 4, 1, 1, &Pattern);
	if (Bitmap != NULL)
	{
		m_HalfBrush = ::CreatePatternBrush(Bitmap);
		::DeleteObject(Bitmap);
	}

}

CWebView::~CWebView()
{
	if(m_HalfBrush){
		::DeleteObject(m_HalfBrush);
	}
	if (m_BkgImag)
	{
		DeleteObject(m_BkgImag);
	}
	
}



void CWebView::Layout(bool Redraw ){
	if(m_Created){
		RECT rcClient;
		::GetClientRect(GetHwnd(),&rcClient);
		if (m_BkgImag)
		{
			::MoveWindow(m_IEFrame,rcClient.right,rcClient.bottom,100,100,TRUE);
			Invalidate();
		}
		else
		{
			::MoveWindow(m_IEFrame,rcClient.left,rcClient.top,RectWidth(rcClient),RectHeight(rcClient),TRUE);
		}
	}
}
void  CWebView::Draw(HDC hDC,ePipeline* Pipe /*= NULL*/){
	if (!m_BkgImag)return;

	RECT rc;
    ::GetClientRect(GetHwnd(),&rc);

	HDC DCMem = ::CreateCompatibleDC(hDC);

	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, m_BkgImag);


	if(!BitBlt(hDC,
		0,0,
		RectWidth(rc), RectHeight(rc),
		DCMem,
		0,0,
		SRCCOPY))
    {
        MessageBox(GetHwnd(), L"BitBlt has failed", L"Failed", MB_OK);
    }

    ::SelectObject(DCMem, OldBitmap);
	DeleteObject(DCMem);

	AlphaBlendGlass(hDC, 0,0,RectWidth(rc), RectHeight(rc), RGB(0,0,0), 40);

	if (!m_bDrag)
	{
		return;
	}

	int32 PenW=1;
	if (m_HalfBrush != NULL){
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC,m_HalfBrush);
		//»­¾ØÐÎ

		int32 w = m_P2.x - m_P1.x;
        int32 h = m_P2.y - m_P1.y;

		::PatBlt(hDC,m_P1.x, m_P1.y,w, PenW, PATINVERT);
		::PatBlt(hDC,m_P1.x, m_P1.y, PenW, h, PATINVERT);

		::PatBlt(hDC,m_P2.x, m_P1.y, PenW, h, PATINVERT);
		::PatBlt(hDC,m_P1.x,m_P2.y, w, PenW, PATINVERT);

		::SelectObject(hDC, hOldBrush);
	}
}


BOOL CWebView::InitWebBrowserEvent(){
	IWebBrowser2* WebBrowser = m_WebBrowser.GetWebBrowser();

	IConnectionPointContainer* conn_point_container;
    IConnectionPoint* conn_point;
    DWORD cookie;
    HRESULT hr;

	m_WebEventSink.m_Frame = GetParent(GetHwnd());
	m_WebEventSink.m_View  = this;

	hr = WebBrowser->QueryInterface(IID_IConnectionPointContainer, (void**)&conn_point_container);
    if(hr != S_OK) {
        return FALSE;
    }

    hr = conn_point_container->FindConnectionPoint(DIID_DWebBrowserEvents2, &conn_point);
	conn_point_container->Release();

    if(hr != S_OK) {
		return FALSE;
    }
    conn_point->Advise((IUnknown*)&m_WebEventSink, &cookie);
    conn_point->Release();
	return TRUE;
}



BOOL CWebView::InitDocumentEvent(){

	if (m_bDocEventInited)
	{
		return TRUE;
	};

	IWebBrowser2* WebBrowser = m_WebBrowser.GetWebBrowser();

	IConnectionPointContainer* conn_point_container;
    IConnectionPoint* conn_point;
    DWORD cookie;
    HRESULT hr;

	IDispatch* spDisp;
	hr = WebBrowser->get_Document(&spDisp);
    assert( SUCCEEDED( hr ) );

	IHTMLDocument2 *spDoc = NULL;
	hr = spDisp->QueryInterface( IID_IHTMLDocument2, ( void** )&spDoc );
	assert( SUCCEEDED( hr ) );

	hr = spDoc->QueryInterface(IID_IConnectionPointContainer, (void**)&conn_point_container);
    if(hr != S_OK) {
        return FALSE;
    }

    hr = conn_point_container->FindConnectionPoint(DIID_HTMLDocumentEvents, &conn_point);
	conn_point_container->Release();

    if(hr != S_OK) {
		return FALSE;
    }
    conn_point->Advise((IUnknown*)&m_DocumentEventSink, &cookie);
    conn_point->Release();

	m_bDocEventInited = TRUE;

	return TRUE;
}

LRESULT CWebView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
   switch(message)
   {
   case WM_ERASEBKGND:
	   return 0;
   case WM_PAINT:
	   return OnPaint(wParam,lParam);
   case WM_MOUSEMOVE:
	   return OnMouseMove(wParam,lParam);
   case WM_LBUTTONDOWN:
	   return OnLButtonDown(wParam,lParam);
   case WM_LBUTTONUP:
		return OnLButtonUp(wParam,lParam);
   case WM_CREATE:
		return OnCreate(wParam,lParam);
   	break;
   case WM_SIZE:
	   Layout(TRUE);
       return 0;
   case WM_DESTROY:
	   m_WebBrowser.UnEmbedBrowserObject(GetHwnd());
   default:
		return CWinSpace2::Reaction(message,wParam,lParam);
   }
}

LRESULT CWebView::OnCreate(WPARAM wParam, LPARAM lParam){
	DWORD style = GetWindowLong(m_hWnd,GWL_STYLE);
	style |=WS_CLIPCHILDREN;
	SetWindowLong(m_hWnd,GWL_STYLE,style);

	m_WebBrowser.EmbedBrowserObject(GetHwnd());

	m_WebBrowser.GetWebBrowser()->put_Silent(VARIANT_TRUE);

	m_WebBrowser.DisplayHTMLPage(GetHwnd(), _T("about:blank"));

	//m_WebBrowser.DisplayHTMLPage(GetHwnd(), _T("file:///D:/project/compile/VC/WebRobot/test.html"));
	//m_WebBrowser.DisplayHTMLPage(GetHwnd(), _T("http://apps.renren.com/boyaa_texas/index.php?origin=103"));
	//m_WebBrowser.DisplayHTMLPage(GetHwnd(), _T("http://game.weibo.com/texaspoker?origin=1002"));

    m_IEFrame =::FindWindowEx(GetHwnd(),NULL,_T("Shell Embedding"),NULL);

    if (!InitWebBrowserEvent())
    {
		return -1;
    }


	m_Created = true;
	return 0;
}
LRESULT CWebView::OnPaint(WPARAM wParam, LPARAM lParam)
{
	RECT rcWin;
	GetClientRect(m_hWnd,&rcWin);

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hWnd, &ps);

	if(rcWin.right==rcWin.left || rcWin.top==rcWin.bottom){
		::EndPaint(m_hWnd, &ps);
		return 0;
	}

	HDC DCMem = ::CreateCompatibleDC(hdc);
	HBITMAP bmpCanvas=::CreateCompatibleBitmap(hdc, RectWidth(rcWin),RectHeight(rcWin));
	assert(bmpCanvas);
	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, bmpCanvas );

	UINT	OldMode	= ::SetBkMode(DCMem,TRANSPARENT );
    Draw(DCMem);
	::SetBkMode(DCMem, OldMode );


	::BitBlt(hdc, 0, 0, RectWidth(rcWin), RectHeight(rcWin), DCMem, 0, 0, SRCCOPY );
	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);
	::EndPaint(m_hWnd, &ps);
	return 0;
}

LRESULT CWebView::OnMouseMove(WPARAM wParam, LPARAM lParam){
	
	if (m_bDrag){
		m_P2 = MAKEPOINTS(lParam);
		Invalidate();

	}
	return 0;
};

LRESULT CWebView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	::SetCapture(GetHwnd());
	POINTS p = MAKEPOINTS(lParam);
    m_P1 = p;
	m_bDrag = TRUE;
	Invalidate();
	return 0;
};

LRESULT CWebView::OnLButtonUp(WPARAM wParam, LPARAM lParam){
	::ReleaseCapture();
	m_P2 = MAKEPOINTS(lParam);
	if (m_bDrag)
	{
		if (m_P1.x != m_P2.x && m_P1.y != m_P2.y)
		{
			SendParentMessage(CM_OBJECT_SELECTED,0,0,NULL);
		}
		m_bDrag = FALSE;
	}
	Invalidate();
	return 0;
};
