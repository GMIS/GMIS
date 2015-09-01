// WSOpitonView.cpp: implementation of the WSOpitonView class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning (disable: 4786)

#include "WSFormView.h"

CInterControl::CInterControl(int64 ID,CVSpace2* Parent,int32 Type)
:CVSpace2(ID,Parent),m_ControlType(Type),m_hControl(NULL)
{
	m_rcControl.left = 0;
	m_rcControl.top  = 0;
	m_rcControl.right = 0;
	m_rcControl.bottom = 0;
}
CInterControl::~CInterControl(){

};

void CInterControl::SetControlRect(int32 left,int32 top,int32 right,int32 bottom){
	m_rcControl.left = left;
	m_rcControl.top  = top;
	m_rcControl.right = right;
	m_rcControl.bottom = bottom;
}

void CInterControl::Layout(bool Redraw ){
	if(m_hControl){
		RECT rc = m_rcControl;
		rc.left += m_AreaLeft;
		rc.top  += m_AreaTop;
		rc.right +=m_AreaLeft;
		rc.bottom +=m_AreaTop;
		::MoveWindow(m_hControl,rc.left,rc.top,RectWidth(rc),RectHeight(rc),FALSE);
	}
}

CWSFormView::CHeadline::CHeadline(int64 ID,CVSpace2* Parent,tstring Title)
:CInterControl(ID,Parent,CONTROL_NULL),m_Title(Title)
{

};

CWSFormView::CHeadline::~CHeadline(){

};

void CWSFormView::CHeadline::Draw(HDC hDC,ePipeline* Pipe){
	RECT rc = GetArea();

	COLORREF crText = RGB(0,0,0);
	COLORREF Oldcr = ::SetTextColor(hDC,crText);
	
	rc.left+=2;
	::DrawText(hDC,m_Title.c_str(),m_Title.size(),&rc,DT_LEFT);	
	
	::SetTextColor(hDC,Oldcr);		

	rc.left-=2;
	rc.top = rc.bottom-2;
	FillRect(hDC,rc,RGB(0,0,0));
};

CWSFormView::CEditText::CEditText(int64 ID,CVSpace2* Parent,tstring Title,BOOL bNumber)
:CInterControl(ID,Parent,CONTROL_EDIT),m_Title(Title),m_bNumber(bNumber)
{

}

CWSFormView::CEditText::~CEditText(){

};	
			
void CWSFormView::CEditText::Draw(HDC hDC,ePipeline* Pipe){

	RECT rc = GetArea();
	COLORREF crText = RGB(0,0,0);
	COLORREF Oldcr = ::SetTextColor(hDC,crText);

	rc.right = m_rcControl.left-2;
	::DrawText(hDC,m_Title.c_str(),m_Title.size(),&rc,DT_LEFT|DT_VCENTER|DT_SINGLELINE);	

	if (m_hControl == NULL)
	{
		rc = m_rcControl;
		rc.left += m_AreaLeft;
		rc.right += m_AreaLeft;
		rc.top  += m_AreaTop;
		rc.bottom += m_AreaTop;

		if (m_State & SPACE_FOCUSED)
		{
			DrawEdge(hDC,rc,RGB(128,128,128));
		}else{
			DrawEdge(hDC,rc,RGB(198,198,198));
		}

		DeflateRect(&rc,2,1,1,1);
	    ::DrawText(hDC,m_EditText.c_str(),m_EditText.size(),&rc,DT_LEFT|DT_VCENTER|DT_SINGLELINE);	
	}

	::SetTextColor(hDC,Oldcr);		
};

