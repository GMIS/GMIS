// WebHost.cpp: implementation of the CWebHost class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning (disable: 4786)

#include "MainFrame.h"
#include "WebHost.h"
#include "BitmapEx.h"
//#include "format.h"


HBITMAP  CaptureImageFromBmp(HWND hWnd,HBITMAP srcBmp,int x, int y,int w, int h)
{
	if(w==0 || h==0)return NULL;

	HDC hDC  = GetDC(hWnd);

	HDC hMemDC = CreateCompatibleDC(hDC); 	
	HDC hMemDC2 = CreateCompatibleDC(hDC); 	

	HBITMAP hBitmapCapture = CreateCompatibleBitmap(hDC,w,h);

	HBITMAP OldBmpFromMemDC = (HBITMAP)SelectObject(hMemDC,hBitmapCapture);
	HBITMAP OldBmpFromMemDC2 = (HBITMAP)SelectObject(hMemDC2,srcBmp);

	// Bit block transfer into our compatible memory DC.
	if(!BitBlt(hMemDC, 
		0,0, 
		w, h, 
		hMemDC2, 
		x,y,
		SRCCOPY))
	{
		//MessageBox(NULL, L"BitBlt has failed", L"Failed", MB_OK);
		DeleteObject(hBitmapCapture);
		hBitmapCapture =  NULL;
	}

	SelectObject(hMemDC,OldBmpFromMemDC);
	SelectObject(hMemDC2,OldBmpFromMemDC2);
	DeleteObject(hMemDC);
	DeleteObject(hMemDC2);
	ReleaseDC(hWnd,hDC);
	return hBitmapCapture;
}
HBITMAP CaptureImageFromWeb(IWebBrowser2* WebBrowser2,HWND hWndFrame,int32 w,int32 h)
{
	IViewObject     *pViewObject	= NULL; 
	IOleObject      *pOleObject		= NULL; 
	IDispatch       *pDoc			= NULL;   
	IHTMLDocument2  *pDocument		= NULL;
	IHTMLElement    *pElement		= NULL;	
	HBITMAP          hBmpCapture    = NULL;

	{

		VARIANT vtAttrib;
		HRESULT hr;

		int ScrollWidth = 0;
		int ScrollHeight = 0;

		hr = WebBrowser2->get_Document(&pDoc);   
		if (FAILED(hr)) {
			MessageBox(NULL, L"GetDocument has failed",L"Failed", MB_OK);
			goto done;
		}	

		hr = pDoc->QueryInterface(IID_IHTMLDocument2, (void**)&pDocument);
		if (FAILED(hr)) {
			MessageBox(NULL, L"GetIHTMLDocument2 has failed",L"Failed", MB_OK);
			goto done;
		}

		hr = pDocument->QueryInterface(IID_IViewObject2, (void**)&pViewObject);
		if(FAILED(hr)) {
			MessageBox(NULL, L"GetIViewObject2 has failed",L"Failed", MB_OK);
			goto done;
		}

		hr = pDocument->get_body(&pElement);
		if (FAILED(hr)) {
			MessageBox(NULL, L"GetHTMLElement has failed",L"Failed", MB_OK);
			goto done;
		}

		pElement->getAttribute( L"scrollWidth", 0, &vtAttrib );
		assert( vtAttrib.vt != VT_NULL && vtAttrib.vt == VT_I4 );
		ScrollWidth = vtAttrib.intVal+25;

		pElement->getAttribute( L"scrollHeight", 0, &vtAttrib );
		assert( vtAttrib.vt != VT_NULL && vtAttrib.vt == VT_I4 );
		ScrollHeight = vtAttrib.intVal;

		RECT rc;
		::GetWindowRect(hWndFrame,&rc);

		if(w==0)w = ScrollWidth;
		if(h==0)h = ScrollHeight;

		//记住网页窗口在父窗口的原始位置
		HWND Parent = GetParent(hWndFrame);
		POINT p1,p2;
		p1.x = rc.left; p1.y = rc.top;
		p2.x = rc.right; p2.y = rc.bottom;
		::ScreenToClient(Parent,&p1);
		::ScreenToClient(Parent,&p2);

		//移动窗口使网页窗口不可见
		::MoveWindow(hWndFrame,2000,0,w,h,TRUE);

		HDC hDC  = GetDC(hWndFrame);
		HDC hMemDC = CreateCompatibleDC(hDC); 	
		hBmpCapture = CreateCompatibleBitmap(hDC,w,h); 
		HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC,hBmpCapture);

		rc.left = 0; 
		rc.top  = 0; 
		rc.right  = w; 
		rc.bottom = h;

		::OleDraw(pViewObject,DVASPECT_CONTENT,hMemDC,&rc);

		SelectObject(hMemDC,hOldBmp);
		DeleteObject(hMemDC);

		//恢复网页窗口位置
		::MoveWindow(hWndFrame,p1.x,p1.y,p2.x-p1.x,p2.y-p1.y,TRUE);

	}
