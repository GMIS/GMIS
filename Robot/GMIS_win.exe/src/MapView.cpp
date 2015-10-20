// BrainMapView.cpp: implementation of the CBrainMapView class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "GMIS.h"
#include "MapView.h"
#include "MainFrame.h"
#include "SystemSetting.h"
#include "Ipclass.h"
#include "commctrl.h"
#include "resource.h"
#include "WorldShow.h"
#include "Win32Tool.h"
#include "MainBrain.h"
#include "LogicDialog.h"

HICON MapItem::hWorld     = NULL; 
HICON MapItem::hContainer = NULL;
HICON MapItem::hPeople    = NULL;
HICON MapItem::hRoom      = NULL;
	
BOOL CALLBACK GetIPDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

MapItem::MapItem(int64 ID,const TCHAR* Name,HICON hIcon)
		:CTreeItem(ID,Name),m_hIcon(hIcon)
{

	SetAreaSize(100,18);
};
void MapItem::Draw(HDC hDC, ePipeline* Pipe){
	tstring Text = m_Text;								
	//输出内容
	RECT rc = GetArea();
	COLORREF crText = SS.crBrainViewItemText;;
    if(m_State & SPACE_SELECTED){
		crText = RGB(255,255,255);
        FillRect(hDC,&rc,SS.crBrainViewItemBorder);
	}else if(m_State & SPACE_FOCUSED){
		crText = RGB(255,255,255);
        FillRect(hDC,&rc,RGB(0,128,0));
		if(m_State & SPACE_DISABLE)Text = _T("Invalid");
	}
	if(m_hIcon){	
		::DrawIconEx(hDC, rc.left,rc.top, m_hIcon, RectHeight(rc), RectHeight(rc), 0, 0, DI_NORMAL);
	}
	rc.right = rc.left+ RectHeight(rc);

	rc.left = rc.right;
	rc.right = m_AreaRight;


	COLORREF Oldcr = ::SetTextColor(hDC,crText);
	::DrawText(hDC,Text.c_str(),Text.size(),&rc,DT_LEFT|DT_EXPANDTABS|DT_END_ELLIPSIS|
		DT_WORDBREAK|DT_NOPREFIX|DT_VCENTER);		
	::SetTextColor(hDC,Oldcr);
}

WorldStart::WorldStart(int64 ID,const TCHAR* Name,HICON hIcon)
	:MapItem(ID,Name,hIcon),m_IPAddress(NULL),m_GotoBnt(BN_GOTO_SPACE,_T("Goto")){
};
WorldStart::~WorldStart()
{

}

CVSpace2*  WorldStart::HitTest(int32 x, int32 y){
	if(m_GotoBnt.HitTest(x,y)==&m_GotoBnt)return &m_GotoBnt;
	return CVSpace2::HitTest(x,y);
}

void WorldStart::Layout(bool Redraw  /*=true*/){
	RECT rc = GetArea();
	rc.left = rc.right-40;
	m_GotoBnt.SetArea(rc);

	rc.right = rc.left-8;
	rc.left  = rc.right-120;
	rc.top +=1;
	rc.bottom -=1;
	if(m_IPAddress){
		::MoveWindow(m_IPAddress,rc.left,rc.top,RectWidth(rc),RectHeight(rc),TRUE);
	}	
}

void WorldStart::Draw(HDC hDC, ePipeline* Pipe){
	tstring Text = m_Text;

	RECT rc = GetArea();;
	FillRectGlass(hDC,&rc,RGB(145,150,155),FALSE,80);
	
	if(m_hIcon){
		int h = (RectHeight(rc)-16)/2;
		::DrawIconEx(hDC, rc.left+h,rc.top+h, m_hIcon, 16, 16, 0, 0, DI_NORMAL);
	}
	rc.left = rc.left+ RectHeight(rc)+4;
	rc.right = rc.left+100;
  
	COLORREF Oldcr = ::SetTextColor(hDC,RGB(0,0,0));
	::DrawText(hDC,Text.c_str(),Text.size(),&rc,DT_LEFT|DT_SINGLELINE|DT_VCENTER);		
	::SetTextColor(hDC,Oldcr);		

	m_GotoBnt.Draw(hDC);
}