void CWSFormView::CEditText::OnLButtonDown(POINT& point){
	if(m_hControl == NULL){
		CWinSpace2* Parent = (CWinSpace2*)m_Parent;

		DWORD style = WS_VISIBLE | WS_CHILD |ES_LEFT |ES_WANTRETURN|WS_EX_STATICEDGE|WS_BORDER|ES_AUTOHSCROLL;
		if (m_bNumber)
		{
			style |= ES_NUMBER;
		}

		RECT rcEdit = {0};
		HWND hEdit =  CreateWindowEx(
			0,//WS_EX_STATICEDGE,      // extended window style
			_T("EDIT"),  // pointer to registered class name
			NULL, // pointer to window name
			style,        // window style
			rcEdit.left,                // horizontal position of window
			rcEdit.top,                // vertical position of window
			RectWidth(rcEdit),           // window width
			RectHeight(rcEdit),          // window height
			Parent->GetHwnd(),      // handle to parent or owner window
			(HMENU)0,          // handle to menu, or child-window identifier
			GetHinstance(),  // handle to application instance
			NULL       // pointer to window-creation data
			);
		assert(hEdit != NULL);
		if(hEdit==NULL)return;

		m_hControl = hEdit;
		::SetWindowText(m_hControl,m_EditText.c_str());
		::SetFocus(hEdit);
		Layout();
	}else  if (m_hControl)
	{
		if(::GetWindowTextLength(m_hControl))return ;

		TCHAR buf[255];
		::GetWindowText(m_hControl,buf,255);
		m_EditText = buf;
		::DestroyWindow(m_hControl);
		m_hControl = NULL;
	};
}

void CWSFormView::CEditText::OnLoseFocus(){
	if (m_hControl)
	{
		TCHAR buf[255];
		::GetWindowText(m_hControl,buf,255);
		m_EditText = buf;
		::DestroyWindow(m_hControl);
		m_hControl = NULL;

	}
}

BOOL CWSFormView::CEditText::FromPipe(ePipeline& Pipe){
	if (Pipe.Size()==0)
	{
		return FALSE;
	}
	eElectron e;
	Pipe.Pop(&e);

	if (e.EnergyType() !=  TYPE_PIPELINE)
	{
		return FALSE;
	}

	ePipeline* Value = (ePipeline*)e.Value();
	if (Value->GetID() != m_Alias)
	{
		return FALSE;
	}

	eElectron e1;
	Value->Pop(&e1);
	if (e1.EnergyType() != TYPE_STRING)
	{
		return FALSE;
	}

	m_EditText = *(tstring*)e1.Value();
	return TRUE;
};

void CWSFormView::CEditText::ToPipe(ePipeline& Pipe){
	OnLoseFocus();
	ePipeline Value(m_Alias);
	Value.PushString(m_EditText);
    Pipe.PushPipe(Value);
};

//////////////////////////////////////////////////////////////////////////

CWSFormView::CCheckBox::CCheckBox(int64 ID,CVSpace2* Parent,tstring Title,BOOL bChecked)
:CInterControl(ID,Parent,CONTROL_CHECKBOX),m_Title(Title),m_bChecked(bChecked)
{
	
}
CWSFormView::CCheckBox::~CCheckBox(){

}				
void CWSFormView::CCheckBox::Draw(HDC hDC,ePipeline* Pipe){
	RECT rc = GetArea();
	COLORREF crText = RGB(0,0,0);
	COLORREF Oldcr = ::SetTextColor(hDC,crText);


	rc.right = m_rcControl.left+m_AreaLeft-4;
	::DrawText(hDC,m_Title.c_str(),m_Title.size(),&rc,DT_LEFT|DT_VCENTER|DT_SINGLELINE);	

	rc = m_rcControl;
	rc.left   += m_AreaLeft;
	rc.right  += m_AreaLeft;
	rc.top    += m_AreaTop;
    rc.bottom += m_AreaTop;
	

	if (m_bChecked)
	{
		::DrawFrameControl(hDC,&rc, DFC_BUTTON, DFCS_BUTTONCHECK|DFCS_CHECKED); 
	}else{
		::DrawFrameControl(hDC,&rc, DFC_BUTTON, DFCS_BUTTONCHECK); 
	}	
	::SetTextColor(hDC,Oldcr);			
}

