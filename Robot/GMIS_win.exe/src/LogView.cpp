
#pragma  warning (disable:4786)

#include "LogView.h"
#include "Brain.h"
#include "UserTimer.h"
#include "GUIMsgDefine.h"
#include "GMIS.h"

void CLogView::CFooterBar::ControlBnt::Draw(HDC hDC,ePipeline* Pipe /*=NULL*/){
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


CLogView::CFooterBar::CFooterBar()
	:m_UpdateBnt(BNT_UPDATE,_T("Update")),
     m_ClearBnt(BNT_CLEAR,_T("Clear")),
	 m_CloseBnt(BNT_CLOSE,_T("Close"))
{
	m_crWinBorder  = RGB(128,128,128);
	m_crViewBkg    = RGB(245,245,243);

	PushChild(&m_UpdateBnt);
	PushChild(&m_ClearBnt);
	PushChild(&m_CloseBnt);
}

CLogView::CFooterBar::~CFooterBar()
{
	m_ChildList.clear();
}

void CLogView::CFooterBar::Layout(bool Redraw /*= true*/){
	RECT rc;
	GetClientRect(GetHwnd(),&rc);

	int32 w = RectWidth(rc)/3;
	if (w==0)
	{
		return;
	}

	int32 w1=w/2;

	m_UpdateBnt.SetArea(rc.left+w1,rc.top+1,rc.left+w1+50,rc.bottom-1);
	m_ClearBnt.SetArea(rc.left+w+w1,rc.top+1,rc.left+w+w1+50,rc.bottom-1);
	m_CloseBnt.SetArea(rc.left+w+w+w1,rc.top+1,rc.left+w+w+w1+50,rc.bottom-1);

}

LRESULT CLogView::CFooterBar::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
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

LRESULT CLogView::CFooterBar::OnPaint(){

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

LRESULT CLogView::CFooterBar::OnLButtonDown(WPARAM wParam, LPARAM lParam){
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

LRESULT CLogView::CFooterBar::OnLButtonUp(WPARAM wParam, LPARAM lParam){

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


CLogView::CLogView(void)
{
	m_LastItemTimeStamp = 0;
	m_NcBoderWidth = 4;
	m_LogFlag = 0;

	m_crViewBkg = RGB(255,255,255);

	SetTitle(_T("LogView"));
	//////////////////////////////////////////////////////////////////////////

	//NOTE: CHeadline.m_Alias must =0;
	CWSFormView::CHeadline* Headline1 = new CWSFormView::CHeadline(0,&m_OptionView,_T("信息传送"));
	Headline1->SetArea(0,0,200,24);
    m_OptionView.PushChild(Headline1);

	CWSFormView::CCheckBox* CheckBox10 = new CWSFormView::CCheckBox(LOG_MSG_IO_PUSH,&m_OptionView,_T("PUSH_TO_SEND"),FALSE);
	CheckBox10->SetArea(0,0,200,24);
	CheckBox10->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox10);

	CWSFormView::CCheckBox* CheckBox11 = new CWSFormView::CCheckBox(LOG_MSG_I0_RECEIVED,&m_OptionView,_T("LINKER_RECEIVED"),FALSE);
	CheckBox11->SetArea(0,0,200,24);
	CheckBox11->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox11);

	CWSFormView::CCheckBox* CheckBox12 = new CWSFormView::CCheckBox(LOG_MSG_IO_SENDED,&m_OptionView,_T("LINKER_SENDED"),FALSE);
	CheckBox12->SetArea(0,0,200,24);
	CheckBox12->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox12);

	CWSFormView::CCheckBox* CheckBox13 = new CWSFormView::CCheckBox(LOG_MSG_IO_REMOTE_RECEIVED,&m_OptionView,_T("REMOTE_LINKER_RECEIVED"),FALSE);
	CheckBox13->SetArea(0,0,200,24);
	CheckBox13->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox13);



	//////////////////////////////////////////////////////////////////////////
	CWSFormView::CHeadline* Headline2 = new CWSFormView::CHeadline(0,&m_OptionView,_T("信息分发"));
	Headline2->SetArea(0,0,200,24);
	m_OptionView.PushChild(Headline2);

	CWSFormView::CCheckBox* CheckBox21 = new CWSFormView::CCheckBox(LOG_MSG_CENTRL_NERVE_PUSH,&m_OptionView,_T("LOG_MSG_CENTRL_NERVE_PUSH"),FALSE);
	CheckBox21->SetArea(0,0,260,24);
	CheckBox21->SetControlRect(240,0,256,24);
	m_OptionView.PushChild(CheckBox21);

	CWSFormView::CCheckBox* CheckBox23 = new CWSFormView::CCheckBox(LOG_MSG_NERVE_PUSH,&m_OptionView,_T("LOG_MSG_NERVE_PUSH"),FALSE);
	CheckBox23->SetArea(0,0,200,24);
	CheckBox23->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox23);

	CWSFormView::CCheckBox* CheckBox24 = new CWSFormView::CCheckBox(LOG_MSG_TASK_PUSH,&m_OptionView,_T("LOG_MSG_TASK_PUSH"),FALSE);
	CheckBox24->SetArea(0,0,200,24);
	CheckBox24->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox24);

	//////////////////////////////////////////////////////////////////////////
	CWSFormView::CHeadline* Headline3 = new CWSFormView::CHeadline(0,&m_OptionView,_T("信息处理"));
	Headline3->SetArea(0,0,200,24);
	m_OptionView.PushChild(Headline3);

	CWSFormView::CCheckBox* CheckBox31 = new CWSFormView::CCheckBox(LOG_MSG_PROC_BEGIN,&m_OptionView,_T("LOG_MSG_PROC_BEGIN"),FALSE);
	CheckBox31->SetArea(0,0,200,24);
	CheckBox31->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox31);


	CWSFormView::CCheckBox* CheckBox32 = new CWSFormView::CCheckBox(LOG_MSG_PROC_PATH,&m_OptionView,_T("LOG_MSG_PROC_PATH"),FALSE);
	CheckBox32->SetArea(0,0,200,24);
	CheckBox32->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox32);

	CWSFormView::CCheckBox* CheckBox33 = new CWSFormView::CCheckBox(LOG_MSG_RUNTIME_TIP,&m_OptionView,_T("LOG_MSG_RUNTIME_TIP"),FALSE);
	CheckBox33->SetArea(0,0,200,24);
	CheckBox33->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox33);

	//////////////////////////////////////////////////////////////////////////
	CWSFormView::CHeadline* Headline4 = new CWSFormView::CHeadline(0,&m_OptionView,_T("其他"));
	Headline4->SetArea(0,0,200,24);
	m_OptionView.PushChild(Headline4);

	CWSFormView::CCheckBox* CheckBox41 = new CWSFormView::CCheckBox(LOG_ERROR,&m_OptionView,_T("LOG_ERROR"),FALSE);
	CheckBox41->SetArea(0,0,200,24);
	CheckBox41->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox41);

	CWSFormView::CCheckBox* CheckBox42 = new CWSFormView::CCheckBox(LOG_WARNING,&m_OptionView,_T("LOG_WARNING"),FALSE);
	CheckBox42->SetArea(0,0,200,24);
	CheckBox42->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox42);

	CWSFormView::CCheckBox* CheckBox43 = new CWSFormView::CCheckBox(LOG_TIP,&m_OptionView,_T("LOG_TIP"),FALSE);
	CheckBox43->SetArea(0,0,200,24);
	CheckBox43->SetControlRect(170,0,186,24);
	m_OptionView.PushChild(CheckBox43);

}