//MAP VIEW
CMapView::CMapView(){
	m_Outport = NULL;
	m_OldFindItem = NULL;
	m_State &= ~SPACE_SHOWWINBORDER;

	m_Padding = 8;

//	m_bShowBorder = true;
	m_crViewBkg =  RGB(245,245,245);
	m_crWinBorder = SS.crBrainViewBorder;

	m_crText           = RGB(192,192,192);
	SetLineColor(RGB(128,128,128));

	MapItem::hWorld     = ::LoadIcon(GetHinstance(),MAKEINTRESOURCE(IDI_EARTH)); 
	MapItem::hContainer = ::LoadIcon(GetHinstance(),MAKEINTRESOURCE(IDI_CONTAINER)); 
	MapItem::hPeople    = ::LoadIcon(GetHinstance(),MAKEINTRESOURCE(IDI_PEOPLE)); 
	MapItem::hRoom     =  ::LoadIcon(GetHinstance(),MAKEINTRESOURCE(IDI_ROOM)); 

	WorldStart* Item = new WorldStart(ROOT_SPACE,_T("World"),MapItem::hWorld);
	Item->SetAreaSize(250,22);
//	Item->m_State |= SPACE_DISABLE;
    m_ChildList.push_back(Item);
	MapItem* Outer = new MapItem(OUTER_SPACEID,_T("Outer Space"),MapItem::hRoom); //加入虚节点,以显示还有
	Item->PushChild(Outer);
	Outer->m_State |= SPACE_DISABLE;
	MapItem* Local = new MapItem(LOCAL_SPACEID,_T("Local Space(Invalid)"),MapItem::hRoom); //加入虚节点,以显示还有
	Local->SetAreaSize(180,18);
	Item->PushChild(Local);
}

void  CMapView::GetSpacePath(ePipeline& Path){
    if(m_SpaceSelected == NULL)return;
	MapItem* Map = (MapItem*)m_SpaceSelected;
	Path.SetID(Map->m_Alias);

	deque<tstring> List;
	while(Map->m_Parent)
	{
		tstring& s = Map->m_Text;
        List.push_front(s);
		Map = (MapItem*)Map->m_Parent;
	};
	
	//反转
	deque<tstring>::reverse_iterator it = List.rbegin();
	while (it != List.rend())
	{
		tstring& s = *it;
		Path.PushString(s);
		it++;
	}
    return;
}

MapItem* CMapView::Path2Item(ePipeline& Path)
{
	MapItem* ParentItem = (MapItem*)FindSpace(LOCAL_SPACEID);
	if(ParentItem==NULL){
		return 0;
	}
	
	while(Path.Size())
	{
		tstring Name = Path.PopString();
		int i;
		for(i=0; i<ParentItem->m_ChildList.size(); i++){
			MapItem* ChildItem = (MapItem*)ParentItem->m_ChildList[i];
			if (ChildItem->m_Text == Name)
			{
				ParentItem = ChildItem;
				break;
			}
		}
		if (i==ParentItem->m_ChildList.size())
		{
			ParentItem = NULL;
			break;  //没找到
		}
	}
	
    return ParentItem;
}
void CMapView::InitLocalRoom(tstring LocalName){
	assert(m_ChildList.size()==1);
	WorldStart* World = (WorldStart*)m_ChildList.front();
	assert(World->m_ChildList.size()==2);
	MapItem* Local = (MapItem*)World->m_ChildList.back();
	Local->m_Text = LocalName;
	Local->m_State = SPACE_SELECTED;
	m_SpaceSelected = Local;
	World->m_State &= ~SPACE_DISABLE;
	World->m_State |= SPACE_OPEN;
	Invalidate();
}
void CMapView::EnableNewConnect(BOOL bEnable){
	WorldStart* World = (WorldStart*)m_ChildList.front();
    ::EnableWindow(World->m_IPAddress,bEnable);
}

void CMapView::SetOuterRoomName(tstring Name){
	assert(m_ChildList.size()==1);
	WorldStart* World = (WorldStart*)m_ChildList.front();
	assert(World->m_ChildList.size()==2);
	MapItem* Outer = (MapItem*)World->m_ChildList.front();
	Outer->m_Text = Name;
}
	
void CMapView::SetLocalRoomName(tstring Name){
	assert(m_ChildList.size()==1);
	WorldStart* World = (WorldStart*)m_ChildList.front();
	assert(World->m_ChildList.size()==2);
	MapItem* Local = (MapItem*)World->m_ChildList.back();
	Local->m_Text = Name;
}

void CMapView::Layout(bool Redraw  /*=true*/){
	CWSTreeView::Layout(false);
    WorldStart* World = (WorldStart*)m_ChildList.front();
	World->Layout(false);
    Invalidate();
}

