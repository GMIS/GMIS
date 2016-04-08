// ResultView.cpp: implementation of the CResultView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable : 4786)

#include "TexasPokerEvaluate.h"
#include "ResultView.h"

HANDLE CResultView::ImageItem::hPokerImage = NULL;

CResultView::InfoItem::InfoItem(int64 ID,const TCHAR* text)
:CVSpace2(ID),m_Text(text)
{	
	
};

void CResultView::InfoItem::Draw(HDC hDC,ePipeline* Pipe /*=Pipe*/){
	//输出文字
    RECT rc = GetArea();


	if(m_State & SPACE_SELECTED){
        DrawEdge(hDC,&rc,RGB(128,128,128));
	}else if(m_State & SPACE_FOCUSED){
		DrawEdge(hDC,&rc,RGB(128,128,128));
	}
	rc.left +=2;
	rc.top  +=2;
	rc.bottom -=2;
	rc.right -=2;

	COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,0,0));
	::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
		DT_NOPREFIX|DT_VCENTER);	
	::SetTextColor(hDC,Oldcr);	
}


CResultView::ImageItem::ImageItem(int64 ID,const TCHAR* text)
:CVSpace2(ID),m_Text(text)
{	
	
};

CResultView::ImageItem::~ImageItem(){

};

void CResultView::ImageItem::Draw(HDC hDC,ePipeline* Pipe /*=Pipe*/){
	//输出文字
    RECT rc = GetArea();
	
	if(m_State & SPACE_SELECTED){
        DrawEdge(hDC,&rc,RGB(128,128,128));
	}else if(m_State & SPACE_FOCUSED){
		DrawEdge(hDC,&rc,RGB(128,128,128));
	}
	rc.left +=2;
	rc.top  +=2;
	rc.bottom -=2;
	rc.right -=2;
	

	COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,0,0));
	::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
		DT_NOPREFIX|DT_VCENTER);	
	
	rc.top += 21;
	
	HDC MemDC=CreateCompatibleDC(hDC); 
	HBITMAP Oldbitmap = (HBITMAP)SelectObject(MemDC,NULL);
	SelectObject(MemDC,hPokerImage);
	
	int x = rc.left;
	int y = rc.top;
	
	for (int i=0; i<m_CardList.size(); i++)
	{	
		int32 Face = m_CardList[i]%100;
		int32 Suit = m_CardList[i]/100;
		
        tstring FaceStr = GetFaceStr(Face);
		::SetRect(&rc,x,y,x+16,y+20);
		::DrawText(hDC,FaceStr.c_str(),FaceStr.size(),&rc,DT_CENTER);	
		BitBlt(hDC, x,y+16,16,16,MemDC, Suit*16,0,SRCCOPY);				
		x += 22;
	};
	
	::SetTextColor(hDC,Oldcr);	
	::SelectObject(MemDC,Oldbitmap);
	::DeleteDC(MemDC); 
	
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResultView::CResultView()
{
  m_crViewBkg = RGB(255,255,255);
  m_MaxItemCount = 200;
  SetTextFont(12,FALSE,FALSE,_T("新宋体"));
}

CResultView::~CResultView()
{

}

void CResultView::AddInfo(ePipeline* Msg){
	SendChildMessage(GetHwnd(),RESULT_ADD_INFO,(int64)Msg,0);
};


LRESULT CResultView::ChildReaction(SpaceRectionMsg* SRM){

	ePipeline* Msg = (ePipeline*)SRM->wParam;
	if (SRM->Msg == RESULT_ADD_INFO)
	{
		CVSpace2* Item = NULL;
		tstring s;
		Msg->PopString(s);

		int64 ID = m_ChildList.size()==0? 0:m_ChildList.back()->m_Alias+1;
		if(Msg->Size() == 0){
			Item  = new InfoItem(ID,s.c_str());
			SIZE  size=CalcuTextSize(s.c_str());
            size.cx=size.cx<250?250:size.cx+10;
			size.cy=size.cy<25?25:size.cy;
			Item->SetAreaSize(size.cx,size.cy);
			PushChild(Item);
		}else {
			Item  = new ImageItem(ID,s.c_str());
			SIZE  size=CalcuTextSize(s.c_str());
            size.cx=size.cx<250?250:size.cx+10;
			size.cy +=25;
			size.cy=size.cy<50?50:size.cy;
			Item->SetAreaSize(size.cx,size.cy);

			ImageItem* v=(ImageItem*)Item;
			while(Msg->Size()){
				int32 card = Msg->PopInt();
				v->m_CardList.push_back(card);
			}		
			PushChild(Item);
		}
		int n = m_ChildList.size()-m_MaxItemCount;
		if (n>0)
		{
			for(int i=0; i<n; i++){
				CVSpace2* v = m_ChildList[i];
				delete v;
				m_ChildList.pop_front();
			}
		}
		Layout();
		if(Item)EnsureVisible(Item,FALSE);
	}
	return 0;
}

LRESULT CResultView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
    if (message == WM_LBUTTONDOWN)
	{
		return OnLButtonDown(wParam,lParam);
	}else {
		return CWSListView::Reaction(message,wParam,lParam);
	}
	
}
LRESULT CResultView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	LRESULT ret = CWSScrollView::OnLButtonDown(wParam,lParam);
	
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	
	if(m_ScrollSelected){
		if(m_SpaceSelected){ 
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			//m_Toolbar.m_Owner = NULL;
			m_SpaceSelected = NULL;
		}
		return 0;
	}
	
	CVSpace2* NewSelect = Hit(point);
	
	if(NewSelect ==NULL)return 0; 
	
	if(NewSelect != m_SpaceSelected){		
				
		//取消旧的选择
		if(m_SpaceSelected){ 
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			m_SpaceSelected = NULL;
		}

		//设置新选择
		m_SpaceSelected = NewSelect;
		m_SpaceSelected->m_State |= SPACE_SELECTED;
		Layout();		
	}
	return 0;
}


