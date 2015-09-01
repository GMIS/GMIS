// OptionView.cpp: implementation of the COptionView class.
//
//////////////////////////////////////////////////////////////////////

#pragma  warning (disable:4786)

#include "OptionView.h"

void CFooterBar::ControlBnt::Draw(HDC hDC,ePipeline* Pipe /*=NULL*/){
	RECT rc = GetArea();
	HFONT GuiFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	
	HFONT OldFont = (HFONT)::SelectObject(hDC, GuiFont );	
	DWORD Format = DT_CENTER|DT_VCENTER|DT_SINGLELINE;	
	
	COLORREF crBorder = RGB(192,192,192);
	COLORREF crFkg = RGB(0,0,0);
	COLORREF crBkg = RGB(192,192,192);
	
	//画文字边框			
	//	DrawEdge(hDC,m_Area,crBorder);
    DeflateRect(&rc,1,1,1,1);
	COLORREF Oldcr; 
	if(m_State & SPACE_DISABLE){
		DrawEdge(hDC,&rc,crBorder);
		Oldcr = ::SetTextColor(hDC,crBkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else if(m_State & SPACE_SELECTED){
		DrawEdge(hDC,&rc,crBorder);
		//FillRect(hDC,rc,crFkg);
		Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else if(m_State&SPACE_FOCUSED){
		FillRect(hDC,rc,crBkg);
		DrawEdge(hDC,&rc,BDR_RAISEDOUTER|BDR_RAISEDINNER,BF_BOTTOMRIGHT|BF_TOPLEFT);
		
		Oldcr = ::SetTextColor(hDC,crFkg);
		rc.left-=1;
		rc.top -=1;
		
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);
	}
	else {
		FillRect(hDC,rc,crBkg);
		DrawEdge(hDC,&rc,BDR_RAISEDOUTER|BDR_RAISEDINNER,BF_BOTTOMRIGHT|BF_TOPLEFT);
		COLORREF Oldcr = ::SetTextColor(hDC,crFkg);
		::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,Format);	
		::SetTextColor(hDC,Oldcr);	
	}
	
    ::SelectObject(hDC,OldFont ); 
}


CFooterBar::CFooterBar():
m_OKBnt(BNT_OK,_T("OK")),
m_CancleBnt(BNT_CANCLE,_T("Cancle"))
{
	m_crWinBorder  = RGB(128,128,128);
    m_crViewBkg    = RGB(245,245,243);
	PushChild(&m_OKBnt);
	PushChild(&m_CancleBnt);
}

CFooterBar::~CFooterBar()
{
	m_ChildList.clear();
}

void CFooterBar::Layout(bool Redraw /*= true*/){
    RECT rc;
    GetClientRect(GetHwnd(),&rc);

	int32 w = RectWidth(rc)/2;
	if (w==0)
	{
		return;
	}
	
	m_OKBnt.SetArea(rc.left+w-100,rc.top+1,rc.left+w-30,rc.bottom-1);
	m_CancleBnt.SetArea(rc.left+w+30,rc.top+1,rc.left+w+100,rc.bottom-1);

}

LRESULT CFooterBar::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if(message == WM_ERASEBKGND)
		return TRUE;
	else if(message == WM_PAINT)
		return OnPaint();
	else if(message == WM_LBUTTONDOWN){
		return OnLButtonDown(wParam,lParam);
	}else if (message == WM_LBUTTONUP)
	{
		return OnLButtonUp(wParam,lParam);
	}else if (message == WM_SIZE){
		Layout();
		return 0;
	}
	return CWinSpace2::Reaction(message,wParam,lParam);
}
	
LRESULT CFooterBar::OnPaint(){
	
	RECT rcViewport; 
	GetClientRect(m_hWnd,&rcViewport);
	
	PAINTSTRUCT ps;				
	HDC hdc = BeginPaint(m_hWnd, &ps);
	
	if(rcViewport.right==rcViewport.left || rcViewport.top==rcViewport.bottom){
		::EndPaint(m_hWnd, &ps);	
		return 0;
	}
	
	HDC DCMem = ::CreateCompatibleDC(hdc);
	HBITMAP bmpCanvas=::CreateCompatibleBitmap(hdc, RectWidth(rcViewport),RectHeight(rcViewport));
	assert(bmpCanvas);
	HBITMAP OldBitmap = (HBITMAP)::SelectObject(DCMem, bmpCanvas );
	
	if(m_State & SPACE_SHOWBKG){
		FillRect(DCMem,rcViewport,m_crViewBkg);
	}
	
	Draw(DCMem);
			
	if(m_State & SPACE_SHOWWINBORDER){
		//	DrawEdge(DCMem,rcViewport,m_crWinBorder);// Border
		::DrawEdge(DCMem,&rcViewport,BDR_SUNKENOUTER,BF_RECT);
	}
	
	::BitBlt(hdc, 0, 0, RectWidth(rcViewport), RectHeight(rcViewport), DCMem, 0, 0, SRCCOPY );
	::SelectObject(DCMem, OldBitmap );
	::DeleteObject(bmpCanvas);
	::DeleteDC(DCMem);
	::EndPaint(m_hWnd, &ps);
	return 0;
}
	
