// OcrHost.h: interface for the COcrHost class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OCRHOST_H__3D1B6F15_E4C2_4A34_BCE5_7EAACF817626__INCLUDED_)
#define AFX_OCRHOST_H__3D1B6F15_E4C2_4A34_BCE5_7EAACF817626__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TheorySpace.h"

#include <Exdisp.h>  /* Defines of stuff like IWebBrowser2. This is an include file with Visual C 6 and above */
#include <mshtml.h>  // Defines of stuff like IHTMLDocument2. This is an include file with Visual C 6 and above


HRESULT __stdcall MyCoCreateInstance(
									 LPCTSTR szDllName,
									 IN REFCLSID rclsid,
									 IUnknown* pUnkOuter,
									 IN REFIID riid,
									 OUT LPVOID FAR* ppv);

class COcrHost  
{
public:
	COcrHost();
	virtual ~COcrHost();

	BOOL Init();
    
	BOOL UseOcr(tstring ImageFile,tstring& Output);

};

#endif // !defined(AFX_OCRHOST_H__3D1B6F15_E4C2_4A34_BCE5_7EAACF817626__INCLUDED_)