done:
	if(pViewObject)pViewObject->Release();
	if(pDocument)pDocument->Release();
	if(pElement)pElement->Release();
	if(pDoc)pDoc->Release();
	return hBmpCapture;
};
HBITMAP CaptureImageFromWin(HWND hWnd,int x, int y, int w, int h)
{
	HDC hDC;
	HDC hMemDC = NULL;
	HBITMAP hBitmapCapture = NULL;
	HBITMAP hOldBmp;

	hDC  = GetDC(hWnd);
	hMemDC = CreateCompatibleDC(hDC); 

	if(!hMemDC)
	{
		MessageBox(NULL, L"CreateCompatibleDC has failed",L"Failed", MB_OK);
		goto done;
	}

	// Create a compatible bitmap from the Window DC
	hBitmapCapture = CreateCompatibleBitmap(hDC,w,h);
	if(!hBitmapCapture )
	{
		MessageBox(NULL, L"CreateCompatibleBitmap Failed",L"Failed", MB_OK);
		goto done;
	}

	hOldBmp = (HBITMAP)SelectObject(hMemDC,hBitmapCapture);


	// Bit block transfer into our compatible memory DC.
	if(!BitBlt(hMemDC, 
		0,0, 
		w, h, 
		hDC, 
		x,y,
		SRCCOPY))
	{
		MessageBox(NULL, L"BitBlt has failed", L"Failed", MB_OK);
		DeleteObject(hBitmapCapture);
		goto done;
	}



	SelectObject(hMemDC,hOldBmp);
done:
	if(hMemDC)DeleteObject(hMemDC);
	ReleaseDC(hWnd,hDC);

	return hBitmapCapture;
};

