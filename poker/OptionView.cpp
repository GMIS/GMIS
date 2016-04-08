// OptionView.cpp: implementation of the COptionView class.
//
//////////////////////////////////////////////////////////////////////

#include "OptionView.h"
#include "StatusBar.h"

COptionView::CMyOptionView::CMyOptionView(){
	
}

COptionView::CMyOptionView::~CMyOptionView(){

}

LRESULT COptionView::CMyOptionView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if (message == WM_LBUTTONDOWN)
	{
		return OnLButtonDown(wParam,lParam);
	}else{
		return CWSFormView::Reaction(message,wParam,lParam);
	} 
}

LRESULT COptionView::CMyOptionView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
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
    
		ePipeline ResultPipe;
		ToPipe(ResultPipe);
		SendParentMessage(NEW_OPTION_RESULT,(int64)&ResultPipe,0,NULL);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionView::COptionView()
{

	BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
	TextItem*     Text     = new TextItem(ID_TITLETEXT,_T("Option"));	
	CloseboxItem* Closebox = new CloseboxItem(ID_CLOSE);
	
	if(TitleSpace){
		TitleSpace->DeleteAll();
		TitleSpace->PushChild(Text);
		TitleSpace->PushChild(Closebox);
	}


	CWSFormView::CHeadline* Headline = new CWSFormView::CHeadline(0,&m_OptionView,_T("玩法"));
	Headline->SetArea(0,0,300,24);
    m_OptionView.PushChild(Headline);

	CWSFormView::CRadioSet* Radio = new CWSFormView::CRadioSet(PLAY_TYPE,&m_OptionView);
	Radio->PushRadioValue(_T("1 自由场"),TRUE);
	Radio->PushRadioValue(_T("2 刷局"),FALSE);
	Radio->PushRadioValue(_T("3 定时任务"),FALSE);
	Radio->PushRadioValue(_T("4 转分"),FALSE);
	Radio->PushRadioValue(_T("5 二人刷盆"),FALSE);
	Radio->PushRadioValue(_T("6 刷盆"),FALSE);
	Radio->PushRadioValue(_T("7 比赛"),FALSE);
	Radio->PushRadioValue(_T("8 淘金赛"),FALSE);

	int n = 8;//Radio->m_ChildList.size();
	Radio->SetArea(0,0,300,24*n);
	Radio->SetControlRect(120,0,136,24*n);
	m_OptionView.PushChild(Radio);
	
	Headline = new CWSFormView::CHeadline(0,&m_OptionView,_T("风险"));
	Headline->SetArea(0,0,300,24);
    m_OptionView.PushChild(Headline);
	
	 Radio = new CWSFormView::CRadioSet(PLAY_FACTOR,&m_OptionView);
	Radio->PushRadioValue(_T("1 正常"),TRUE);
	Radio->PushRadioValue(_T("2 保守"),FALSE);
	Radio->PushRadioValue(_T("3 积极"),FALSE);
	
	n = 3;//Radio->m_ChildList.size();
	Radio->SetArea(0,0,300,24*n);
	Radio->SetControlRect(120,0,136,24*n);
	m_OptionView.PushChild(Radio);

	Headline = new CWSFormView::CHeadline(0,&m_OptionView,_T("时间"));
	Headline->SetArea(0,0,300,24);
	m_OptionView.PushChild(Headline);

	Radio = new CWSFormView::CRadioSet(PLAY_TIME,&m_OptionView);
	Radio->PushRadioValue(_T("1 一直玩"),TRUE);
	Radio->PushRadioValue(_T("2 15分钟"),FALSE);
	Radio->PushRadioValue(_T("3 20分钟"),FALSE);
	Radio->PushRadioValue(_T("4 30分钟"),FALSE);

	n = 4;//Radio->m_ChildList.size();
	Radio->SetArea(0,0,300,24*n);
	Radio->SetControlRect(120,0,136,24*n);
	m_OptionView.PushChild(Radio);

	Headline = new CWSFormView::CHeadline(0,&m_OptionView,_T("选场"));
	Headline->SetArea(0,0,300,24);
	m_OptionView.PushChild(Headline);

	Radio = new CWSFormView::CRadioSet(PLAY_SELECTROOM,&m_OptionView);
	Radio->PushRadioValue(_T("1 不选"),TRUE);
	Radio->PushRadioValue(_T("2 自动选100场"),FALSE);
	Radio->PushRadioValue(_T("3 自动选200场"),FALSE);
	Radio->PushRadioValue(_T("4 自动选400场"),FALSE);

	n = 4;//Radio->m_ChildList.size();
	Radio->SetArea(0,0,300,24*n);
	Radio->SetControlRect(120,0,136,24*n);

	m_OptionView.PushChild(Radio);

	
	Headline = new CWSFormView::CHeadline(0,&m_OptionView,_T("其它"));
	Headline->SetArea(0,0,300,24);
	m_OptionView.PushChild(Headline);
	
	CWSFormView::CCheckBox* CheckBox = new CWSFormView::CCheckBox(PLAY_DEBUG,&m_OptionView,_T("调试"),FALSE);
	CheckBox->SetArea(0,0,300,24);
	CheckBox->SetControlRect(120,0,136,24);
	m_OptionView.PushChild(CheckBox);

	CheckBox = new CWSFormView::CCheckBox(PLAY_LINGFEN,&m_OptionView,_T("自动领分"),FALSE);
	CheckBox->SetArea(0,0,300,24);
	CheckBox->SetControlRect(120,0,136,24);
	m_OptionView.PushChild(CheckBox);

	Headline = new CWSFormView::CHeadline(0,&m_OptionView,_T("联系：642128162@qq.com"));
	Headline->SetArea(0,0,300,24);
	m_OptionView.PushChild(Headline);

/*
	CWSFormView::CEditText* EditBox = new CWSFormView::CEditText(1,&m_OptionView,_T("编辑"),FALSE);
	EditBox->SetArea(0,0,300,24);
	EditBox->SetControlRect(50,0,300,24);
	m_OptionView.PushChild(EditBox);
	
	CWSFormView::CCheckBox* CheckBox = new CWSFormView::CCheckBox(2,&m_OptionView,_T("CheckBox"),TRUE);
	CheckBox->SetArea(0,0,300,24);
	CheckBox->SetControlRect(120,0,136,24);
	m_OptionView.PushChild(CheckBox);
	
	CWSFormView::CComboList* ComboList = new CWSFormView::CComboList(2,&m_OptionView,_T("ComboLIst"),_T("没有选择"));
	ComboList->SetArea(0,0,300,24);
	ComboList->SetControlRect(100,0,300,24);
	m_OptionView.PushChild(ComboList);
*/
}

