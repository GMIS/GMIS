// LocalObjectView.cpp: implementation of the LocalObjectView class.
//
//////////////////////////////////////////////////////////////////////

#include "GMIS.h"
#include "space.h"
#include "LocalObjectView.h"
#include "SystemSetting.h"
#include "NotifyMsgDef.h"
#include "UseShGetFileInfo.h"
#include "MapView.h"
#include "resource.h"


CLocalObjectView::CObjectLabel::CObjectLabel(int32 TypeID,tstring Name,HICON hIcon){
	m_Alias = TypeID;
	m_Name=Name; 
	m_hIcon = hIcon;
	SetAreaSize(300,20);
};

void CLocalObjectView::CObjectLabel::Draw(HDC hDC, ePipeline* Pipe){
	
	RECT rc = GetArea();
    //FillRect(hDC,rc,SS.crBrainViewItemTextBk); 
	
	if(m_hIcon){
		::DrawIconEx(hDC, rc.left,rc.top, m_hIcon, 16, 16, 0, 0, DI_NORMAL);
	}
	
	COLORREF crOld = SetTextColor(hDC,SS.crBrainViewItemText);
	
	rc.left += RectHeight(rc)+2;
	rc.right =rc.left+80;
	::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	
	//引用数
	rc.left =m_AreaLeft+ 100; rc.right =rc.left+100 ;
	TCHAR buf[30];
	_stprintf(buf,_T("Num: %d"),m_ChildList.size());
	::DrawText(hDC,buf,_tcslen(buf),&rc,DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);	
    
	SetTextColor(hDC,crOld);
};

CLocalObjectView::CObjectItem::CObjectItem(ePipeline& ItemData)
:m_ObjectData(ItemData)
{
	m_Alias = 0; //AbstractSpace::CreateTimeStamp();

	HICON hIcon = NULL;		
	assert(m_ObjectData.m_Type!=LOCAL_SPACE && m_ObjectData.m_Type != OUTER_SPACE);
	
	if(m_ObjectData.m_Type == CONTAINER_SPACE){
		hIcon = MapItem::hContainer;
	}else if(m_ObjectData.m_Type == ROBOT_VISITER){
		hIcon = MapItem::hPeople;
	}
	else{
		hIcon = CUseShGetFileInfo::GetFileIconHandle(m_ObjectData.m_Name.c_str(),TRUE);
	} 

    m_hIcon = hIcon;
}

CLocalObjectView::CObjectItem::~CObjectItem(){

}

ePipeline* CLocalObjectView::CObjectItem::GetItemData(){
   return m_ObjectData.GetItemData();
}