BOOL SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName ) 
{ 
	HDC hDC; //设备描述表 
	int iBits; //当前显示分辨率下每个像素所占字节数 
	WORD wBitCount; //位图中每个像素所占字节数 
	DWORD dwPaletteSize=0, //定义调色板大小， 位图中像素字节大小 ，位图文件大小 ， 写入文件字节数 
		dwBmBitsSize, 
		dwDIBSize, dwWritten; 
	BITMAP Bitmap; //位图属性结构 
	BITMAPFILEHEADER bmfHdr; //位图文件头结构 
	BITMAPINFOHEADER bi; //位图信息头结构 
	LPBITMAPINFOHEADER lpbi; //指向位图信息头结构 

	HANDLE fh, hDib, hPal,hOldPal=NULL; //定义文件，分配内存句柄，调色板句柄 

	//计算位图文件每个像素所占字节数 
	HDC hWndDC = CreateDC(_T("DISPLAY"),NULL,NULL,NULL); 
	hDC = ::CreateCompatibleDC( hWndDC ) ; 
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES); 
	DeleteDC(hDC); 

	if (iBits <= 1) 
		wBitCount = 1; 
	else if (iBits <= 4) 
		wBitCount = 4; 
	else if (iBits <= 8) 
		wBitCount = 8; 
	else if (iBits <= 24) 
		wBitCount = 24; 
	else 
		wBitCount = 24 ; 

	//计算调色板大小 
	if (wBitCount <= 8) 
		dwPaletteSize = (1 << wBitCount) * sizeof(RGBQUAD); 

	//设置位图信息头结构 
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap); 
	bi.biSize = sizeof(BITMAPINFOHEADER); 
	bi.biWidth = Bitmap.bmWidth; 
	bi.biHeight = Bitmap.bmHeight; 
	bi.biPlanes = 1; 
	bi.biBitCount = wBitCount; 
	bi.biCompression = BI_RGB; 
	bi.biSizeImage = 0; 
	bi.biXPelsPerMeter = 0; 
	bi.biYPelsPerMeter = 0; 
	bi.biClrUsed = 0; 
	bi.biClrImportant = 0; 

	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount+31)/32) * 4 * Bitmap.bmHeight ; 

	//为位图内容分配内存 
	hDib = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER)); 
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib); 
	*lpbi = bi; 

	// 处理调色板 
	hPal = GetStockObject(DEFAULT_PALETTE); 
	if (hPal) 
	{ 
		hDC = ::GetDC(NULL); 
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE); 
		RealizePalette(hDC); 
	} 

	// 获取该调色板下新的像素值 
	GetDIBits(hDC, hBitmap, 0, (UINT) Bitmap.bmHeight, 
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) 
		+dwPaletteSize, 
		(LPBITMAPINFO ) 
		lpbi, DIB_RGB_COLORS); 

	//恢复调色板 
	if (hOldPal) 
	{ 
		SelectPalette(hDC, (HPALETTE)hOldPal, TRUE); 
		RealizePalette(hDC); 
		::ReleaseDC(NULL, hDC); 
	} 

	//创建位图文件 
	fh = CreateFile(lpFileName, GENERIC_WRITE, 
		0, NULL, CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL); 

	if (fh == INVALID_HANDLE_VALUE) 
		return FALSE; 

	// 设置位图文件头 
	bmfHdr.bfType = 0x4D42; // "BM" 
	dwDIBSize = sizeof(BITMAPFILEHEADER) 
		+ sizeof(BITMAPINFOHEADER) 
		+ dwPaletteSize + dwBmBitsSize; 
	bmfHdr.bfSize = dwDIBSize; 
	bmfHdr.bfReserved1 = 0; 
	bmfHdr.bfReserved2 = 0; 
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) 
		+ (DWORD)sizeof(BITMAPINFOHEADER) 
		+ dwPaletteSize; 

	// 写入位图文件头 
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL); 

	// 写入位图文件其余内容 
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, 
		&dwWritten, NULL); 

	//清除 
	GlobalUnlock(hDib); 
	GlobalFree(hDib); 
	CloseHandle(fh); 

	return TRUE; 

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWebHost::CWebHost(CMainFrame* MainFrame)
:m_MainFrame(MainFrame)
{
	m_bSaveBmp = FALSE;
//	m_BmpCapture = NULL;
//	m_ScrollPos.x = 0;
//	m_ScrollPos.y = 0;
//	m_FramePos.x = 0;
//	m_FramePos.y = 0;

	TCHAR lpBuffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,lpBuffer);
	m_TempDir = Format1024(_T("%s\\temp\\"),lpBuffer);
	_tmkdir(m_TempDir.c_str());
}

CWebHost::~CWebHost()
{

}

void  CWebHost::ScrollTo(int32 dx,int32 dy){
	
	IWebBrowser2* WebBrowser = m_WebView.m_WebBrowser.GetWebBrowser();
	HRESULT hr;
	
	IDispatch* spDisp;
	hr = WebBrowser->get_Document(&spDisp);
    assert( SUCCEEDED( hr ) );

	IHTMLDocument3 *spDoc3 = NULL;
	hr = spDisp->QueryInterface( IID_IHTMLDocument3, ( void** )&spDoc3 );
	assert( SUCCEEDED( hr ) );	
	
	if (spDoc3)
	{
		IHTMLElement* pElement = NULL;
		hr = spDoc3->get_documentElement( &pElement );
		assert( SUCCEEDED( hr ) );
		
		IHTMLElement2* pElement2 = NULL;
		hr = pElement->QueryInterface( IID_IHTMLElement2, ( void** )&pElement2 );
		assert( SUCCEEDED( hr ) );

		pElement2->put_scrollLeft(dx);
		pElement2->put_scrollTop(dy);
		
		pElement2->Release();
		pElement->Release();
		spDoc3->Release();
	}
	
	spDisp->Release();

}

