// OcrReusltView.cpp: implementation of the COcrResultView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4786)

#include "MainFrame.h"
#include "OcrResultView.h"
#include "FORMAT.H"

BOOL COcrEdit::ProcessWindowMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult )
{
/*
	if(uMsg == WM_KILLFOCUS){
	    SendParentMessage(CM_KILLFOCUS,0,0);
	}
*/
	return CEditWin::ProcessWindowMessage( uMsg, wParam, lParam, lResult );
}

COcrItem::COcrItem()
:m_hBMP(NULL),m_Crc32(0)
{

}
COcrItem::COcrItem(int64 ID,tstring Name,HBITMAP hBMP,uint32 Crc32,tstring& OcrText)
:CVSpace2(ID),m_hBMP(hBMP),m_Crc32(Crc32),m_OcrText(OcrText),m_Name(Name)
{

}
COcrItem::~COcrItem(){
	if (m_hBMP)
	{
		::DeleteObject(m_hBMP);
	}
}

void COcrItem::Draw(HDC hDC, ePipeline* Pipe/* =NULL */){
	
	RECT rc = GetArea();
	
	COLORREF crText=RGB(0x17,0x35,0x7b);

	if(m_State & SPACE_SELECTED){
        DrawEdge(hDC,&rc,RGB(128,128,128));
	}else if(m_State & SPACE_FOCUSED){
		DrawEdge(hDC,&rc,RGB(128,128,128));
	}

    InflateRect(&rc,-8,-4);
	
	COLORREF Oldcr = ::SetTextColor(hDC,crText);
	
	rc.bottom = rc.top+20;
	
	//name
	::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,DT_LEFT|DT_VCENTER);	

	rc.top = rc.bottom;
	rc.bottom = rc.top + 20;

	if (m_OcrText.size())
	{
		tstring ocrStr = tformat(_T("OCR: %s"),m_OcrText.c_str());
		::DrawText(hDC,ocrStr.c_str(),ocrStr.size(),&rc,DT_LEFT|DT_VCENTER);	
	}else {
		tstring ocrStr = _T("OCR: NULL");
		::DrawText(hDC,ocrStr.c_str(),ocrStr.size(),&rc,DT_LEFT|DT_VCENTER);	
	}
	
	rc.top = rc.bottom;
	rc.bottom = rc.top+45;
	
	//图像
	if(m_hBMP){			
		HDC MemDC=CreateCompatibleDC(hDC); 
		HBITMAP Oldbitmap = (HBITMAP)SelectObject(MemDC,m_hBMP); 
		
		BitBlt(hDC, 
			rc.left,rc.top,
			RectWidth(rc),RectHeight(rc),
			MemDC, 
			0,0,
			SRCCOPY);		
		::SelectObject(MemDC,Oldbitmap);
		::DeleteDC(MemDC); 
	}
	
	::SetTextColor(hDC,Oldcr);	
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COcrResultView::COcrResultView(CMainFrame* Frame)
:m_MainFrame(Frame)
{
	 m_crViewBkg = RGB(255,255,255);
}

COcrResultView::~COcrResultView()
{

}

void COcrResultView::AddItem(tstring Name,HBITMAP hBMP,uint32 Crc32,tstring OcrStr){
	int64 ID = m_ChildList.size()==0? 0:m_ChildList.back()->m_Alias+1;

	COcrItem* OcrItem = new COcrItem(ID,Name,hBMP,Crc32,OcrStr);
	OcrItem->SetAreaSize(100,70);
	if(GetHwnd()){
		SendChildMessage(GetHwnd(),CM_ADDITEM,(int64)OcrItem,0);
	}else{
		PushChild(OcrItem);
	}

}
void  COcrResultView::Layout(bool Redraw){
	CWSListView::Layout(false);
	
	if (m_SpaceSelected)
	{
		RECT rc = m_SpaceSelected->GetArea();
		rc.top = rc.bottom-24;
        rc.bottom -=1;
        rc.left  +=1;
		rc.right -=1;
		::SetWindowPos(m_OcrEdit.GetHandle(),NULL,rc.left,rc.top,RectWidth(rc),RectHeight(rc),SWP_SHOWWINDOW);
		::SetFocus(m_OcrEdit.GetHandle());
	}
	if(Redraw)Invalidate();
}

LRESULT COcrResultView::ParentReaction(SpaceRectionMsg* SRM){
	if (SRM->Msg == CM_KILLFOCUS)
	{
		tstring OcrText = m_OcrEdit.GetEditText(true);
		if (OcrText.size() && m_SpaceSelected)
		{
			m_SpaceSelected->m_AreaBottom-=20;
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			
			tstring OcrText = m_OcrEdit.GetEditText(true);
			if (OcrText.size())
			{
				COcrItem* Item = (COcrItem*)m_SpaceSelected;
				Item->m_OcrText = OcrText;
				SendParentMessage(CM_OCRTEXT_SET,NULL,NULL,Item);
				
			}
			m_SpaceSelected = NULL;
			Layout();
		}
	}
	return 0;
}
	