COptionView::~COptionView()
{
	
}

void COptionView::Layout(bool Redraw /* = true */)
{
	CWSTitleView::Layout(FALSE);
	BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
	if(TitleSpace==NULL || TitleSpace->m_ChildList.size()==0)return;
	
	TextItem*     TextLabel = (TextItem*)TitleSpace->m_ChildList[0];
	CloseboxItem* Closebox  = (CloseboxItem*)TitleSpace->m_ChildList[1];
	
	TextLabel->m_AreaTop    = TitleSpace->m_AreaTop;
	TextLabel->m_AreaBottom = TitleSpace->m_AreaBottom;
	TextLabel->m_AreaLeft =   TitleSpace->m_AreaLeft+2;
	TextLabel->m_AreaRight =  TitleSpace->m_AreaRight-16;

	
	Closebox->SetArea(TitleSpace);
	int32 len = ((TitleSpace->m_AreaBottom-TitleSpace->m_AreaTop)-13)/2;
	Closebox->DeflateArea(len,len,len,len);		
	Closebox->m_AreaLeft = Closebox->m_AreaRight-14;

	//Closebox->m_AreaLeft  -=2;

	
	::MoveWindow(m_OptionView.GetHwnd(),m_rcClient.left,m_rcClient.top,RectWidth(m_rcClient),RectHeight(m_rcClient),TRUE);
}


LRESULT COptionView::ParentReaction(SpaceRectionMsg* SRM){
	return SendParentMessage(SRM->Msg,SRM->wParam,SRM->lParam,NULL);
}

LRESULT COptionView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if (message == WM_CREATE)
	{
		return OnCreate(wParam,lParam);
	}else if (message == WM_LBUTTONDOWN)
	{
		return OnLButtonDown(wParam,lParam);
	}
    else return CWSTitleView::Reaction(message,wParam,lParam);
}

LRESULT COptionView::OnCreate(WPARAM wParam, LPARAM lParam){
	RECT rc = {0};
	
	if(!m_OptionView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}
	
	return 0;
}

LRESULT COptionView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	POINT point;
	point.x = GET_X_LPARAM(lParam); 
	point.y = GET_Y_LPARAM(lParam);
	
	CVSpace2 * Space = NcHit(point.x, point.y);	
	if(Space){
		int64 ID = Space->m_Alias;
		switch(ID){
		case ID_TITLETEXT: //TextLabel
			::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
			break;	
		case ID_CLOSE:
		    SendParentMessage(BNT_TASK_OPTION,0,0,NULL);
			//::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_CLOSE, 0);
			return 0;
		}
	}
	return ::DefWindowProc(m_hWnd,WM_LBUTTONDOWN,wParam,lParam);
}


