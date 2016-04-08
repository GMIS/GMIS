// WebView.h: interface for the CWebView class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WEBVIEW_H__
#define _WEBVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WinSpace2.h"

#include <mshtml.h>  // Defines of stuff like IHTMLDocument2. This is an include file with Visual C 6 and above
#include <exdisp.h>  // Defines of stuff like IWebBrowser2. This is an include file with Visual C 6 and above

#include "WebBrowser.h"


#define  CM_MOUSE_MOVE        100
#define  CM_WEB_COMPLETED     101
#define  CM_WEB_BEGIN         102
#define  CM_CAPTURE_OBJECT    103
#define  CM_OBJECT_SELECTED   104
#define  CM_OCR_OBJECT        105
#define  CM_SET_CUR_OBJECT    106
#define  CM_DELETE_OBJECT     107
#define  CM_WEB_NEWWINDOW     108

using  namespace VISUALSPACE;

class CWebView; 
class CWebBrowserEventSink : public IDispatch{
public:
	HWND       m_Frame;
	CWebView*  m_View;
	tstring	   m_URL;
public:	
	CWebBrowserEventSink();
	virtual ~CWebBrowserEventSink();
	
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
		virtual ULONG STDMETHODCALLTYPE AddRef( void);
    virtual ULONG STDMETHODCALLTYPE Release( void) ;
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount( 
		/* [out] */ UINT __RPC_FAR *pctinfo);
        
		virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
		/* [in] */ UINT iTInfo,
		/* [in] */ LCID lcid,
		/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
		virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames( 
		/* [in] */ REFIID riid,
		/* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
		/* [in] */ UINT cNames,
		/* [in] */ LCID lcid,
		/* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
		virtual /* [local] */ HRESULT STDMETHODCALLTYPE Invoke( 
		/* [in] */ DISPID dispIdMember,
		/* [in] */ REFIID riid,
		/* [in] */ LCID lcid,
		/* [in] */ WORD wFlags,
		/* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
		/* [out] */ VARIANT __RPC_FAR *pVarResult,
		/* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
		/* [out] */ UINT __RPC_FAR *puArgErr);
};

class CHtmlDocumentEventSink : public IDispatch{
public:
	HWND       m_Frame;
	CWebView*  m_View;
public:	
	CHtmlDocumentEventSink():m_Frame(NULL){};
	virtual ~CHtmlDocumentEventSink(){};
	
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
		virtual ULONG STDMETHODCALLTYPE AddRef( void);
    virtual ULONG STDMETHODCALLTYPE Release( void) ;
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount( 
		/* [out] */ UINT __RPC_FAR *pctinfo);
        
		virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
		/* [in] */ UINT iTInfo,
		/* [in] */ LCID lcid,
		/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
		virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames( 
		/* [in] */ REFIID riid,
		/* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
		/* [in] */ UINT cNames,
		/* [in] */ LCID lcid,
		/* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
		virtual /* [local] */ HRESULT STDMETHODCALLTYPE Invoke( 
		/* [in] */ DISPID dispIdMember,
		/* [in] */ REFIID riid,
		/* [in] */ LCID lcid,
		/* [in] */ WORD wFlags,
		/* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
		/* [out] */ VARIANT __RPC_FAR *pVarResult,
		/* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
		/* [out] */ UINT __RPC_FAR *puArgErr);
};


class CWebView : public CWinSpace2  
{
public:
	BOOL						 m_Created;
	CWebBrowser				     m_WebBrowser;
	HWND						 m_IEFrame;


	HBITMAP                      m_BkgImag;

	CWebBrowserEventSink	     m_WebEventSink;    //处理必要的web事件
	CHtmlDocumentEventSink		 m_DocumentEventSink;
	BOOL                         m_bDocEventInited;

	POINTS  m_P1;  //begin drag point
	POINTS  m_P2;  //end drag point
	
	HBRUSH  m_HalfBrush; 
	BOOL    m_bDrag;

public:
	CWebView();
	virtual ~CWebView();
	BOOL InitWebBrowserEvent();
	BOOL InitDocumentEvent();
	virtual void  Layout(bool Redraw=TRUE);

protected:

	virtual void    Draw(HDC hDC,ePipeline* Pipe = NULL);

	virtual LRESULT Reaction(UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam); 
	LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);

};

#endif // !defined(_WEBVIEW_H__)
