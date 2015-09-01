// TitleView.cpp: implementation of the CTitleView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "TitleView.h"
#include "SystemSetting.h"
#include "WorldShow.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTitleView::CTitleView()
{	
    m_TitleHeight  = 18;
    m_NcBoderWidth = 0;

	HICON	 hIcon  = 0;
	
	
	IconItem*     Icon     = new IconItem(ID_ICON,hIcon);
	TextItem*     TextLabel= new TextItem(ID_TITLETEXT,_T(""));
    CloseboxItem* Closebox = new CloseboxItem(ID_CLOSE);
	
	BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
	if(TitleSpace){
		TitleSpace->DeleteAll();
		
		TitleSpace->PushChild(Icon);
		TitleSpace->PushChild(TextLabel);
		TitleSpace->PushChild(Closebox);
	}
/*	
	COLORREF crAct    = RGB(0,0,255);//0x0064ff; //蓝RGB(120,120,120);
	COLORREF crInact  = RGB(48,101,150); 
	COLORREF crFocuse = 0x77aaff;    //洋红;
	
	TitleSpace->SetColor(crAct,crAct,crAct);

	BorderItem* NcLeftBorderSpace   = (BorderItem*)m_NcBorderList[1];
	BorderItem* NcRightBorderSpace  = (BorderItem*)m_NcBorderList[2];
	BorderItem* NcBottomBorderSpace = (BorderItem*)m_NcBorderList[3];
	
	if(NcLeftBorderSpace)NcLeftBorderSpace->SetColor(crAct,crInact,crFocuse);
	if(NcRightBorderSpace)NcRightBorderSpace->SetColor(crAct,crInact,crFocuse);
	if(NcBottomBorderSpace)NcBottomBorderSpace->SetColor(crAct,crInact,crFocuse);	
*/
}

CTitleView::~CTitleView()
{

}
    
void CTitleView::SetID(int64 ID){
	m_Alias = ID;
	BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
    CloseboxItem* Closebox = (CloseboxItem*)TitleSpace->m_ChildList[2];
    Closebox->m_Alias = ID;
}	

void CTitleView::SetIcon(HICON nIcon){
	BorderItem* TitleSpace = NULL;		
	if(m_NcBorderList[0] && m_NcBorderList[0]->m_Alias == ID_TITLE){
		TitleSpace = (BorderItem*)m_NcBorderList[0];
	}	
	if(TitleSpace == NULL)return;
	IconItem*     Item     = (IconItem*)TitleSpace->FindSpace(ID_ICON);
	if(Item)Item->m_hIcon = nIcon;
}
	

void CTitleView::SetTitle(const TCHAR* Text){
	BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
	TextItem*     TextLabel = (TextItem*)TitleSpace->FindSpace(ID_TITLETEXT);
	if(TextLabel)TextLabel->m_Text = Text;
	if(GetHwnd())Invalidate();
}

void CTitleView::NcLayout(){
	
	CWSTitleView::NcLayout();
	
	BorderItem* TitleSpace = (BorderItem*)m_NcBorderList[0];
    if(TitleSpace->m_ChildList.size()==0)return;

	IconItem*     Icon = (IconItem*)TitleSpace->m_ChildList[0];
	TextItem*     TextLabel = (TextItem*)TitleSpace->m_ChildList[1];
    CloseboxItem* Closebox  = (CloseboxItem*)TitleSpace->m_ChildList[2];
	
	int32 len = (TitleSpace->AreaHeight()-13)/2;
	
    Closebox->m_AreaTop = TitleSpace->m_AreaTop+len;
	Closebox->m_AreaBottom = TitleSpace->m_AreaBottom-len;
    Closebox->m_AreaRight = TitleSpace->m_AreaRight-len;
	Closebox->m_AreaLeft = Closebox->m_AreaRight-14;
  	
    if (Icon->m_hIcon != NULL)
    {
		Icon->SetArea(TitleSpace);
		Icon->m_AreaRight = TitleSpace->m_AreaLeft+TitleSpace->AreaHeight();
		
		TextLabel->SetArea(TitleSpace);
		TextLabel->m_AreaLeft = Icon->m_AreaRight+2;
		TextLabel->m_AreaRight = Closebox->m_AreaLeft-4;
    } 
    else
    {
		TextLabel->SetArea(TitleSpace);
		TextLabel->m_AreaLeft += 2;
		TextLabel->m_AreaRight = Closebox->m_AreaLeft-4;
    }

}
LRESULT CTitleView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){

	switch(message){

	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	default:
		return CWSTitleView::Reaction(message,wParam,lParam);
	}
}
LRESULT CTitleView::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
{
    POINTS* p = (POINTS*)(&lParam);

    CVSpace2 * Space = NcHit(p->x,p->y);	
	if(Space){
		int64 ID = Space->m_Alias;
        if(ID==ID_CLOSE){
			SendParentMessage(CM_CLOSEVIEW,m_Alias,0,Space);
		}
	}
    return 0;
}