LRESULT CMapView::OnReset(){
	WorldStart* World = (WorldStart*)m_ChildList.front();
	World->m_State |= SPACE_DISABLE;
    GetGUI()->m_WorldShow.Reset(LOCAL_SPACEID,1,_T("Outer World"),OUTER_SPACE);
	OnClearSpace(0,0);
	m_SpaceSelected = NULL;
	return 0;
}

LRESULT CMapView::OnAddSpace(int64 wParam, int64 lParam){
	MapItem* Item = (MapItem*)wParam;
	int64 ParentID = lParam;
	MapItem* ParentItem = (MapItem*)OnFind(ParentID,0);
	if(ParentItem == NULL)return -1;
	
	//首先删除可能有的虚节点
	if(ParentItem->m_ChildList.size()==1){
         if(ParentItem->m_ChildList.front()->m_Alias == -1) 
			 ParentItem->DeleteAll();
	}
	
    ParentItem->PushChild(Item);
	
	//如果childItem是room或Container则加入虚节点
	//MapItem* DummyChild = new MapItem(-1,"",0); //加入虚节点,以显示还有
	//Item->PushChild(DummyChild);	
	Layout(true);
	return 0;
}
	
LRESULT CMapView::OnDeleteSpace(int64 wParam, int64 lParam){
	ePipeline* Path = (ePipeline*)wParam;
    tstring Name  = *(tstring*)lParam;

	MapItem* ParentItem = Path2Item(*Path);
	if(ParentItem==NULL){
		return 0;
	}

	for(int i=0; i<ParentItem->m_ChildList.size(); i++){
		MapItem* ChildSpace = (MapItem*)ParentItem->m_ChildList[i];
		if(ChildSpace->m_Text == Name){
			ParentItem->m_ChildList.erase(ParentItem->m_ChildList.begin()+i);
			Layout();
			return 0;
		}
		
	}
	return 0;
}

LRESULT CMapView::OnClearSpace(int64 wParam, int64 lParam){
	WorldStart* World = (WorldStart*)m_ChildList.front();
	MapItem* Local = (MapItem*)World->m_ChildList.back();
	Local->DeleteAll();
	Local->m_Text = _T("Local Space(Invalid)");
	Local->SetAreaSize(180,18);
	Layout(true);
	return 0;
};

LRESULT CMapView::OnFind(int64 wParam, int64 lParam){
	int64 ID = wParam;
	if(m_OldFindItem && m_OldFindItem->m_Alias == ID)return (LRESULT)m_OldFindItem;
	CVSpace2* Space = FindSpace(ID);
	if(Space==NULL)return NULL;
	m_OldFindItem  =  (MapItem*)(Space);
	return (LRESULT)m_OldFindItem;
};

LRESULT CMapView::OnLButtonDown(WPARAM wParam, LPARAM lParam){
	LRESULT ret = CWSScrollView::OnLButtonDown(wParam,lParam);
 	
    POINTS* p = (POINTS*)(&lParam);
	POINT point;
	point.x = p->x;
	point.y = p->y;
	if(m_ScrollSelected){
		//if(m_SpaceSelected)m_SpaceSelected->m_State &= ~SPACE_SELECTED;
		//m_SpaceSelected = NULL; //点击了Scrollbar不能再点击其它
		return 0;
	}

	CVSpace2* SpaceSelected = Hit(point);

	if(SpaceSelected){
		
		//点击发生在treebox?
		RECT TreeBox = GetHeaderBox(SpaceSelected); 
		POINT point;
		point.x = p->x;
		point.y = p->y;
		if(::PtInRect(&TreeBox,point)){
			ToggleSpace(SpaceSelected);
			return 0;
		}

	
		if ( SpaceSelected->m_Alias == OUTER_SPACEID)
		{   //目前不处理
			return 0;
		}else if (SpaceSelected->m_Alias == BN_GOTO_SPACE)
		{

				WorldStart* World = (WorldStart*)m_ChildList.front();
				IP ip;
				DWORD dwAddress=0;
				::SendMessage(World->m_IPAddress,IPM_GETADDRESS,0,(LPARAM)&dwAddress);
				
				if(dwAddress>0 ){
					ip.seg1 = FIRST_IPADDRESS(dwAddress);
					ip.seg2 = SECOND_IPADDRESS(dwAddress);
					ip.seg3 = THIRD_IPADDRESS(dwAddress);
					ip.seg4 = FOURTH_IPADDRESS(dwAddress);

					ePipeline  Msg(GUI_CONNECT_TO);
                    Msg.PushString(ip.Get());
					Msg.PushInt(SPACE_PORT);

					int64 EventID = AbstractSpace::CreateTimeStamp();
					GetGUI()->SendMsgToBrain(SYSTEM_SOURCE,DEFAULT_DIALOG,Msg,EventID);
				}

			 	return 0;
		}else if (SpaceSelected->m_Parent) //其他任意子条目
		{

			if(SpaceSelected == m_SpaceSelected)return 0;

			if(m_SpaceSelected){
				m_SpaceSelected->m_State &= ~SPACE_SELECTED;
			}
			SpaceSelected->m_State |= SPACE_SELECTED;
			m_SpaceSelected = SpaceSelected;
			
			CLinker World;
			GetBrain()->GetLinker(SPACE_SOURCE,World);
			if(World.IsValid()){
				MapItem* Item = (MapItem*)SpaceSelected;
				MapItem* ParentRoom = (MapItem*)Item->m_Parent;
                assert(ParentRoom);
				GetGUI()->m_WorldShow.Reset(Item->m_Alias,ParentRoom->m_Alias,ParentRoom->m_Text,LOCAL_SPACE);

				ePipeline Path;
				GetGUI()->GetSpacePath(Path);
				CMsg Msg(MSG_ROBOT_GOTO_SPACE,NULL,NULL);
				ePipeline& Letter = Msg.GetLetter();
                Letter.PushPipe(Path);
				World().PushMsgToSend(Msg);
			}else{
				OnReset();
				tstring Title = _T("Space Unconnected");
				GetGUI()->m_WorldShow.ConnectRoomFail(Title);
			}

		}
	}	
	Invalidate();
	return 0;
}