CLogView::~CLogView(void)
{
}


void CLogView::ResetFlag(){
	m_LogFlag = 0;

	deque<CVSpace2*>::iterator it = m_OptionView.m_ChildList.begin();
	while (it != m_OptionView.m_ChildList.end())
	{
		CVSpace2* v = *it;
		if (v->m_Alias !=0)// CHeadline
		{
			CWSFormView::CCheckBox* ck = (CWSFormView::CCheckBox*)v;
			ck->m_bChecked = FALSE;
		}
		it++;
	}
	ePipeline Cmd(TO_BRAIN_MSG::GUI_SET_LOG_FLAG);
	Cmd.PushInt(m_LogFlag);
	int64 EventID = AbstractSpace::CreateTimeStamp();
	GetGUI()->SendMsgToBrain(SYSTEM_SOURCE,DEFAULT_DIALOG,Cmd,EventID);

	::KillTimer(GetHwnd(),LOG_TIMER);
}

void CLogView::UpdateFlag(){
	m_LogFlag = 0;

	deque<CVSpace2*>::iterator it = m_OptionView.m_ChildList.begin();
	while (it != m_OptionView.m_ChildList.end())
	{
		CVSpace2* v = *it;
		if (v->m_Alias!=0) // CHeadline
		{
			CWSFormView::CCheckBox* ck = (CWSFormView::CCheckBox*)v;
			if(ck->m_bChecked == TRUE){
				m_LogFlag |= ck->m_Alias;
			};	
		}
		it++;
	}

	if (m_LogFlag)
	{
		ePipeline Cmd(TO_BRAIN_MSG::GUI_SET_LOG_FLAG);
		Cmd.PushInt(m_LogFlag);
		int64 EventID = AbstractSpace::CreateTimeStamp();
		GetGUI()->SendMsgToBrain(SYSTEM_SOURCE,DEFAULT_DIALOG,Cmd,EventID);

	}	
}
void  CLogView::UpdateItem(){
	if (m_LogFlag==0)
	{
		return;
	}

	int64 t = m_LastItemTimeStamp;
	ePipeline Pipe;
	m_LogDB.ReadItem(m_LastItemTimeStamp,Pipe);
	while (Pipe.Size())
	{
		t = Pipe.PopInt();
		tstring Who, Say;
		Pipe.PopString(Who);
		Pipe.PopString(Say);

		tstring timestr = Pipe.GetTimer()->GetHMSM(t);

		tstring text = Format1024(_T("%010s %10s:  %s"),timestr.c_str(),Who.c_str(),Say.c_str());
		m_LogEdit.AddText(text.c_str(),false);
	}

	m_LastItemTimeStamp = t;
}