LRESULT CFooterBar::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	
	::SetCapture(m_hWnd);
	
	m_SpaceSelected = Hit(point);
	if(m_SpaceSelected){
		if (m_SpaceSelected->m_Alias == 0 || m_SpaceSelected->m_State & SPACE_DISABLE)
		{
			m_SpaceSelected = NULL;
			return 0;
		}
		
		m_SpaceSelected->m_State |= SPACE_SELECTED;
		Invalidate();
	}
	return 0;	
};
	
LRESULT CFooterBar::OnLButtonUp(WPARAM wParam, LPARAM lParam){
	
	POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	::ReleaseCapture();
	
	CVSpace2* v = Hit(point);
	if(v && v==m_SpaceSelected){
				
		v->m_State &=~SPACE_SELECTED;
		Invalidate();
		
		SendParentMessage(v->m_Alias,0,0,NULL);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionView::COptionView()
{
	m_crViewBkg = RGB(255,255,255);
/*
	BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
	TextItem*     Text     = new TextItem(ID_TITLETEXT,_T("Option"));	
	CloseboxItem* Closebox = new CloseboxItem(ID_CLOSE);
	
	if(TitleSpace){
		TitleSpace->DeleteAll();
		TitleSpace->PushChild(Text);
		TitleSpace->PushChild(Closebox);
	}
*/

	CWSFormView::CHeadline* Headline = new CWSFormView::CHeadline(0,&m_OptionView,_T("选择1"));
	Headline->SetArea(0,0,300,24);
    m_OptionView.PushChild(Headline);

	CWSFormView::CEditText* EditBox = new CWSFormView::CEditText(1,&m_OptionView,_T("编辑"),FALSE);
	EditBox->SetArea(0,0,300,24);
	EditBox->SetControlRect(50,0,300,24);
	m_OptionView.PushChild(EditBox);

	CWSFormView::CCheckBox* CheckBox = new CWSFormView::CCheckBox(2,&m_OptionView,_T("CheckBox"),TRUE);
	CheckBox->SetArea(0,0,300,24);
	CheckBox->SetControlRect(120,0,136,24);
	m_OptionView.PushChild(CheckBox);

	CWSFormView::CRadioSet* Radio = new CWSFormView::CRadioSet(3,&m_OptionView);
	Radio->PushRadioValue(_T("选择1"),FALSE);
	Radio->PushRadioValue(_T("选择2"),TRUE);
	Radio->PushRadioValue(_T("选择3"),FALSE);
	
	Radio->SetArea(0,0,300,24*3);
	Radio->SetControlRect(120,0,136,24*3);
	m_OptionView.PushChild(Radio);
	
	CWSFormView::CComboList* ComboList = new CWSFormView::CComboList(2,&m_OptionView,_T("ComboLIst"),_T("没有选择"));
	ComboList->SetArea(0,0,300,24);
	ComboList->SetControlRect(100,0,300,24);
	m_OptionView.PushChild(ComboList);
}

COptionView::~COptionView()
{
	
}

void COptionView::Layout(bool Redraw /* = true */)
{
	CWSFrameView::Layout(FALSE);
	
	::MoveWindow(m_OptionView.GetHwnd(),m_rcClient.left,m_rcClient.top,RectWidth(m_rcClient),RectHeight(m_rcClient)-24,TRUE);
	::MoveWindow(m_FooterBar.GetHwnd(),m_rcClient.left,m_rcClient.bottom-24,RectWidth(m_rcClient),24,TRUE);
}

LRESULT  COptionView::ParentReaction(SpaceRectionMsg* SRM){
	if (SRM->Msg == BNT_OK)
	{
		return SendMessage(GetHwnd(),WM_CLOSE,0,0);

	}else if (SRM->Msg == BNT_CANCLE)
	{
		return SendMessage(GetHwnd(),WM_CLOSE,0,0);
	}
	return 0;
}

LRESULT COptionView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if (message == WM_CREATE)
	{
		return OnCreate(wParam,lParam);
	}
    else return CWSFrameView::Reaction(message,wParam,lParam);
}

LRESULT COptionView::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT rc = {0};
	
	if(!m_OptionView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}
	
	if(!m_FooterBar.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}
	return 0;
}

