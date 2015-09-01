// LocalLogicView.cpp: implementation of the CLocalLogicView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786) 

#include "GMIS.h"
#include "resource.h"
#include "LogicView.h"
#include "SystemSetting.h"
#include "TextAnalyse.h"
#include "NotifyMsgDef.h"


CLogicView::CLogicLabel::CLogicLabel(int32 TypeID,tstring Name,HICON hIcon){
	m_Alias = TypeID;
	m_Name=Name; 
	m_hIcon = hIcon;
	SetAreaSize(300,20);
};

void CLogicView::CLogicLabel::Draw(HDC hDC, ePipeline* Pipe){
	
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

CLogicView::CRefItem::CRefItem(){ 
	SetAreaSize(300,20);
};

CLogicView::CRefItem::CRefItem(tstring RefName, tstring WhoRef){
	m_RefName   = RefName;
	m_WhoRef = WhoRef;
	SetAreaSize(300,20);
};

void CLogicView::CRefItem::Draw(HDC hDC, ePipeline* Pipe){
	FillRect(hDC,GetArea(),SS.crTaskMassBk);
	if(m_State & SPACE_FOCUSED){
		DrawEdge(hDC,GetArea(),RGB(192,192,255));
	}
	
	COLORREF crOld = SetTextColor(hDC,SS.crBrainViewItemText);
	
	RECT rc = GetArea();
	rc.right  = rc.left + 80;
	rc.bottom = rc.top  + 18;
	rc.left +=2;
	
	//被引用名
	::DrawText(hDC,m_RefName.c_str(),m_RefName.size(),&rc,DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	
	//引用者名
	rc.left = rc.right; rc.right +=100 ;
	::DrawText(hDC,m_WhoRef.c_str(),m_WhoRef.size(),&rc,DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);	
	
	SetTextColor(hDC,crOld);
};



CLogicView::CLogicItem::CLogicItem(ePipeline* ItemData)
{
	m_Alias = AbstractSpace::CreateTimeStamp();

	assert(ItemData);
    
    m_LogicName  = ItemData->PopString();
	m_Depend     = ItemData->PopInt();
    m_ActionType = ItemData->PopInt();
	m_LogicText  = ItemData->PopString();
    m_LogicMemo  = ItemData->PopString();

	m_MemoSize.cx = 0;
	m_MemoSize.cy = 0;
	//SetSize(300,20);


};

CLogicView::CLogicItem::~CLogicItem(){
	
}



//引用或解引用由它的调用者锁定
CLogicView::CRefItem*  CLogicView::CLogicItem::AddRef(tstring RefName,tstring WhoRef){
	CRefItem*  Item = new CRefItem(RefName,WhoRef);
	Item->SetAreaSize(200,18);
	PushChild(Item);
	return Item;
}

CLogicView::CRefItem*  CLogicView::CLogicItem::DelRef(tstring RefName,tstring WhoRef){
	CRefItem*  Item;
	deque<CVSpace2*>::iterator It = m_ChildList.begin();
	while(It<m_ChildList.end()){
		Item = (CRefItem*)(*It);
        if(Item->m_RefName == RefName && Item->m_WhoRef == WhoRef){
			m_ChildList.erase(It);
			return Item;
		}
		It++;
	}
	return NULL;
}

void CLogicView::CLogicItem::Draw(HDC hDC, ePipeline* Pipe){
	
	RECT rc = GetArea();
	if(m_State & SPACE_SELECTED){
        //FillRect(hDC,&rc,RGB(0xe1,0xe6,0xef));
		RECT r = rc;
		r.bottom = rc.top+18;
		FillRectGlass(hDC,&r,RGB(192,192,255));
        DrawEdge(hDC,&rc,RGB(192,192,255));
	}
	else if(m_State & SPACE_FOCUSED){
		RECT r = rc;
		r.bottom = rc.top+18;
		FillRectGlass(hDC,&r,RGB(192,192,255));
        DrawEdge(hDC,&rc,RGB(192,192,255));
	}   
	else {
		FillRect(hDC,&rc,RGB(255,255,255));
	}	
	
	
	COLORREF crOld = SetTextColor(hDC,SS.crBrainViewItemText);
	
	
	//名字
	rc.left += 4;
	rc.right =rc.left+100;
	rc.bottom = rc.top +18;
	::DrawText(hDC,m_LogicName.c_str(),m_LogicName.size(),&rc,DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	
	//状态
	rc.left = rc.right; rc.right +=100 ;
	tstring s;
	if(m_ActionType & COMMON_ACTION)s   = _T("common");
	else if(m_ActionType&OUTER_ACTION)s = _T("outer");
	else if(m_ActionType&INTER_ACTION)s = _T("inter");
	else s = _T("inter*");
	::DrawText(hDC,s.c_str(),s.size(),&rc,DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);	
	
	//引用数
	rc.left = rc.right; rc.right +=100 ;
	
	tstring ss = Format1024(_T("RefNum:%d"),m_ChildList.size());
	::DrawText(hDC,ss.c_str(),ss.size(),&rc,DT_END_ELLIPSIS|DT_LEFT|DT_SINGLELINE|DT_VCENTER);	
	
	rc = GetArea();
	rc.left += 4;
	rc.top += 18;
	rc.bottom -= m_MemoSize.cy;
	::DrawText(hDC,m_LogicText.c_str(),m_LogicText.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_NOPREFIX|DT_VCENTER);
	
	if(m_LogicMemo.size()){
        rc.top = rc.bottom;
		rc.bottom = m_AreaBottom;
		::DrawText(hDC,m_LogicMemo.c_str(),m_LogicMemo.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_NOPREFIX|DT_VCENTER);
	}
	
	SetTextColor(hDC,crOld);	
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogicView::CLogicView()
{
	m_UseSystemScrollbar = false;
	m_crViewBkg = RGB(255,255,255);
    //m_State &= ~SPACE_SHOWWINBORDER;
	
	HICON	hIcon = ::LoadIcon(GetHinstance(),MAKEINTRESOURCE(IDI_LOGIC));
	CLogicView::CLogicLabel* Lable = new CLogicView::CLogicLabel(0,_T("Logic"),hIcon);
	Lable->m_State = SPACE_NOTOOLABR;
	PushChild(Lable);


	ButtonItem*  DelBnt = new ButtonItem(CM_DELETE_LOGIC,_T("Delete"),NULL,false,NULL);
	m_Toolbar.PushChild(DelBnt);

	ButtonItem*  ClearBnt = new ButtonItem(CM_CLEAR_LOGIC,_T("Clear"),NULL,false,NULL);
	m_Toolbar.PushChild(ClearBnt);


}

CLogicView::~CLogicView()
{

}


void CLogicView::Layout(bool Redraw /* =true */){
    
    CWSTreeView::Layout(false);   
	m_Toolbar.Layout();
	if(Redraw){
		Invalidate();
	}
};	
void CLogicView::Draw(HDC hDC, ePipeline* Pipe){
	CWSTreeView::Draw(hDC,Pipe);
	if(m_Toolbar.m_Owner){
		m_Toolbar.Draw(hDC);
	}
};

void CLogicView::LogicElementProc(ePipeline* ItemData){
    SendChildMessage(GetHwnd(),ELEMENT_PROC,(int64)ItemData,0);
}

void CLogicView::ResetLocalElement(ePipeline& List){
    SendChildMessage(GetHwnd(),ELEMENT_RESET,(int64)&List,0);
}


LRESULT CLogicView::ChildReaction(SpaceRectionMsg* SRM){
    switch(SRM->Msg)
    {
    case ELEMENT_PROC:
		{
           return OnElementProc(SRM->wParam,SRM->lParam);
		}
		break;
	case ELEMENT_RESET:
		{
		   return OnResetLocalElement(SRM->wParam,SRM->lParam);
		}
		break;
    }
    return 0;
};

LRESULT CLogicView::OnElementProc(WPARAM wParam, LPARAM lParam){

	ePipeline* Pipe = (ePipeline*)wParam;

	int64 Act  = Pipe->PopInt();


	switch(Act){
	case ADD_LOGIC:
		{
			CLogicLabel* Logic = (CLogicLabel*)m_ChildList[0];
			ePipeline* ItemData = (ePipeline*)Pipe->GetData(0);
			CLogicItem* Item = new CLogicItem(ItemData);
			
			SIZE s1 = CalcuTextSize(Item->m_LogicText.c_str());    	
			SIZE s2 = CalcuTextSize(Item->m_LogicMemo.c_str());
			int w = max(s1.cx,s2.cx);
			w = max(w,320);
			int h = s1.cy+s2.cy+20;
			Item->SetAreaSize(w,h); //预设大小很重要
			Logic->PushChild(Item);
			
			Logic->m_State |= SPACE_OPEN;

			EnsureVisible(Item,true);
			return 0;
		}
		break;
	case DELETE_LOGIC:
		{
			CLogicLabel* Logic = (CLogicLabel*)m_ChildList[0];
			ePipeline* ItemData = (ePipeline*)Pipe->GetData(0);
			CLogicItem Item(ItemData);

			deque<CVSpace2*>::iterator it = Logic->m_ChildList.begin();
			while (it != Logic->m_ChildList.end())
			{
				CLogicItem* Item1=(CLogicItem*)*it;
				if (Item1->m_LogicName == Item.m_LogicName)
				{
					Logic->m_ChildList.erase(it);
					if (m_SpaceSelected == Item1)
					{
						m_SpaceSelected = NULL;
						m_Toolbar.m_Owner = NULL;
					}
					
					if (m_SpaceFocused == Item1)
					{
						m_SpaceFocused = NULL;
					}				
					break;
				}
				it++;
			}

		}
		break;
/*暂时屏蔽显示
	case ADD_CAPA:
		{
			CLogicLable* Capa = (CLogicLable*)Title->m_ChildList[1];
			ePipeline* ItemData = (ePipeline*)Pipe->GetData(0);
			CElementItem* Item = new CElementItem(ItemData);
			
			Item->SetAreaSize(320,20);
			Capa->PushChild(Item);
			EnsureVisible(Item,true);
			return 0;
		}
		break;
	case ADD_INDU:
		{
			CLogicLable* Indu = (CLogicLable*)Title->m_ChildList[2];
			ePipeline* ItemData = (ePipeline*)Pipe->GetData(0);
			CElementItem* Item = new CElementItem(ItemData);
			Item->SetAreaSize(320,20);
			Indu->PushChild(Item);
			EnsureVisible(Item,true);
			return 0;
		}
		break;
*/
	case REF_LOGIC:
        {
			CLogicLabel* Logic = (CLogicLabel*)m_ChildList[0];
			ePipeline* ItemData = (ePipeline*)Pipe->GetData(0);
			int64 bAdd = ItemData->PopInt();
			tstring SrcName = ItemData->PopString();
			tstring RefName = ItemData->PopString();
			tstring WhoRef  = ItemData->PopString();

			
			deque<CVSpace2*>::iterator It = Logic->m_ChildList.begin();
			while(It<Logic->m_ChildList.end()){
				CLogicItem* Item = (CLogicItem*)(*It);
				if(Item->m_LogicName == SrcName){
					if (bAdd)
					{
						Item->AddRef(RefName,WhoRef);
					} 
					else
					{
						CRefItem* Ref = Item->DelRef(RefName,WhoRef);
						if(Ref)delete Ref;
					}
					break;
				}
				It++;
			}	
		}
		break;
/*暂时屏蔽显示
	case REF_CAPA:
        {
			CLogicLable* Capa = (CLogicLable*)Title->m_ChildList[1];
            ePipeline* ItemData = (ePipeline*)Pipe->GetData(0);
			int64 bAdd = ItemData->PopInt64();
			tstring SrcName = ItemData->PopString();
			tstring RefName = ItemData->PopString();
			tstring WhoRef  = ItemData->PopString();

			deque<CVSpace2*>::iterator It = Capa->m_ChildList.begin();
			while(It<Capa->m_ChildList.end()){
				CElementItem* Item = (CElementItem*)(*It);
				if(Item->m_Name == SrcName){
                    if(bAdd){
						Item->AddRef(RefName,WhoRef);
					}else{
						CRefItem* Ref = Item->DelRef(RefName,WhoRef);
						if(Ref)delete Ref;
					}
					break;
				}
				It++;
			}	
		}
		break;
	case REF_INDU:
        {
			CLogicLable* Indu = (CLogicLable*)Title->m_ChildList[2];
			
			ePipeline* ItemData = (ePipeline*)Pipe->GetData(0);
			int64 bAdd = ItemData->PopInt64();
			tstring SrcName = ItemData->PopString();
			tstring RefName = ItemData->PopString();
			tstring WhoRef  = ItemData->PopString();

			deque<CVSpace2*>::iterator It = Indu->m_ChildList.begin();
			while(It<Indu->m_ChildList.end()){
				CElementItem* Item = (CElementItem*)(*It);
				if(Item->m_Name == SrcName){
                    if(bAdd){
						Item->AddRef(RefName,WhoRef);
					}else{
						CRefItem* Ref = Item->DelRef(RefName,WhoRef);
						if(Ref)delete Ref;
					}
					break;
				}
				It++;
			}	
		}
		break;
*/
	case CLEAR_ALL:
		{
			CLogicLabel* Logic = (CLogicLabel*)m_ChildList[0];
			Logic->DeleteAll();
			m_Toolbar.m_Owner = NULL;
			m_SpaceFocused = NULL;
			m_SpaceSelected = NULL;

		}
		break;
	default:
		break;
	}

	Layout(true);

	return 0;
};

LRESULT CLogicView::OnResetLocalElement(WPARAM wParam, LPARAM lParam){
	
	ePipeline* Pipe = (ePipeline*)wParam;

	//重置本地临时逻辑及引用
	ePipeline* LogicData = (ePipeline*)Pipe->GetData(0);

	CLogicLabel* Logic = (CLogicLabel*)m_ChildList[0];
	Logic->DeleteAll();
	
	m_Toolbar.m_Owner = NULL;
	m_SpaceFocused = NULL;

	int i=0,j=0;
	for (i; i<LogicData->Size(); i+=2)
	{
		ePipeline* ItemData = (ePipeline*)LogicData->GetData(i);
		ePipeline* ItemRefList  = (ePipeline*)LogicData->GetData(i+1);
		
		CLogicItem* Item = new CLogicItem(ItemData);
		
		SIZE s1 = CalcuTextSize(Item->m_LogicText.c_str());    	
		SIZE s2 = CalcuTextSize(Item->m_LogicMemo.c_str());
		
		int w = max(s1.cx,s2.cx);
		w = max(w,320);
		int h = s1.cy+s2.cy+20;
		Item->SetAreaSize(w,h); //预设大小很重要
		Logic->PushChild(Item);


		//引用
        for(j=0; j<ItemRefList->Size(); j++){
            ePipeline* ItemRef = (ePipeline*)ItemRefList->GetData(j);

			tstring SrcName = ItemRef->PopString();
			tstring RefName = ItemRef->PopString();
			tstring WhoRef  = ItemRef->PopString();
			
			assert(Item->m_LogicName == SrcName);
			Item->AddRef(RefName,WhoRef);
		}
	}
/*暂时屏蔽显示

	//重置本地capa及引用
	ePipeline* CapaData = (ePipeline*)Pipe->GetData(1);

	CLogicLable* Capa = (CLogicLable*)Title->m_ChildList[1];
	Capa->DeleteAll();
	
	for (i=0; i<CapaData->Size(); i+=2)
	{
		ePipeline* ItemData = (ePipeline*)CapaData->GetData(i);
		ePipeline* ItemRefList  = (ePipeline*)CapaData->GetData(i+1);
		
		CElementItem* Item = new CElementItem(ItemData);
		
		Item->SetAreaSize(320,20);
		Capa->PushChild(Item);
		
		//引用
        for(j=0; j<ItemRefList->Size(); j++){
            ePipeline* ItemRef = (ePipeline*)ItemRefList->GetData(j);
			
			tstring SrcName = ItemRef->PopString();
			tstring RefName = ItemRef->PopString();
			tstring WhoRef  = ItemRef->PopString();
			
			assert(Item->m_Name == SrcName);
			Item->AddRef(RefName,WhoRef);
		}
	}



	//重置本地indu及引用
	ePipeline* InduData = (ePipeline*)Pipe->GetData(2);
	CLogicLable* Indu = (CLogicLable*)Title->m_ChildList[2];
	Indu->DeleteAll();
	
	for (i=0; i<InduData->Size(); i+=2)
	{
		ePipeline* ItemData = (ePipeline*)InduData->GetData(i);
		ePipeline* ItemRefList  = (ePipeline*)InduData->GetData(i+1);
		
		CElementItem* Item = new CElementItem(ItemData);
		
		Item->SetAreaSize(320,20);
		Indu->PushChild(Item);
		
		//引用
        for(j=0; j<ItemRefList->Size(); j++){
            ePipeline* ItemRef = (ePipeline*)ItemRefList->GetData(j);
			
			tstring SrcName = ItemRef->PopString();
			tstring RefName = ItemRef->PopString();
			tstring WhoRef  = ItemRef->PopString();
			
			assert(Item->m_Name == SrcName);
			Item->AddRef(RefName,WhoRef);
		}
	}
*/
	Layout();
	return 0;
}

LRESULT CLogicView::ToolbarReaction(ButtonItem* Bnt){
	switch(Bnt->m_Alias){
 
    case CM_DELETE_LOGIC:
		{
			CLogicItem* Item = (CLogicItem*)m_Toolbar.m_Owner;
			//发信息给大脑，然后大脑生成处理好以后再返回处理结果
			ePipeline Msg(GUI_LOGIC_OPERATE);		
			Msg.PushInt(DEL_LOGIC);
			Msg.PushString(Item->m_LogicName);
			
			GetGUI()->SendMsgToBrainFocuse(Msg);

		}
		break;
    case CM_CLEAR_LOGIC:
		{
			//发信息给大脑，然后大脑生成处理好以后再返回处理结果
			ePipeline Msg(GUI_LOGIC_OPERATE);		
			Msg.PushInt(CLEAR_LOGIC);
			
			GetGUI()->SendMsgToBrainFocuse(Msg);
		}
		break;
	}
	return 0;
}

LRESULT CLogicView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
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


LRESULT CLogicView::OnRButtonDown(WPARAM wParam, LPARAM lParam){

	//弹出菜单
	/*
	POINT pos;
	::GetCursorPos(&pos);
	
	HMENU hmenu;
	HMENU subMenu;
	
	HINSTANCE hInstance = CWinSpace2::GetHinstance();
	hmenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_TEMPLOGICMENU));
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

LRESULT CLogicView::OnCommand(WPARAM wParam, LPARAM lParam){


	return 0;
}