void  CWebHost::Click(POINT& p){
	IDispatch* pDoc = NULL;   

    HRESULT hr = m_WebView.m_WebBrowser.GetWebBrowser()->get_Document(&pDoc);   
    if( SUCCEEDED(hr) && pDoc)   
    {   
        IHTMLDocument2* pHTMLDoc = NULL;   
        IDispatch*  pDispatch=NULL;    
        pDoc->QueryInterface(IID_IHTMLDocument2, (void**)&pHTMLDoc);   
        pDoc->QueryInterface(IID_IDispatch, (void**)&pDispatch);   
        if(SUCCEEDED(hr) && pHTMLDoc != NULL)   
        {   
            IHTMLElement* pElement = NULL;   
            hr = pHTMLDoc->elementFromPoint(p.x,p.y,&pElement);   
            if ( SUCCEEDED(hr) && pElement )   
            {   
                pElement->click();   
            }              
        }   
        pDispatch->Release();   
        pHTMLDoc->Release();   
        pDoc->Release();   
    }   
}

void CWebHost::Click(ePipeline* Ob){
	int32      x = *(int32*)Ob->GetData(3);
	int32      y = *(int32*)Ob->GetData(4);
	int32      w = *(int32*)Ob->GetData(5);
	int32      h = *(int32*)Ob->GetData(6);

	POINT p;
	p.x = x+w/2;
	p.y = y+h/2;
	Click(p);
}

void CWebHost::AuotFillForm(tstring UserName,tstring Password){
	IDispatch* pDoc = NULL;   
	
    HRESULT hr = m_WebView.m_WebBrowser.GetWebBrowser()->get_Document(&pDoc);   
    if( SUCCEEDED(hr) && pDoc)   
    {   
        IHTMLDocument3* pHTMLDoc = NULL;   
        pDoc->QueryInterface(IID_IHTMLDocument3, (void**)&pHTMLDoc);   
        if(SUCCEEDED(hr) && pHTMLDoc != NULL)   
        {   
            IHTMLElement* pElement = NULL;
           	
            BSTR bsValue     = ::SysAllocString(_T("value"));
          
			//填充用户名
			BSTR bsUserLabel =  ::SysAllocString(_T("email"));
			BSTR bsUserName =   ::SysAllocString(UserName.c_str());
			hr = pHTMLDoc->getElementById(bsUserLabel,&pElement);   
            if ( SUCCEEDED(hr) && pElement )   
            {   
				VARIANT v;
				v.vt = VT_BSTR;
				v.bstrVal = bsUserName; 
				pElement->setAttribute(bsValue,v);
				pElement->Release();
                pElement = NULL;
            }           		
            SysFreeString(bsUserLabel);
            SysFreeString(bsUserName);


			//填充密码
			BSTR bsPassLabel =  ::SysAllocString(_T("password"));
			BSTR bsPassword =   ::SysAllocString(Password.c_str());
			hr = pHTMLDoc->getElementById(bsPassLabel,&pElement);   
            if ( SUCCEEDED(hr) && pElement )   
            {   
				VARIANT v;
				v.vt = VT_BSTR;
				v.bstrVal = bsPassword; 
                pElement->setAttribute(bsValue,v);   
				pElement->Release();
				pElement = NULL;
            } 
            SysFreeString(bsPassLabel);
            SysFreeString(bsPassword);
			
			//登录
			BSTR bsSubmit =  ::SysAllocString(_T("poplogin"));
			hr = pHTMLDoc->getElementById(bsPassLabel,&pElement);   
            if ( SUCCEEDED(hr) && pElement )   
            {   
			    pElement->click();
				pElement->Release();
				pElement = NULL;
            } 
            SysFreeString(bsSubmit);
        }   
        pHTMLDoc->Release();   
        pDoc->Release();   
    }   
}