LRESULT CMapView::ChildReaction(SpaceRectionMsg* srm){
    switch(srm->Msg)
    {
    case MAP_ADD:
		return OnAddSpace(srm->wParam,srm->lParam);
    case MAP_DEL:
		return OnDeleteSpace(srm->wParam,srm->lParam);
    case MAP_CLR:
		return OnClearSpace(srm->wParam,srm->lParam);
	case MAP_RESET:
		return OnReset();
	case MAP_FIND:
		return OnFind(srm->wParam,srm->lParam);
    }
	return 0;
}

LRESULT CMapView::OnCreate(WPARAM wParam, LPARAM lParam){
	WorldStart* World = (WorldStart*)m_ChildList.front();
    World->m_IPAddress = CreateWindowEx(WS_EX_CLIENTEDGE,WC_IPADDRESS,NULL,   
					WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP,   
					0,0,120,18,GetHwnd(),(HMENU)0,   
					GetHinstance(),NULL) ;  
	SendMessage(World->m_IPAddress,IPM_SETADDRESS,0,MAKEIPADDRESS(127,0,0,1));
	return 0;
}

///////////////////////////////////////////////
LRESULT CMapView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if(message == WM_LBUTTONDOWN){
		return OnLButtonDown(wParam,lParam);
	}else if (message == WM_CREATE)
	{
		return OnCreate(wParam,lParam);
	}
    else return CWSTreeView::Reaction(message,wParam,lParam);
};

	

void CMapView::SetCurRoom(int64 ChildID){
	MapItem* Item = (MapItem*)OnFind(ChildID,0);
	if(Item==NULL)return;
    if(m_SpaceSelected){
		m_SpaceSelected->m_State &= ~SPACE_SELECTED;
	}
	Item->m_State |= SPACE_SELECTED;
	m_SpaceSelected = Item;
   
	CVSpace2* Parent = Item->m_Parent;
	while (Parent)
	{
	    Parent->m_State |= SPACE_OPEN;
        Parent = Parent->m_Parent;
		if(Parent && Parent->m_State & SPACE_OPEN)break;
	}
	Layout();
}

MapItem* CTitleMapView::AddItem(int64 ParentID,int64 ChildID,tstring Name,HICON Type){
	MapItem* Item = new MapItem(ChildID,Name.c_str(),Type);
	LRESULT ret;
	if (GetHwnd())
	{
		ret = SendChildMessage(m_MapView.GetHwnd(),MAP_ADD,(int64)Item,ParentID);
	}else {
		ret = m_MapView.OnAddSpace((WPARAM)Item,ParentID);
	}
	if(ret == -1){
		delete Item;
		Item = NULL;
		return NULL;
	}
	return Item;
}
bool CTitleMapView::DeleteItem(ePipeline* Path,tstring& Name){
	LRESULT ret;
	if (GetHwnd())
	{
		ret = SendChildMessage(m_MapView.GetHwnd(),MAP_DEL,(int64)Path,(int64)&Name);
	} 
	else
	{
		ret = m_MapView.OnDeleteSpace((int64)Path,(int64)&Name);
	}
	if(ret ==-1)return false;
	return true;
};