void CWSFormView::CCheckBox::OnLButtonDown(POINT& point){
	RECT rc;
	rc = m_rcControl;
	rc.left += m_AreaLeft;
	rc.right += m_AreaLeft;
	rc.top  += m_AreaTop;
    rc.bottom += m_AreaTop;
	if (::PtInRect(&rc,point))
	{
		m_bChecked = m_bChecked? FALSE:TRUE;
	}
}

BOOL CWSFormView::CCheckBox::FromPipe(ePipeline& Pipe){
	if (Pipe.Size()==0)
	{
		return FALSE;
	}
	eElectron e;
	Pipe.Pop(&e);
	
	if (e.EnergyType() !=  TYPE_PIPELINE)
	{
		return FALSE;
	}
	
	ePipeline* Value = (ePipeline*)e.Value();
	if (Value->GetID() != m_Alias)
	{
		return FALSE;
	}
	
	eElectron e1;
	Value->Pop(&e1);
	if (e1.EnergyType() != TYPE_INT)
	{
		return FALSE;
	}

	m_bChecked = *(int32*)e1.Value();
	return TRUE;
};
void CWSFormView::CCheckBox::ToPipe(ePipeline& Pipe){

	ePipeline Value(m_Alias);
	Value.PushInt(m_bChecked);
	Pipe.PushPipe(Value);
};

//////////////////////////////////////////////////////////////////////////
CWSFormView::CRadioSet::CRadioSet(int64 ID,CVSpace2* Parent)
:CInterControl(ID,Parent,CONTROL_RADIO)
{

}

CWSFormView::CRadioSet::~CRadioSet(){

}

void CWSFormView::CRadioSet::PushRadioValue(tstring Name,BOOL Value){
	m_RadioSet[Name] = Value;
}
void CWSFormView::CRadioSet::Draw(HDC hDC,ePipeline* Pipe){
	COLORREF crText = RGB(0,0,0);
	COLORREF Oldcr = ::SetTextColor(hDC,crText);
	int h = 20;

	RECT rc1 = GetArea();
	rc1.right = rc1.left + m_rcControl.left-5;
    rc1.bottom = rc1.top + 20;

	RECT rc2 = m_rcControl;
	rc2.left  += m_AreaLeft;
    rc2.right += m_AreaLeft;
	rc2.top   += m_AreaTop;
	rc2.bottom = rc2.top +20;

    map<tstring,BOOL>::iterator it = m_RadioSet.begin();
	while(it != m_RadioSet.end()){
		tstring Name = it->first;
		::DrawText(hDC,Name.c_str(),Name.size(),&rc1,DT_LEFT|DT_VCENTER|DT_SINGLELINE);	
	    BOOL b = it->second;
		if (b)
		{
			::DrawFrameControl(hDC,&rc2, DFC_BUTTON, DFCS_BUTTONRADIO|DFCS_CHECKED); 
		}else{
			::DrawFrameControl(hDC,&rc2, DFC_BUTTON, DFCS_BUTTONRADIO); 
		}	
		rc1.top += 20;
		rc1.bottom += 20;
        rc2.top += 20;
		rc2.bottom += 20;
		it++;
	}	
	::SetTextColor(hDC,Oldcr);			
}
	
void CWSFormView::CRadioSet::OnLButtonDown(POINT& point){

	RECT rc2 = m_rcControl;
	rc2.left  += m_AreaLeft;
    rc2.right += m_AreaLeft;
	rc2.top   += m_AreaTop;
    rc2.bottom += m_AreaTop;

	if (!::PtInRect(&rc2,point))
	{
		return;
	}

	rc2.bottom = rc2.top +20;
    map<tstring,BOOL>::iterator it = m_RadioSet.begin();
	BOOL find=FALSE;
	tstring PreSelectName;
	while(it != m_RadioSet.end()){
		if (it->second)
		{
			PreSelectName = it->first;
		}
		if (::PtInRect(&rc2,point))
		{
			it->second = TRUE;
			find = TRUE;
		}else {
			it->second = FALSE;
		}
        rc2.top += 20;
		rc2.bottom += 20;
		it ++;
	}		

	//因为当点击到空白出，上述操作会让整个选择=FALSE,这里恢复为之前的选择
	if(!find && PreSelectName.size()){
		m_RadioSet[PreSelectName]=TRUE;
	}
}

