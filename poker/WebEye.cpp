// WebEye.cpp: implementation of the CWebEye class.
//
//////////////////////////////////////////////////////////////////////

#include "WebEye.h"


#define   LWA_COLORKEY                         0x00000001 
#define   LWA_ALPHA                            0x00000002 


#define   ULW_COLORKEY                         0x00000001 
#define   ULW_ALPHA                            0x00000002 
#define   ULW_OPAQUE                           0x00000004 


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWebEye::CWebEye()
{
	m_crViewBkg = RGB(255,235,255);
	m_bDrag=FALSE;

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

CWebEye::~CWebEye()
{
	if(m_HalfBrush){
		::DeleteObject(m_HalfBrush);	
	}
}


void  CWebEye::Draw(HDC hDC,ePipeline* Pipe /*= NULL*/){
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



LRESULT CWebEye::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){

	case WM_MOUSEMOVE:
		return OnMouseMove(wParam,lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam,lParam);
	default:
		return CWinSpace2::Reaction(message,wParam,lParam);
	}
}

LRESULT CWebEye::OnMouseMove(WPARAM wParam, LPARAM lParam){
	m_P2 = MAKEPOINTS(lParam);
	Invalidate();
	return 0;
};

LRESULT CWebEye::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	::SetCapture(GetHwnd());
	POINTS p = MAKEPOINTS(lParam);
    m_P1 = p;
	m_bDrag = TRUE;
	Invalidate();
	return 0;
};

LRESULT CWebEye::OnLButtonUp(WPARAM wParam, LPARAM lParam){
	::ReleaseCapture();
	m_P2 = MAKEPOINTS(lParam);
	m_bDrag = FALSE;
	Invalidate();
	return 0;
};