MapItem* CTitleMapView::FindChildItem(int64 ID){
	MapItem* Item = NULL;
	if(GetHwnd()) //有时候不执行就返回NULL
		Item = (MapItem*)SendChildMessage(m_MapView.GetHwnd(),MAP_FIND,ID,0);	
	else 
		Item = (MapItem*)m_MapView.OnFind(ID,0);	
	return Item;
};
void CTitleMapView::ClearAllItem(){
	if(GetHwnd())SendChildMessage(m_MapView.GetHwnd(),MAP_CLR,0,0);	
	else m_MapView.OnClearSpace(0,0);
};
void CTitleMapView::Reset(){
	if (GetHwnd())SendChildMessage(m_MapView.GetHwnd(),MAP_RESET,0,0);
	else m_MapView.OnReset();
};

void CTitleMapView::SetCurRoom(int64 ChildID){
	m_MapView.SetCurRoom(ChildID);
};

void CTitleMapView::Layout(bool Redraw /* = true */){
	CTitleView::Layout(Redraw);
	if(m_Created)
	::MoveWindow(m_MapView.GetHwnd(),m_rcClient.left+1,m_rcClient.top,RectWidth(m_rcClient)-2,RectHeight(m_rcClient)-1,TRUE);
}

LRESULT CTitleMapView::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	if(message == WM_CREATE){
	   return OnCreate(wParam,lParam);
	}else if (message == WM_LBUTTONDOWN){
		return OnLButtonDown(wParam,lParam);
	}
    else{
		return CTitleView::Reaction(message,wParam,lParam);
	}
}


LRESULT CTitleMapView::OnCreate(WPARAM wParam, LPARAM lParam){
	
	RECT rc;
	::SetRect(&rc,0,0,0,0);
	if(!m_MapView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE,rc,GetHwnd())){
		return -1;
	}

	m_Created = true;
	return 0;
}
LRESULT CTitleMapView::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
{
    POINTS* p = (POINTS*)(&lParam);

    CVSpace2 * Space = NcHit(p->x,p->y);	
	if(Space){
		int64 ID = Space->m_Alias;
        switch(ID){
			
		case ID_LEFTSIDE:
			::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTBOTTOMLEFT,lParam);
			break;		
		case ID_RIGHTSIDE:
			::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTBOTTOMRIGHT,lParam);
			break;	
		case ID_BOTTOMSIDE:
			::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTBOTTOM,lParam);
			break;		
		case ID_CLOSE:
			{
				SendParentMessage(CM_CLOSEVIEW,m_Alias,0,NULL);
			}
			break;
		case ID_TITLETEXT: //TextLabel
			::SendMessage(m_hWnd,WM_NCLBUTTONDOWN,HTCAPTION,lParam);
			break;
		}
	}
    return 0;
}

BOOL CALLBACK GetIPDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	
	switch (message)
	{
		case WM_INITDIALOG:
			{
             CenterWindow(hDlg,GetParent(hDlg));
			::SetFocus(GetDlgItem(hDlg,IDC_NEWIP));
			return TRUE;
			}
		case WM_COMMAND:

			if (LOWORD(wParam) == IDOK ) 
			{
				IP ip;
				HWND hAdd = GetDlgItem(hDlg,IDC_NEWIP);
				DWORD dwAddress=0;
				::SendMessage(hAdd,IPM_GETADDRESS,0,(LPARAM)&dwAddress);

				if(dwAddress>0 ){
					ip.seg1 = FIRST_IPADDRESS(dwAddress);
					ip.seg2 = SECOND_IPADDRESS(dwAddress);
					ip.seg3 = THIRD_IPADDRESS(dwAddress);
					ip.seg4 = FOURTH_IPADDRESS(dwAddress);


					CMainFrame* Brain = GetGUI();
				//	_ACCOUNT ac = GetAccount();
				//	if(ac.ip == ip && Brain->IsConnectSuperior())return FALSE; 
				//	Brain->Connect(ip.Get().c_str(),SYSTEM_PORT,5);

					EndDialog(hDlg, 1);
				}

				return TRUE;
			}else if(LOWORD(wParam) == IDCANCEL){
                ::EndDialog(hDlg, 0);
				return TRUE;
			}
		
	}
    return FALSE;
};