HBITMAP CWebHost::ScreenShot(BOOL bSaveFile,int32 w,int32 h){
	
	POINT ScrollPos;
	GetWebScrollPos(ScrollPos);

	ScrollTo(0,0);

	IWebBrowser2* WebBrowser2 = m_WebView.m_WebBrowser.GetWebBrowser();
	HWND hIEFrame = m_WebView.m_IEFrame;

	HBITMAP BmpCapture = CaptureImageFromWeb(WebBrowser2,hIEFrame);
	if (BmpCapture && bSaveFile)
	{
		CBitmapEx bmp;
		bmp.Load(BmpCapture);
		bmp.Save(_T("ScreenShot.bmp"));
	}
	ScrollTo(ScrollPos.x,ScrollPos.y);

	return BmpCapture;
}



void CWebHost::StartSelectObject(){


	RECT rc;
	::GetClientRect(m_WebView.GetHwnd(),&rc);
	

	int32 w = RectWidth(rc);
	int32 h = RectHeight(rc);

	HBITMAP BmpCapture = CaptureImageFromWin(m_WebView.GetHwnd(),rc.left,rc.top,w,h);

	//1
	SaveBitmapToFile(BmpCapture,_T("test.bmp"));

	//or 2   HBITMAP BmpCapture = CaptureFrameFromWeb(w,h);
	
	/*or 3 
	IWebBrowser2* WebBrowser2 = m_WebView.m_WebBrowser.GetWebBrowser();
	HWND hIEFrame = m_WebView.m_IEFrame;

	HBITMAP BmpCapture = CaptureImageFromWeb(WebBrowser2,hIEFrame);
	*/
    if(BmpCapture==NULL){
		return;
	};

	if (m_WebView.m_BkgImag)
	{
		::DeleteObject(m_WebView.m_BkgImag);
	}
	m_WebView.m_BkgImag = BmpCapture;

	m_WebView.Layout();

	//BOOL ret = m_MainFrame->CalculateOffsetPos(m_BmpCapture);  //再此截图中找基准点位置，这也意味着截图必须保证含有基准图像，除非是截取基准图像本身
	
};

BOOL CWebHost::GetSelectedObject(int64 ID,CObjectData& ObjectData,BOOL bSaveFile){
	//根据选择的坐标位置截图
	int x = m_WebView.m_P1.x;
	int y = m_WebView.m_P1.y;
	int w = m_WebView.m_P2.x - m_WebView.m_P1.x;
	int h = m_WebView.m_P2.y - m_WebView.m_P1.y;

	if (w==0 || h==0)
	{
		return FALSE;
	}
	if (w<0 )
	{
		x = m_WebView.m_P2.x;
		w = -w;
	}
	if (h<0)
	{
		y = m_WebView.m_P2.y;
		h = -h;
	}

	assert(m_WebView.m_BkgImag);

    HBITMAP BmpObject = CaptureImageFromBmp(GetHandle(),m_WebView.m_BkgImag,x,y,w,h);

	if (BmpObject == NULL)
	{
		return FALSE;
	}

	if (bSaveFile)
	{
		tstring Name = tformat(_T("%sx%dy%dw%dh%d.bmp"),m_TempDir.c_str(),x,y,w,h);

		CBitmapEx BmpFile;
		BmpFile.Load(BmpObject);
		BmpFile.Save((LPTSTR)Name.c_str());
		
		ObjectData.GetLabel() = Name;
	}
	
	ObjectData.PushInt(ID);              //占位，不使用
	ObjectData.PushString(_T(""));         //占位，不使用
    ObjectData.PushInt(0);               //占位，不使用
    ObjectData.PushInt(x);
	ObjectData.PushInt(y);
	ObjectData.PushInt(w);
	ObjectData.PushInt(h);
    
	ObjectData.PushInt(0); //crc32 占位，不使用

	tstring ocrTxt;   //当前只占位
	ObjectData.PushString(ocrTxt);

	ObjectData.PushInt((uint32)BmpObject);

	return TRUE;
};	

void CWebHost::EndSelectObject(){
	if (m_WebView.m_BkgImag)
	{
		::DeleteObject(m_WebView.m_BkgImag);
		m_WebView.m_BkgImag = NULL;
	}
	m_WebView.Layout();
};



