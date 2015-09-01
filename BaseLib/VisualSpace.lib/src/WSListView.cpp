// ListViewport.cpp : implementation file
//

#include "WSListView.h"
#include <stdio.h>

namespace VISUALSPACE{


void CWSListView::CListItem::Draw(HDC hDC,ePipeline* Pipe){
	COLORREF crBorder    = RGB(128,128,128);
	COLORREF crFkg       = RGB(0,0,0); 
	COLORREF crBkg       = RGB(255,255,255); 
	COLORREF crSelectFkg = RGB(255,255,255); 
	COLORREF crSelectBkg = RGB(255,0,0);  
		
	//输出内容
	tstring& text = m_Text;
	RECT rc = GetArea();
	DeflateRect(&rc,4,2,4,2);

	if(m_State&SPACE_FOCUSED){
		//pDC->DrawFocusRect(&m_Area);
		FillRect(hDC,GetArea(),crFkg);
		COLORREF Oldcr = ::SetTextColor(hDC,crBkg);
		::DrawText(hDC,text.c_str(),text.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
			DT_NOPREFIX|DT_VCENTER);	
		::SetTextColor(hDC,Oldcr);
	}
    else if(m_State & SPACE_SELECTED){
		FillRect(hDC,GetArea(),crSelectBkg);
		COLORREF Oldcr = ::SetTextColor(hDC,crSelectFkg);
		::DrawText(hDC,text.c_str(),text.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
			DT_NOPREFIX|DT_VCENTER);		
		::SetTextColor(hDC,Oldcr);
	}
	else {
		FillRect(hDC,GetArea(),crBkg);
		COLORREF Oldcr =::SetTextColor(hDC,crFkg);
		::DrawText(hDC,text.c_str(),text.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
			DT_NOPREFIX|DT_VCENTER);	
		::SetTextColor(hDC,Oldcr);
	}

	//画文字边框			
	DrawEdge(hDC,GetArea(),crBorder);
		
}


/////////////////////////////////////////////////////////////////////////////
// WSListView

CWSListView::CWSListView()
{
	m_HeaderWidth = 30;
}

CWSListView::~CWSListView()
{
}


CWSListView::CListItem* CWSListView::AddItem(const TCHAR* text,bool Redraw){

	CListItem* Item = new CListItem(GetItemCount(),text); 
    Push(Item);	
	SIZE s = CalcuTextSize(Item->m_Text.c_str());    	
	Item->SetArea(0,0,s.cx+m_Padding,s.cy+m_Padding);
	if(Redraw) {
        Layout(true);
	}
	return Item;
}

void CWSListView::DrawItemHeader( HDC hDC, CVSpace2* Space){
   	 
	RECT rcHeader = Space->GetArea();
	rcHeader.right = rcHeader.left-4;
    rcHeader.left -= m_HeaderWidth;

	TCHAR buf[100];
	_stprintf(buf,_T("%03I64ld"),Space->m_Alias);
	
	int len = lstrlen(buf);
//	RECT rc;
//	::SetRect(&rc,0,rcHeader.top,rcHeader.right-2,rcHeader.bottom);
	::DrawText(hDC,buf,len,&rcHeader,DT_RIGHT|DT_VCENTER|DT_NOPREFIX);
}

};// namespace VISUALSPACE