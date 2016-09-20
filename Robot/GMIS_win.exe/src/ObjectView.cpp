// BrainSpaceView.cpp: implementation of the CBrainObjectView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4786)
#pragma warning (disable: 4244)
#include "ObjectView.h"
#include "BrainMemory.h"
#include "SystemSetting.h"

ObjectItem::ObjectItem(int64 ID,tstring& Name,SPACETYPE Type, tstring& Fingerprint,HICON hIcon)
		:CVSpace2(ID),m_Name(Name),m_Type(Type),m_Fingerprint(Fingerprint),m_hIcon(hIcon)
{	
	SetAreaSize(350,22);
};

void ObjectItem::Draw(HDC hDC, ePipeline* Pipe){
									
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
	::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
		DT_WORDBREAK|DT_NOPREFIX|DT_VCENTER);	

	::SetTextColor(hDC,Oldcr);	
};
	
ObjectHeader::ObjectHeader(){
	m_Alias =0;
	m_Name = _T("NAME");
	m_Type = LOCAL_SPACE;
	m_hIcon = NULL;
	m_Title = _T("Space unconnected!");
	SetAreaSize(350,42);
	m_crText = SS.crBrainViewItemText;
};

void ObjectHeader::Draw(HDC hDC, ePipeline* Pipe){
	//输出内容
	RECT rc = GetArea();
	 
	COLORREF Oldcr = ::SetTextColor(hDC,m_crText);
	
	rc.right  = rc.left + 22;
	rc.bottom = rc.top + 22;
    DeflateRect(&rc,4,4,4,4);
	if(m_State & SPACE_DISABLE)
		FillRect(hDC,&rc,RGB(128,0,0));
	else 
        FillRect(hDC,&rc,RGB(255,0,0));
	
	rc.left = rc.right+5;
	rc.right = m_AreaRight-10;
	//rc.bottom = rc.top+22;
	::DrawText(hDC,m_Title.c_str(),m_Title.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
		DT_WORDBREAK|DT_NOPREFIX|DT_VCENTER);	

	rc.top = m_AreaTop + 22;
	rc.bottom = m_AreaBottom-4;

	//名字
    rc.right = rc.left+170;
	::DrawText(hDC,m_Name.c_str(),m_Name.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
		DT_WORDBREAK|DT_NOPREFIX|DT_VCENTER);	

	//给header添加一根横线
	rc = GetArea();
	rc.top = rc.bottom-4;
	rc.bottom = rc.bottom-2;
	FillRect(hDC,&rc,m_crText);	

	::SetTextColor(hDC,Oldcr);	
};


//CObjectView
////////////////////////////////////////////////////////////////////

CObjectView::CObjectView(){
	
    m_crWinBorder = RGB(32,32,32);

	m_ToobarItemSelected = NULL;

	ObjectHeader*   Header = new ObjectHeader;
	Header->m_State |= SPACE_DISABLE;
	m_ChildList.push_back(Header);

    ButtonItem* Delete = new ButtonItem(OBJECT_DEL,_T("Delete Object"),NULL,false);
	m_Toolbar.PushChild(Delete);
};
 
void  CObjectView::Layout(bool Redraw){
	CWSListView::Layout(false);
	CVSpace2* Header = m_ChildList.front();
	if(m_HotSpaceList.size()>0){
		CVSpace2* First  = m_HotSpaceList.front();
		Header->SetArea(First);
		if(Header != First && Redraw)Invalidate();
	}
	m_Toolbar.Layout();
	if(Redraw)Invalidate();
}

void CObjectView::Draw(HDC hDC, ePipeline* Pipe){
	CWSListView::Draw(hDC,Pipe);
	if(m_Toolbar.m_Owner)m_Toolbar.Draw(hDC,Pipe);
}

void CObjectView::DrawItemHeader( HDC hDC, CVSpace2* Space){   		 
	if(Space->m_Alias== 0)return;		
	RECT rcHeader = GetHeaderBox(Space);
    ObjectItem*  Item = (ObjectItem*)Space;
	TCHAR buf[100];
	_stprintf(buf,_T("%3d"),(uint32)Item->m_Index);
	RECT rc;

	COLORREF Oldcr = ::SetTextColor(hDC,SS.crBrainViewItemText);
	::SetRect(&rc,0,rcHeader.top,rcHeader.right-2,rcHeader.bottom);
	::DrawText(hDC,buf,_tcslen(buf),&rc,DT_CENTER|DT_VCENTER|DT_NOPREFIX);
    ::SetTextColor(hDC,Oldcr);
}

RECT CObjectView::GetHeaderBox(CVSpace2* Space){
	RECT rc = Space->GetArea();
	rc.left -= m_HeaderWidth+4;
	rc.right = rc.left + m_HeaderWidth;
	return rc;
}


LRESULT CObjectView::OnAddObject(int64 wParam, int64 lParam){
	ObjectItem* Item = (ObjectItem*)wParam;
	Item->m_Index = m_ChildList.size();
	PushChild(Item);
				
	ObjectHeader* Header = (ObjectHeader*)m_ChildList.front();
	tstring title = Format1024(_T("%s:     %d "),m_Name.c_str(),m_ChildList.size()-1);
	Header->m_Title = title;
	
	Layout(true);
	return 0;
}
	