IHTMLElement2* CWebHost::FindGameFrame(){

	IHTMLElement2* Result = NULL;

	IWebBrowser2* WebBrowser = m_WebView.m_WebBrowser.GetWebBrowser();
	HRESULT hr;

	IDispatch* spDisp;
	hr = WebBrowser->get_Document(&spDisp);
	assert( SUCCEEDED( hr ) );

	IHTMLDocument3 *spDoc3 = NULL;
	hr = spDisp->QueryInterface( IID_IHTMLDocument3, ( void** )&spDoc3 );
	assert( SUCCEEDED( hr ) );

	long top=0, left=0,right=0,bottom=0;

	VARIANT    vIndex;
	vIndex.vt = VT_I4;

	if (spDoc3)
	{

		BSTR bs = ::SysAllocString(_T("IFRAME"));
		IHTMLElementCollection*   pElemCollFrame = NULL; 
		hr=spDoc3-> getElementsByTagName(bs,&pElemCollFrame); 
		assert( SUCCEEDED( hr ) );

		::SysFreeString(bs);

		if   (pElemCollFrame) 
		{ 
			long   pLength; 
			hr=pElemCollFrame-> get_length(&pLength); 
			if(hr==S_OK) 
			{ 
				for(int   i=0;i <pLength;i++) 
				{ 
					IDispatch   *pDispFrame=NULL; 
					vIndex.lVal =i; 
					hr=pElemCollFrame-> item(vIndex,vIndex,&pDispFrame); 
					if(hr==S_OK) 
					{ 
						IHTMLElement2* pElement2 = NULL;
						hr = pDispFrame->QueryInterface( IID_IHTMLElement2, ( void** )&pElement2 );
						assert( SUCCEEDED( hr ) );

						IHTMLRect* pRect;
						pElement2->getBoundingClientRect(&pRect);


						pRect->get_left(&left);
						pRect->get_top(&top);
						pRect->get_right(&right);
						pRect->get_bottom(&bottom);

						pRect->Release();

						int w = right-left;
						int h = bottom - top;
						if (w>700 && h>1000)
						{

							Result = pElement2;
							break;
						}

						//pElement2->Release();
					}
				}
			}
			pElemCollFrame->Release();
		}
		spDoc3->Release();
	}

	spDisp->Release();
	return Result;
}

void CWebHost::GetFramePos(POINT& p){

	p.x = 0;
	p.y = 0;

	IHTMLElement2* pElement2 = FindGameFrame();
	if (pElement2)
	{
		IHTMLRect* pRect;
		pElement2->getBoundingClientRect(&pRect);


		pRect->get_left(&p.x);
		pRect->get_top(&p.y);
		pElement2->Release();
	}	
};


tstring CWebHost::GetUrl(){
	IWebBrowser2* WebBrowser = m_WebView.m_WebBrowser.GetWebBrowser();
	HRESULT hr;

	BSTR bs;
	hr = WebBrowser->get_LocationURL(&bs);
	assert( SUCCEEDED( hr ) );

	tstring url;

	UINT len = ::SysStringLen(bs);
	LPTSTR sz = new TCHAR[len+1];
	_tcsncpy(sz, bs, len);
	url = sz;
	delete []sz;

	::SysFreeString(bs);
	return url;
}

