// Tesseract.h: interface for the Tesseract class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESSERACT_H__17A2919F_7A0E_4881_B01B_22D3E3EEE374__INCLUDED_)
#define AFX_TESSERACT_H__17A2919F_7A0E_4881_B01B_22D3E3EEE374__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "typedef.h"
#include "baseapi.h"
#include <BaseTsd.h>
#include "allheaders.h"
#include "strngs.h"
#include "BitmapEx.h"

#include <string>

#ifdef _DEBUG
#pragma comment(lib,"libtesseract_static-debug")
#pragma comment(lib,"liblept")
#else 
#pragma comment(lib,"libtesseract_static.lib")
#pragma comment(lib,"liblept")
#endif


PIX * GetPixFromeStreamBmp(unsigned char* ImageData,int DataSize);

using namespace tesseract;

class CTesseract : public TessBaseAPI
{
protected:
	bool     m_InitOK;
public:
    CTesseract();
	~CTesseract();
	bool IsValid(){ return m_InitOK;};
    bool SetLang(const char* LangDir, const char* LangName);
public:

	//×¢Òâ£¬·µ»ØµÄÊÇUTF8±àÂë×Ö·û´®
	std::string OCR(tstring  FileName);
	std::string OCR(PIX* pix);
	std::string OCR(CBitmapEx& Image);
};



#endif // !defined(AFX_TESSERACT_H__17A2919F_7A0E_4881_B01B_22D3E3EEE374__INCLUDED_)