BOOL CWSFormView::CRadioSet::FromPipe(ePipeline& Pipe){
	if (Pipe.Size()==0)
	{
		return FALSE;
	}
	eElectron e;
	Pipe.Pop(&e);
	
	if (e.EnergyType() !=  TYPE_PIPELINE)
	{
		return FALSE;
	}
	
	ePipeline* Value = (ePipeline*)e.Value();
	if (Value->GetID() != m_Alias)
	{
		return FALSE;
	}
	
	eElectron e1;
	Value->Pop(&e1);
	if (e1.EnergyType() != TYPE_STRING)
	{
		return FALSE;
	}

	tstring Name = *(tstring*)e1.Value();
	
	map<tstring,BOOL>::iterator it = m_RadioSet.begin();
	while(it != m_RadioSet.end()){
		if (it->first == Name)
		{
			it->second = TRUE;
		}else{
			it->second = FALSE;
		}
		it ++;
	}	
	return TRUE;
}
void CWSFormView::CRadioSet::ToPipe(ePipeline& Pipe){
    map<tstring,BOOL>::iterator it = m_RadioSet.begin();
	while(it != m_RadioSet.end()){
		if (it->second)
		{
			ePipeline Value(m_Alias);
			Value.PushString(it->first);
			Pipe.PushPipe(Value);
			return;
		}
		it ++;
	}		
};

//////////////////////////////////////////////////////////////////////////
CWSFormView::CComboList::CComboList(int64 ID,CVSpace2* Parent,tstring Title,tstring CurText)
:CInterControl(ID,Parent,CONTROL_COMBOLIST),m_Title(Title),m_CurText(CurText)
{
	
};

CWSFormView::CComboList::~CComboList(){

}

void CWSFormView::CComboList::Draw(HDC hDC,ePipeline* Pipe){
	RECT rc = GetArea();
	COLORREF crText = RGB(0,0,0);
	COLORREF Oldcr = ::SetTextColor(hDC,crText);
	
	rc.right = m_rcControl.left-2;
	::DrawText(hDC,m_Title.c_str(),m_Title.size(),&rc,DT_LEFT|DT_VCENTER|DT_SINGLELINE);	
	
	if (m_hControl == NULL)
	{
		rc = m_rcControl;
		rc.left += m_AreaLeft;
		rc.right += m_AreaLeft;
		rc.top  += m_AreaTop;
		rc.bottom += m_AreaTop;
		
		if (m_State & SPACE_FOCUSED)
		{
			DrawEdge(hDC,rc,RGB(128,128,128));
		}else{
			DrawEdge(hDC,rc,RGB(198,198,198));
		}
		
		DeflateRect(&rc,1,1,1,1);
		::DrawText(hDC,m_CurText.c_str(),m_CurText.size(),&rc,DT_LEFT|DT_VCENTER|DT_SINGLELINE);	
	}
	
	::SetTextColor(hDC,Oldcr);		
};

void CWSFormView::CComboList::OnLButtonDown(POINT& point){
	if(m_hControl == NULL){
		CWinSpace2* Parent = (CWinSpace2*)m_Parent;
		
		DWORD style = CBS_DROPDOWN | WS_CHILD | WS_VISIBLE|WS_EX_STATICEDGE|WS_BORDER;
	
		RECT rc;
		rc.left=0;
		rc.top = 0;
		rc.right = m_rcControl.right-m_rcControl.left;
		rc.bottom = 200;

		m_hControl  =  CreateWindowEx(
			0,//WS_EX_STATICEDGE,      // extended window style
			_T("COMBOBOX"),  // pointer to registered class name
			NULL, // pointer to window name
			style,        // window style
			rc.left,                // horizontal position of window
			rc.top,                // vertical position of window
			RectWidth(rc),           // window width
			RectHeight(rc),          // window height
			Parent->GetHwnd(),      // handle to parent or owner window
			(HMENU)0,          // handle to menu, or child-window identifier
			GetHinstance(),  // handle to application instance
			NULL       // pointer to window-creation data
			);
		assert(m_hControl != NULL);
		if(m_hControl==NULL)return;
		::SetWindowText(m_hControl,m_CurText.c_str());
		Layout();
	}else  if (m_hControl)
	{
		if(::GetWindowTextLength(m_hControl))return ;

		TCHAR buf[255];
		::GetWindowText(m_hControl,buf,255);
		m_CurText = buf;
		::DestroyWindow(m_hControl);
		m_hControl = NULL;
	};
};