void CLocalObjectView::CObjectItem::Draw(HDC hDC,ePipeline* Pipe /*= NULL*/){
	
	//输出内容
	RECT rc = GetArea();
	
	COLORREF crText=SS.crBrainViewItemText;
	
	if(m_State & SPACE_SELECTED){
		FillRect(hDC,rc,SS.crBrainViewToolbarBk);
		crText = RGB(255,255,255);
		//	DrawEdge(hDC,&rc,SS.crBrainViewBorder);
	}else if(m_State & SPACE_FOCUSED){
		DrawEdge(hDC,&rc,SS.crBrainViewToolbarBk);
	}
	
	rc.top+=2;
	rc.left+=2;
	rc.bottom -=2;
	
	
	if(m_hIcon){	
		::DrawIconEx(hDC, rc.left,rc.top, m_hIcon, 16, 16, 0, 0, DI_NORMAL);
	}
	
	COLORREF Oldcr = ::SetTextColor(hDC,crText);
	rc.bottom = rc.top +20;
	rc.left = rc.left+ 20;
    rc.right = rc.left+170;
	::DrawText(hDC,m_ObjectData.m_Name.c_str(),m_ObjectData.m_Name.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
		DT_WORDBREAK|DT_NOPREFIX|DT_VCENTER);	
	
	::SetTextColor(hDC,Oldcr);	
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLocalObjectView::CLocalObjectView()
{
	m_UseSystemScrollbar = false;
	m_crViewBkg = RGB(255,255,255);

	m_ObjectIDCount = 0;

	HICON	hIcon = ::LoadIcon(GetHinstance(),MAKEINTRESOURCE(IDI_LOGIC));
	CLocalObjectView::CObjectLabel* Lable = new CLocalObjectView::CObjectLabel(0,_T("Object"),hIcon);
	Lable->m_State = SPACE_NOTOOLABR;
	PushChild(Lable);

//	HICON	hIcon = ::LoadIcon(GetHinstance(),MAKEINTRESOURCE(IDI_LOGIC));

	ButtonItem* Bnt1 = new ButtonItem(ID_DEL_OBJECT,_T("Del "),NULL,false,NULL);
	m_Toolbar.PushChild(Bnt1);
	ButtonItem* Bnt2 = new ButtonItem(ID_CLR_OBJECT,_T("Clear "),NULL,false,NULL);
	m_Toolbar.PushChild(Bnt2);
	
}

CLocalObjectView::~CLocalObjectView()
{

}

void CLocalObjectView::Layout(bool Redraw /* =true */){
    
    CWSTreeView::Layout(false);   
	m_Toolbar.Layout();
	if(Redraw){
		Invalidate();
	}
};	
void CLocalObjectView::Draw(HDC hDC, ePipeline* Pipe){
	CWSTreeView::Draw(hDC,Pipe);
	if(m_Toolbar.m_Owner){
		m_Toolbar.Draw(hDC);
	}
};

void CLocalObjectView::ObjectProc(ePipeline* ItemData){
    SendChildMessage(GetHwnd(),OBJECT_PROC,(int64)ItemData,0);
}

void CLocalObjectView::ResetLocalObject(ePipeline& List){
    SendChildMessage(GetHwnd(),OBJECT_RESET,(int64)&List,0);
}


LRESULT CLocalObjectView::ChildReaction(SpaceRectionMsg* SRM){
    switch(SRM->Msg)
    {
    case OBJECT_PROC:
		{
           return OnObjectProc(SRM->wParam,SRM->lParam);
		}
		break;
	case OBJECT_RESET:
		{
		   return OnResetLocalObject(SRM->wParam,SRM->lParam);
		}
		break;
    }
    return 0;
};

LRESULT CLocalObjectView::OnObjectProc(WPARAM wParam, LPARAM lParam){

	ePipeline* Pipe = (ePipeline*)wParam;

	int64 Act  = Pipe->PopInt();

	switch(Act){
	case ADD_OBJECT:
		{
			CObjectLabel* ObjectList = (CObjectLabel*)m_ChildList[0];

			ePipeline*  ItemData = (ePipeline*)Pipe->GetData(0);
			CObjectItem* Item = new CObjectItem(*ItemData);

			Item->m_Alias = m_ObjectIDCount++;

			Item->SetAreaSize(300,24); //预设大小
			ObjectList->PushChild(Item);

			ObjectList->m_State |= SPACE_OPEN;

			EnsureVisible(Item,true);
			return 0;
		}
		break;
	case DEL_OBJECT:
		{
			CObjectLabel* ObjectList = (CObjectLabel*)m_ChildList[0];

			int64 ObjectID = Pipe->PopInt();
			deque<CVSpace2*>::iterator it = ObjectList->m_ChildList.begin();
			while (it != ObjectList->m_ChildList.end())
			{
				CObjectItem* Item=(CObjectItem*)*it;
				if (Item->m_ObjectData.m_ID == ObjectID)
				{
					ObjectList->m_ChildList.erase(it);
					if (m_SpaceSelected == Item)
					{
						m_SpaceSelected = NULL;
						m_Toolbar.m_Owner = NULL;
					}
					
					if (m_SpaceFocused == Item)
					{
						m_SpaceFocused = NULL;
					}
					break;
				}
				it++;
			}
		}
		break;
	case CLEAR_ALL:
		{
			CObjectLabel* ObjectList = (CObjectLabel*)m_ChildList[0];

			ObjectList->DeleteAll();
			m_Toolbar.m_Owner = NULL;
			m_SpaceFocused = NULL;
			m_ScrollSelected = NULL;
		}
		break;
	default:
		break;
	}

	Layout(true);

	return 0;
};

LRESULT CLocalObjectView::OnResetLocalObject(WPARAM wParam, LPARAM lParam){
	
	ePipeline* ObjectList = (ePipeline*)wParam;

	CObjectLabel* ObjectLabel = (CObjectLabel*)m_ChildList[0];
	ObjectLabel->DeleteAll();

	m_Toolbar.m_Owner = NULL;
	m_SpaceFocused = NULL;
	m_SpaceSelected = NULL;

	//重置本地物体
	int i=0;
	for (i; i<ObjectList->Size(); i++)
	{
		ePipeline* ItemData = (ePipeline*)ObjectList->GetData(i);
		CObjectItem* Item = new CObjectItem(*ItemData);	
		Item->SetAreaSize(300,24); //预设大小
		ObjectLabel->PushChild(Item);
	}
	
	Layout();

	return 0;
}

LRESULT CLocalObjectView::ToolbarReaction(ButtonItem* Bnt)
{
	switch(Bnt->m_Alias){

	case ID_DEL_OBJECT:
		{					
			ePipeline Msg(TO_BRAIN_MSG::GUI_OBJECT_OPERATE::ID);
			Msg.PushInt(TO_BRAIN_MSG::GUI_OBJECT_OPERATE::DELETE_OBJECT);

			CObjectItem* Item = (CObjectItem*)m_SpaceSelected;
			
			Msg.PushInt(Item->m_Alias);
			ePipeline* ItemData = Item->GetItemData();
			Msg.Push_Directly(ItemData);
			
			GetGUI()->SendMsgToBrainFocuse(Msg);
		}
		break;
	case ID_CLR_OBJECT:
		{		
			ePipeline Msg(TO_BRAIN_MSG::GUI_OBJECT_OPERATE::ID);
			Msg.PushInt(TO_BRAIN_MSG::GUI_OBJECT_OPERATE::CLR_OBJECT);			
			GetGUI()->SendMsgToBrainFocuse(Msg);
		}
		break;
	}
	return 0;
}

LRESULT CLocalObjectView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){

	case WM_RBUTTONDOWN:
		{
			return OnRButtonDown(wParam,lParam);
		}
	case WM_COMMAND:
		{
			return OnCommand(wParam,lParam);
		}
	default:
		return CWSTreeView::Reaction(message,wParam,lParam);
	}
}




