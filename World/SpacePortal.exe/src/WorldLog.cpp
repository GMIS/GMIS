// WorldLog.cpp: implementation of the CWorldLog class.
//
//////////////////////////////////////////////////////////////////////

#include "WorldLog.h"
#include "UserTimer.h"

InfoItem::InfoItem(int64 ID,const TCHAR* text)
:CVSpace2(ID),m_Text(text)
{	

};

void InfoItem::Draw(HDC hDC,ePipeline* Pipe /*=Pipe*/){

    RECT rc = GetArea();
	COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,255,0));
	::DrawText(hDC,m_Text.c_str(),m_Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|
			DT_NOPREFIX|DT_VCENTER);	
	::SetTextColor(hDC,Oldcr);	
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWorldLog::CWorldLog()
{
	//m_bShowBorder = true;
	m_crViewBkg =  RGB(0,0,0);
	m_MaxItemNum = 1000;
}

CWorldLog::~CWorldLog()
{

}

InfoItem* CWorldLog::AddInfo(const TCHAR* text){
	int64 ID = AbstractSpace::CreateTimeStamp();
	tstring Time = AbstractSpace::GetTimer()->GetHMS(ID);
	Time = Format1024(_T("%s   %s"),Time.c_str(),text);
	InfoItem* Item = new InfoItem(ID,Time.c_str()); 
    SIZE s = CalcuTextSize(Time.c_str());
	Item->SetAreaSize(s.cx,s.cy);
	SendChildMessage(GetHwnd(),ADD_INFO,(int64)Item,0);
	return Item;
};

void CWorldLog::ClearAllItem(){
	SendChildMessage(GetHwnd(),CLR_INFO,0,0);	
};

	

LRESULT CWorldLog::ChildReaction(SpaceRectionMsg* SRM){
	switch(SRM->Msg)
	{
	case ADD_INFO:
		{
			if(m_ChildList.size() == m_MaxItemNum){
				//InfoItem* Front = (InfoItem*)m_ChildList.front();
				//delete Front;	
                Clear();
			}
			InfoItem* Item = (InfoItem*)SRM->wParam;
			PushChild(Item);
			Layout(false);
			EnsureVisible(Item,true);
		}
		break;
	case CLR_INFO:
		Clear();
	    Layout(true);
		break;	
	}
	return 0;
}

LRESULT CWorldLog::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	LRESULT ret = CWSScrollView::OnLButtonDown(wParam,lParam);
 	
    POINTS* p = (POINTS*)(&lParam);

	if(m_ScrollSelected){
		//if(m_SpaceSelected)m_SpaceSelected->m_State &= ~SPACE_SELECTED;
		//m_SpaceSelected = NULL; //when clicked on the Scrollbar, it can not click on other
		return 0;
	}

	POINT point;
	point.x =p->x;
	point.y =p->y;

	CVSpace2* SpaceSelected = Hit(point);

	if(SpaceSelected && SpaceSelected->m_Alias != 0){
		if(m_SpaceSelected)m_SpaceSelected->m_State &= ~SPACE_SELECTED;
		SpaceSelected->m_State |= SPACE_SELECTED;
		m_SpaceSelected = SpaceSelected;
	}	
	Invalidate();
	return 0;
}

LRESULT CWorldLog::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if(message == WM_LBUTTONDOWN){
		return OnLButtonDown(wParam,lParam);
	}
	return CWSListView::Reaction(message,wParam,lParam);
}