LRESULT COcrResultView::ChildReaction(SpaceRectionMsg* SRM){
	if(SRM->Msg == CM_ADDITEM){
		if (m_ChildList.size()>200)
		{
			DeleteAll();
		}
        COcrItem* Item = (COcrItem*)SRM->wParam;
		PushChild(Item);
		Layout();
		EnsureVisible(Item,FALSE);
	}
	return 0;
}

LRESULT COcrResultView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if (message == WM_CREATE)
	{
		return OnCreate(wParam,lParam);
	}else if (message == WM_LBUTTONDOWN)
	{
		return OnLButtonDown(wParam,lParam);
	}else if (message == WM_LBUTTONDBLCLK)
	{
		return OnLButtonDBCLK(wParam,lParam);
	}
	else {
		return CWSListView::Reaction(message,wParam,lParam);
	}

}

LRESULT COcrResultView::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT rcEdit;
	::GetClientRect(m_hWnd,&rcEdit);
	// Create the edit control
	DWORD style = WS_VISIBLE | WS_CHILD |ES_LEFT | ES_WANTRETURN|WS_EX_STATICEDGE|WS_BORDER;
	
	HWND hEdit =  CreateWindowEx(
		0,//WS_EX_STATICEDGE,      // extended window style
		_T("EDIT"),  // pointer to registered class name
		NULL, // pointer to window name
		style,        // window style
		rcEdit.left,                // horizontal position of window
		rcEdit.top,                // vertical position of window
		RectWidth(rcEdit),           // window width
		RectHeight(rcEdit),          // window height
		m_hWnd,      // handle to parent or owner window
		(HMENU)100,          // handle to menu, or child-window identifier
		GetHinstance(),  // handle to application instance
		NULL       // pointer to window-creation data
		);
	assert(hEdit != NULL);
	if(hEdit==NULL)return -1;	

	m_OcrEdit.SetEditCtrl(hEdit);
	return 0;
};


LRESULT COcrResultView::OnLButtonDBCLK(WPARAM wParam, LPARAM lParam){
	LRESULT ret = CWSScrollView::OnLButtonDown(wParam,lParam);
	
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	if(m_ScrollSelected){
		return 0;
	}

	CVSpace2* NewSelect = Hit(point);
	
	if(NewSelect ==NULL)return 0; 
	
	if(NewSelect == m_SpaceSelected){				
		tstring OcrText = m_OcrEdit.GetEditText(true);		
        SetWindowPos(m_OcrEdit.GetHandle(),NULL,0,0,0,0,SWP_HIDEWINDOW);
		m_SpaceSelected = NULL;
	}		
	
	CVSpace2* v=RemoveChild(NewSelect); //注意：没有Delete 
	assert(v = NewSelect);
	delete v;     
    Layout();
	return 0;
}
LRESULT COcrResultView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	LRESULT ret = CWSScrollView::OnLButtonDown(wParam,lParam);
	
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	if(m_ScrollSelected){
		if(m_SpaceSelected){ 
			m_SpaceSelected->m_AreaBottom-=20;
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			//m_Toolbar.m_Owner = NULL;
			m_SpaceSelected = NULL;
		}
		return 0;
	}
	
	CVSpace2* NewSelect = Hit(point);
	
	if(NewSelect ==NULL)return 0; 
	
	if(NewSelect == m_SpaceSelected){		
		
		//点击发生在item?
		RECT rc = m_SpaceSelected->GetArea();
		rc.bottom-=20; //排除toolbar再次点击在Item上则取消选择
		if(::PtInRect(&rc,point)){
			m_SpaceSelected->SetArea(rc);
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;

			tstring OcrText = m_OcrEdit.GetEditText(true);
			if (OcrText.size())
			{
				COcrItem* Item = (COcrItem*)m_SpaceSelected;
				Item->m_OcrText = OcrText;
				SendParentMessage(CM_OCRTEXT_SET,NULL,NULL,Item);
			}

            SetWindowPos(m_OcrEdit.GetHandle(),NULL,0,0,0,0,SWP_HIDEWINDOW);
			m_SpaceSelected = NULL;

			Layout();
			return 0;
		}

	}else{// NewSelect != m_SpaceSelected	
		
		//取消旧的选择
		if(m_SpaceSelected){ 
			m_SpaceSelected->m_AreaBottom-=20;
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;

			tstring OcrText = m_OcrEdit.GetEditText(true);
			if (OcrText.size())
			{
				COcrItem* Item = (COcrItem*)m_SpaceSelected;
				Item->m_OcrText = OcrText;
				SendParentMessage(CM_OCRTEXT_SET,NULL,NULL,Item);		
			}
			m_SpaceSelected = NULL;
		}
		//设置新选择
		m_SpaceSelected = NewSelect;
		m_SpaceSelected->m_State |= SPACE_SELECTED;
		m_SpaceSelected->m_AreaBottom += 20; //增加高度放置toolbar
		Layout();		
	}
	return 0;
}

  