LRESULT CObjectView::OnDeleteObject(int64 wParam, int64 lParam){
	tstring Fingerprint = *(tstring*)wParam;
	deque<CVSpace2*>::iterator It = m_ChildList.begin();
	It++;  //第一个是Header
	LRESULT ret = 0;
	while(It < m_ChildList.end()){
		ObjectItem* Item = (ObjectItem*)(*It);
		if(Item->m_Fingerprint == Fingerprint){               
			delete Item;
			Item = NULL;
			It = m_ChildList.erase(It);
			//重新设置序号
			while(It < m_ChildList.end()){
				Item = (ObjectItem*)(*It);
				Item->m_Index --;
				It++;
			}
			ret = 1;
			break;
		}
		It++;
	}
	tstring title = Format1024(_T("%s:     %d Object"),m_Name.c_str(),m_ChildList.size()-1);
	ObjectHeader* Header = (ObjectHeader*)m_ChildList.front();
	Header->m_Title = title;
	m_Toolbar.m_Owner = NULL;

	Layout(true);
	return ret;
}

LRESULT CObjectView::OnClearAll(int64 wParam, int64 lParam){
	ObjectHeader* Header = (ObjectHeader*)m_ChildList.front();
	Header->m_Title = _T("Invalid");
	Header->m_State |= SPACE_DISABLE;
	m_ChildList.erase(m_ChildList.begin());
	DeleteAll();
	m_HotSpaceList.clear();
	m_ChildList.push_back(Header);
	m_Toolbar.m_Owner = NULL;
	Layout(true);

	
	return 0;
}

LRESULT CObjectView::OnFindObject(int64 wParam, int64 lParam){
	tstring& Name = *(tstring*)wParam;
	vector<ObjectItem>& SpaceList = *(vector<ObjectItem>*)lParam;

    deque<CVSpace2*>::iterator It = m_ChildList.begin();
	while (It != m_ChildList.end())
	{
	    ObjectItem* Item = (ObjectItem*)(*It);
		if(_tcsicmp(Item->m_Name.c_str(),Name.c_str())==0){
			SpaceList.push_back(*Item);
		}
		It++;
	}
	return 0;
}
	
LRESULT CObjectView::OnSetTitle(int64 wParam, int64 lParam)
{
	const TCHAR* Text = (const TCHAR*)wParam;
	BOOL Enable = lParam;
    ObjectHeader* Header = (ObjectHeader*)m_ChildList.front();
	Header->m_Title = Text;
	if(Enable)Header->m_State &= ~SPACE_DISABLE;
	else Header->m_State |= SPACE_DISABLE;
	Invalidate();
	return 0;
}
	
LRESULT CObjectView::ToolbarReaction(ButtonItem* Bnt){
	switch(Bnt->m_Alias)
	{
		case OBJECT_DEL:
		{
			ObjectItem* Item = (ObjectItem*)m_SpaceSelected;
			RemoveChild(Item);
			Layout();
		}
 	}
	return 0;
}


LRESULT CObjectView::ChildReaction(SpaceRectionMsg* srm){
	switch(srm->Msg)
	{
	case OBJECT_ADD:
        OnAddObject(srm->wParam,srm->lParam);
		break;
	case OBJECT_DEL:
		OnDeleteObject(srm->wParam,srm->lParam);
		break;
	case OBJECT_FIND:
		OnFindObject(srm->wParam,srm->lParam);
	case OBJECT_CLR:
		OnClearAll(srm->wParam,srm->lParam);
	    break;
	case OBJECT_SETTITLE:
		OnSetTitle(srm->wParam,srm->lParam);
	}
	return 0;
}



void CObjectView::ConnectSpaceFail(tstring Reason){
	ClearAllObject();
	SetHeaderText(Reason,FALSE);
}

void CObjectView::SetHeaderText(tstring Title,BOOL Enable){
	if(GetHwnd())
		SendChildMessage(GetHwnd(),OBJECT_SETTITLE,(int64)Title.c_str(),(int64)Enable);
	else
		OnSetTitle((int64)Title.c_str(),(int64)Enable);
}

void CObjectView::AddObject(int64 ID,tstring& Name,SPACETYPE Type,tstring& Fingerprint,HICON hIcon){
  
	ObjectItem* Item = new ObjectItem(ID,Name,Type,Fingerprint,hIcon);
	if(GetHwnd())SendChildMessage(GetHwnd(),OBJECT_ADD,(int64)Item,0);
	else OnAddObject((int64)Item,0);
};
    
int32 CObjectView::FindObject(tstring& Name, vector<ObjectItem>& SpaceList){
	if(GetHwnd()){
		SendChildMessage(GetHwnd(),OBJECT_FIND,(int64)&Name,(int64)&SpaceList);
	}else{
	    OnFindObject((int64)&Name,(int64)&SpaceList);
	}
	return SpaceList.size();
}

void CObjectView::DeleteObject(int64 ID){
	if(GetHwnd()){
		SendChildMessage(GetHwnd(),OBJECT_DEL,ID,0);
	}else{
		OnDeleteObject(ID,0);
	}
};
 
void CObjectView::ClearAllObject(){
	if(GetHwnd()){
		SendChildMessage(GetHwnd(),OBJECT_CLR,0,0);	
	}else{
		OnClearAll(0,0);	
	}
};