void CWSFormView::CComboList::OnLoseFocus(){
	if (m_hControl)
	{
		TCHAR buf[255];
		::GetWindowText(m_hControl,buf,255);
		m_CurText = buf;
		::DestroyWindow(m_hControl);
		m_hControl = NULL;
	};
}
BOOL CWSFormView::CComboList::FromPipe(ePipeline& Pipe){
	if (Pipe.Size()==0)
	{
		return FALSE;
	}
	eElectron e;
	Pipe.Pop(&e);
	
	if (e.EnergyType() !=  TYPE_PIPELINE)
	{
		return FALSE;
	}
	
	ePipeline* Value = (ePipeline*)e.Value();
	if (Value->GetID() != m_Alias)
	{
		return FALSE;
	}
	
	eElectron e1;
	Value->Pop(&e1);
	if (e1.EnergyType() != TYPE_STRING)
	{
		return FALSE;
	}

	m_CurText = *(tstring*)e1.Value();
	return TRUE;
};

void CWSFormView::CComboList::ToPipe(ePipeline& Pipe){
	OnLoseFocus();
	ePipeline Value(m_Alias);
    Value.PushString(m_CurText);
	Pipe.PushPipe(Value);
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWSFormView::CWSFormView()
{
	m_HeaderWidth = 10;
	m_Padding = 8;
}

CWSFormView::~CWSFormView()
{

}


BOOL CWSFormView::FromPipe(ePipeline& Pipe){
	for (uint32 i=0; i<m_ChildList.size(); i++)
	{
		CInterControl* Control = (CInterControl*)m_ChildList[i];
		if (!Control->FromPipe(Pipe))
		{
			return FALSE;
		}
	}
    return TRUE;
};

void CWSFormView::ToPipe(ePipeline& Pipe){
	for (uint32 i=0; i<m_ChildList.size(); i++)
	{
		CInterControl* Control = (CInterControl*)m_ChildList[i];
		Control->ToPipe(Pipe);	
	}
}


LRESULT CWSFormView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	
	if(message == WM_ERASEBKGND)
		return TRUE;
	else if(message == WM_LBUTTONDOWN){
		return OnLButtonDown(wParam,lParam);
	}
	return CWSListView::Reaction(message,wParam,lParam);
}



LRESULT CWSFormView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	
	CWSScrollView::OnLButtonDown(wParam,lParam);
	if (m_ScrollSelected && m_SpaceSelected)
	{
		CInterControl* OldControl = (CInterControl*)m_SpaceSelected;
		OldControl->OnLoseFocus(); 
		m_SpaceSelected = NULL;
	}

	POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;


	CVSpace2* Space = Hit(point);

	if (Space == NULL)
	{
		return 0;
	}

	if (Space->m_Parent == this )
	{	
		if (m_SpaceSelected && m_SpaceSelected->m_Parent == this && m_SpaceSelected !=Space)
		{
			CInterControl* OldControl = (CInterControl*)m_SpaceSelected;
			OldControl->OnLoseFocus(); 
			m_SpaceSelected = NULL;
		}

		CInterControl* Control = (CInterControl*)Space;
		m_SpaceSelected = Control;

		Control->OnLButtonDown(point);

		Invalidate();
	}
	return 0;
}