LRESULT CLocalObjectView::OnRButtonDown(WPARAM wParam, LPARAM lParam){

	//弹出菜单
	
	/*
	POINT pos;
	::GetCursorPos(&pos);
	
	HMENU hmenu;
	HMENU subMenu;
	
	HINSTANCE hInstance = CWinSpace2::GetHinstance();
	hmenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_TEMPOBJECTMENU));
	if(hmenu == NULL){
		int32 n = ::GetLastError();
		return TRUE;
	}
				
	subMenu = GetSubMenu(hmenu, 0); 
	
	::SetForegroundWindow(GetHwnd()); 
	::TrackPopupMenu(subMenu,0, pos.x, pos.y,0,GetHwnd(),NULL);
	::DestroyMenu(hmenu); 
	::PostMessage(GetHwnd(),WM_NULL, 0, 0);
	*/
	return 0;
}



LRESULT CLocalObjectView::OnCommand(WPARAM wParam, LPARAM lParam){
/*
    if(wParam = ID_SET_GLOBLEOBJECT)
	{
		if (m_SpaceSelected ==NULL) return 0;
		CTitle* Title = (CTitle*)m_ChildList[1];
		if(m_SpaceSelected->m_Parent != Title){
			return 0;
		}

		ePipeline Msg(GUI_SET_GLOBAL);
		CObjectItem* Item = (CObjectItem*)m_SpaceSelected;

		Msg.PushInt(GLOBAL_OBJECT);

		ePipeline* ObjectData = Item->GetItemData();
		Msg.Push_Directly(ObjectData);

	    GetGUI()->SendMsgToBrainFocuse(Msg);

	}
*/
	return 0;
}