void CWebHost::GetWebScrollPos(POINT& p){
	p.x = 0;
	p.y = 0;

	IWebBrowser2* WebBrowser = m_WebView.m_WebBrowser.GetWebBrowser();
	HRESULT hr;

	IDispatch* spDisp;
	hr = WebBrowser->get_Document(&spDisp);
	assert( SUCCEEDED( hr ) );

	IHTMLDocument2 *spDoc2 = NULL;
	hr = spDisp->QueryInterface( IID_IHTMLDocument2, ( void** )&spDoc2 );
	assert( SUCCEEDED( hr ) );

	IHTMLDocument3 *spDoc3 = NULL;
	hr = spDisp->QueryInterface( IID_IHTMLDocument3, ( void** )&spDoc3 );
	assert( SUCCEEDED( hr ) );

	long x2=0, x3=0;
	long y2=0, y3=0;

	if (spDoc2)
	{
		IHTMLElement* pElement = NULL;

		hr = spDoc2->get_body( &pElement );
		assert( SUCCEEDED( hr ) );

		if (pElement)
		{
			IHTMLElement2* pElement2 = NULL;
			hr = pElement->QueryInterface( IID_IHTMLElement2, ( void** )&pElement2 );
			assert( SUCCEEDED( hr ) );

			if(pElement2){
				pElement2->get_scrollLeft(&x2);
				pElement2->get_scrollTop(&y2);

				pElement2->Release();
			}
			pElement->Release();
		}

		spDoc2->Release();
	}

	if (spDoc3)
	{
		IHTMLElement* pElement = NULL;
		hr = spDoc3->get_documentElement( &pElement );
		assert( SUCCEEDED( hr ) );
		if (pElement)
		{
			IHTMLElement2* pElement2 = NULL;
			hr = pElement->QueryInterface( IID_IHTMLElement2, ( void** )&pElement2 );
			assert( SUCCEEDED( hr ) );

			pElement2->get_scrollLeft(&x3);
			pElement2->get_scrollTop(&y3);

			pElement2->Release();
			pElement->Release();

			spDoc3->Release();
		}

	}

	if(x2==0 && y2==0){
		p.x = x3;
		p.y = y3;
	}else{
		p.x = x2;
		p.y = y2;
	}

	spDisp->Release();

}

HBITMAP CWebHost::CaptureFrameFromWeb(int32 w,int32 h){
	HBITMAP hBmpCapture = NULL;


	IHTMLElement2* pElement2 = FindGameFrame();
	if (pElement2==NULL)
	{
		return NULL;
	};

	HDC hDC = GetDC(GetHandle());

	HDC hMemDC = CreateCompatibleDC(hDC); 	

	hBmpCapture = CreateCompatibleBitmap(hDC,w,h); 
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC,hBmpCapture);

	RECT rc;
	rc.left = 0;
	rc.top  = 0;
	rc.right  = w; 
	rc.bottom = h;

	//::OleDraw(pElement2,DVASPECT_CONTENT,hMemDC,&rc);

	IHTMLElementRender *pRender =  NULL;  
	pElement2 ->QueryInterface(IID_IHTMLElementRender, (void **) &pRender);  
	if(pRender){
		pRender->DrawToDC(hMemDC); 
		pRender->Release();
	}
	SelectObject(hMemDC,hOldBmp);
	DeleteObject(hMemDC);
	ReleaseDC(GetHandle(),hDC);

	pElement2->Release();	

	return hBmpCapture;
}


BOOL CWebHost::SubclassWebWnd(){
	UnsubclassWindow();
	HWND WebServer = ::FindWindowEx(m_WebView.m_IEFrame,NULL,_T("Shell DocObject View"),NULL);
	WebServer = ::GetWindow(WebServer,GW_CHILD);
	if( WebServer != GetHandle() && GetHandle() != NULL )
	{
		UnsubclassWindow();
		return FALSE;
	}
	BOOL ret = SubclassWindow( WebServer,FALSE ); 
	return ret;
}

LRESULT CWebHost::SendParentMessage(UINT Msg,int64 wParam, int64 lParam){
	HWND parent = m_MainFrame->GetHwnd();
	if(parent==NULL)return 0;
	SpaceRectionMsg SRM;
	
	SRM.Msg = Msg;
	SRM.wParam = wParam;
	SRM.lParam = lParam;
	SRM.ChildAffected = NULL;
	SRM.WinSpace = NULL;
	return ::SendMessage(parent,WM_PARENTRECTION,(WPARAM)&SRM,0);
};

BOOL CWebHost::ProcessWindowMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult ){
	if(uMsg == WM_MOUSEMOVE){
		SendParentMessage(CM_MOUSE_MOVE,wParam,lParam);
		return TRUE;
	}else if (uMsg == WM_SIZE)
	{
		BOOL ret =  CSubclassWnd::ProcessWindowMessage( uMsg, wParam, lParam, lResult );

		if (m_WebView.m_BkgImag)
		{
			StartSelectObject();
		}
		
		return ret;
	}
	return CSubclassWnd::ProcessWindowMessage( uMsg, wParam, lParam, lResult );

};