void CLogView::Layout(bool Redraw /* = true */)
{
	CWSFrameView::Layout(FALSE);

	::MoveWindow(m_OptionView.GetHwnd(),m_rcClient.left,m_rcClient.top,300,RectHeight(m_rcClient)-24,TRUE);
	::MoveWindow(m_LogEdit.GetHwnd(),302,m_rcClient.top,RectWidth(m_rcClient)-302,RectHeight(m_rcClient)-24,TRUE);
	::MoveWindow(m_FooterBar.GetHwnd(),m_rcClient.left,m_rcClient.bottom-24,RectWidth(m_rcClient),24,TRUE);
}

LRESULT  CLogView::ParentReaction(SpaceRectionMsg* SRM){
	if (SRM->Msg == BNT_CLOSE)
	{
		return SendMessage(GetHwnd(),WM_CLOSE,0,0);

	}else if (SRM->Msg == BNT_CLEAR)
	{
		m_LogDB.Clear();
		m_LogEdit.ClearText();

	}else if (SRM->Msg == BNT_UPDATE)
	{
		UpdateFlag();
	}
	return 0;
}

LRESULT CLogView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if (message == WM_CREATE)
	{
		return OnCreate(wParam,lParam);
	}
	else if(message == WM_GETMINMAXINFO){
			return OnGetMinMaxInfo(wParam,lParam);
	}
	else if(message ==  WM_TIMER && wParam == LOG_TIMER)
	{
		UpdateItem();
	}
	return CWSFrameView::Reaction(message,wParam,lParam);
}

LRESULT CLogView::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT rc = {0};

	if(!m_OptionView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}
	if(!m_LogEdit.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}
	if(!m_FooterBar.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}

	tstring Dir = GetCurrentDir();
	tstring LogDir =  Dir+_T("\\Log.DB");

	m_LogDB.Open(LogDir);

	::SetTimer(GetHwnd(),LOG_TIMER,1000,NULL);


	return 0;
}

LRESULT CLogView::OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	lpmmi->ptMinTrackSize.x = 620;
	lpmmi->ptMinTrackSize.y = 400;
	CWSFrameView::OnGetMinMaxInfo(wParam,lParam);
	return 0;
}