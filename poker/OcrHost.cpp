// OcrHost.cpp: implementation of the COcrHost class.
//
//////////////////////////////////////////////////////////////////////

#include "MainFrame.h"
#include "OcrHost.h"

HRESULT __stdcall MyCoCreateInstance(
									 LPCTSTR szDllName,
									 IN REFCLSID rclsid,
									 IUnknown* pUnkOuter,
									 IN REFIID riid,
									 OUT LPVOID FAR* ppv)
{
	HRESULT hr = REGDB_E_KEYMISSING;
	
	HMODULE hDll = ::LoadLibrary(szDllName);
	if (hDll == 0)
		return hr;
	
	typedef HRESULT (__stdcall *pDllGetClassObject)(IN REFCLSID rclsid, 
		IN REFIID riid, OUT LPVOID FAR* ppv);
	
	pDllGetClassObject GetClassObject = 
		(pDllGetClassObject)::GetProcAddress(hDll, "DllGetClassObject");
	if (GetClassObject == 0)
	{
		::FreeLibrary(hDll);
		return hr;
	}
	
	IClassFactory *pIFactory;
	
	hr = GetClassObject(rclsid, IID_IClassFactory, (LPVOID *)&pIFactory);
	
	if (!SUCCEEDED(hr))
		return hr;
	
	hr = pIFactory->CreateInstance(pUnkOuter, riid, ppv);
	pIFactory->Release();
	
	return hr;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COcrHost::COcrHost()
{

}

COcrHost::~COcrHost()
{

}

BOOL COcrHost::Init(){
	return TRUE;
};
	

BOOL COcrHost::UseOcr(tstring ImageFile,tstring& Output){
/*	
	BOOL bRet = TRUE;
	HRESULT hr = 0;
	csText.Empty();
	
	IUnknown *pVal = NULL;
	IDocument *IDobj = NULL;
	ILayout *ILayout = NULL;
	IImages *IImages = NULL;
	IImage *IImage = NULL;
	IWords *IWords = NULL;
	IWord *IWord = NULL;
	
	pVal = (IUnknown *) m_MIDOCtrl.GetDocument(); 
	
	if ( pVal != NULL )
	{
		//Already has image in it, Don't need to create again
		
		//Just get IDocument interface
		
		pVal->QueryInterface(IID_IDocument,(void**) &IDobj);
		if ( SUCCEEDED(hr) )
		{
			hr = IDobj->OCR(miLANG_SYSDEFAULT,1,1);
			
			if ( SUCCEEDED(hr) )
			{
				IDobj->get_Images(&IImages);
				long iImageCount=0;
				
				Images->get_Count(&iImageCount);
				for ( int img =0; img<iImageCount;img++)
				{
					IImages->get_Item(img,(IDispatch**)&IImage);
					IImage->get_Layout(&ILayout);
					
					long numWord=0;
					ILayout->get_NumWords(&numWord);
					ILayout->get_Words(&IWords);
					
					IWords->get_Count(&numWord);
					
					for ( long i=0; i<numWord;i++)
					{
						IWords->get_Item(i,(IDispatch**)&IWord);
						CString csTemp;
						BSTR result;
						IWord->get_Text(&result);
						char buf[256];
						sprintf(buf,"%S",result);
						csTemp.Format("%s",buf);
						
						csText += csTemp;
						csText +=" ";
					}
					
					//Release all objects
					
					IWord->Release();
					IWords->Release();
					ILayout->Release();
					IImage->Release();
				}
				IImages->Release();
				
			} else {
				bRet = FALSE;
			}
		} else {
			bRet = FALSE;
		}
		
		IDobj->Close(0);
		IDobj->Release();
		pVal->Release();
		
	} else {
		bRet = FALSE;
	}
	
   return bRet;
*/

	return TRUE;
}