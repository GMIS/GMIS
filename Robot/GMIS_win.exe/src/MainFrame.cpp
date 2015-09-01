// MainFrame.cpp: implementation of the CMainFrame class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "GMIS.h"
#include "MainBrain.h"
#include "MainFrame.h"
#include "TaskDialog.h"
#include "UserTimer.h"
#include "SystemSetting.h"
#include "Win32Tool.h"
#include "UseShGetFileInfo.h"
#include "Ipclass.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainFrame::CMainFrame()
:m_Splitter(this,4,20,true),m_SpaceSelected(NULL),
m_bShowWorld(FALSE),m_MsgList(&m_Mutex),m_MsgProcObject(this)
{
	m_Created = FALSE;

    PushChild(&m_Splitter);	
	//m_crViewBkg = RGB(255,0,0);
	m_State = SPACE_SHOWWINBORDER;

	m_bOpenPerformanceView = FALSE;

}

CMainFrame::~CMainFrame()
{
	m_ChildList[0] = NULL; //avoid delete m_Splitter

}

void CMainFrame::GetFocusDialog(int64& SourceID,int64& DialogID){
	CVSpace2* Item = m_LinkerView.GetCurDialog();
	//assert(Item != NULL);
	if (!Item)
	{
		SourceID = 0;
		DialogID = 0;
		return;
	};

	CVSpace2* Parent = Item->m_Parent;
    if (Parent == &m_LinkerView)
    {
		SourceID = Item->m_Alias;
		DialogID = DEFAULT_DIALOG;
		return;
    }

	DialogID  = Item->m_Alias;	
	do{
		Item = Parent;
		Parent = Item->m_Parent;
	}while(Parent != &m_LinkerView );
	
	SourceID = Item->m_Alias;
}

bool  CMainFrame::IsFocusDialog(int64 SourceID,int64 DialogID){
	CVSpace2* Item = m_LinkerView.GetCurDialog();
    if (Item == NULL)
    {
		return false;
    }
    

	CVSpace2* Parent = Item->m_Parent;
    if (Parent == &m_LinkerView) //第一级树枝
    {
		if (Item->m_Alias == SourceID && DialogID == DEFAULT_DIALOG)
		{
			return true;
		}
		return false;
	}

	if (Item->m_Alias != DialogID)
	{
		return false;
	};

	//对于其它级树枝，只要DialogID相同，SourceID理论上也应该相同
    //似乎不必有下面的步骤，不过为了安全起见还是验证一下
	do{
		Item = Parent;
		Parent = Item->m_Parent;
	}while(Parent != &m_LinkerView );

	if(SourceID == Item->m_Alias){
		return true;
	};

	return false;
}
void CMainFrame::SetCurDialogState(TASK_STATE  State){
    m_ConvView.SetCurDialogState(State);
}


void  CMainFrame::SendMsgToBrain(int64 SourceID,int64 DialogID, ePipeline& Msg,int64 EventID){
	CMsg Msg1(MSG_GUI_TO_BRAIN,0,EventID);
	ePipeline& Letter = Msg1.GetLetter();
	Letter.PushInt(SourceID);
	Letter.PushInt(DialogID);
	Letter.PushPipe(Msg);

	GetBrain()->PushCentralNerveMsg(Msg1); //本地界面直接压入中枢神经，如果是外地界面应该直接网络发送给Brain
}

void CMainFrame::SendMsgToBrainFocuse( ePipeline& Msg,int64 EventID)
{
	int64 SourceID;
	int64 DialogID;
	GetFocusDialog(SourceID,DialogID);

	SendMsgToBrain(SourceID,DialogID,Msg,EventID);
}

void CMainFrame::GUIMsgProc(){

	if (m_MsgList.DataNum()==0)
	{
		return;
	}
	CMsg Msg; 
	m_MsgList.Pop(Msg);

REPEAT:
	ePipeline& Receiver = Msg.GetReceiver();
	int64 SourceID = Receiver.PopInt();
	int64 DialogID = Receiver.PopInt();

	ePipeline& Letter = Msg.GetLetter();
	for (int i=0; i<Letter.Size(); i++)
	{
		ePipeline& Cmd = *(ePipeline*)Letter.GetData(i);
		int64 CmdID = Cmd.GetID();
	
		/* just debug
		tstring s = tformat(_T("Msg:%I64ld"),CmdID);
		m_Status.SetTip(s);
        */ 

		switch(CmdID){

		case GUI_DIALOG_OUTPUT:
			{

				ePipeline* LastItem = (ePipeline*)Cmd.GetData(0);
				
				if (GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					m_ConvView.AddDialogItem(*LastItem);
				} 
				else
				{
					m_LinkerView.NotifyDialogHasNew(SourceID,DialogID);
				}
			}
			break;
		case GUI_RUNTIME_OUTPUT:
			{
				ePipeline* LastItem = (ePipeline*)Cmd.GetData(0);
				
				if (GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					m_ConvView.AddRuntimeInfo(*LastItem);
				} 
				else
				{
					m_LinkerView.NotifyDialogHasNew(SourceID,DialogID);
				}
			}
			break;
		case GUI_FORECAST_OUTPUT:
			{

			}
			break;
		case GUI_TASK_STATE_UPDATE:
			{
				
			}
			break;
		case GUI_CLEAR_OUTPUT:
			{

				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}

				int64 Type = Cmd.PopInt();
				if (Type == CLEAR_DIALOG)
				{
					m_ConvView.ClearDialogInfo();
				}else if (Type == CLEAR_RUNTIME)
				{
					m_ConvView.ClearRuntimeInfo();
				}else if (Type == CLEAR_FORECAST)
				{
					m_ConvView.ClearForecastInfo();
				}
			}
			break;
		case GUI_ONE_LIGHT_FLASH:
			{

				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}

				int64 Enable = Cmd.PopInt();
				if (Enable)
				{
					m_Status.LightLamp(IN_LAMP,TRUE);
				}else{
					m_Status.LightLamp(IN_LAMP,FALSE);
				}
				
			}
			break;
		case GUI_FIND_OUTPUT:
			{
				m_ConvView.FindViewProc(Cmd);
			}
			break;
		case GUI_MEMORY_OUTPUT:
			{
				m_ConvView.MemoryViewProc(Cmd);

			}
			break;
		case GUI_ADDRESSBAR_STATE:
			{
				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}

				int64 Flag = Cmd.PopInt();
				m_AddressBar.SetConnectState(Flag);
			}
			break;
		case GUI_STATUS_SET_TEXT:
			{
				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}

				tstring s = Cmd.PopString();
				m_Status.SetTip(s);
			}
			break;

		case GUI_SET_EIDT:
			{

				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}

				tstring s = Cmd.PopString();
				int64 Enable = Cmd.PopInt();
				m_ConvView.m_InputWin.ClearEdit(s.c_str());
				m_ConvView.m_InputWin.EnableInput(Enable);
			}
			break;
		case GUI_CONTINUE_EDIT:
			{

				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}

				int64 bSetEdit = Cmd.PopInt();
				if(!bSetEdit)
				{
					m_ConvView.m_InputWin.ContinueEdit(NULL);
				}else {
					tstring s = Cmd.PopString();
					m_ConvView.m_InputWin.ContinueEdit(s.c_str());
				}
				m_ConvView.m_InputWin.EnableInput(TRUE);
			}
			break;
		case GUI_PFM_MSG_UPDATE:
			{
				int64 Type = Cmd.PopInt();
				ePipeline* Data = (ePipeline*)Cmd.GetData(0);
				m_PerformanceView.m_NerveMsgNumView.AppendLineData(Type,*Data);
			}
			break;
		case GUI_PFM_THREAD_UPDATE:
			{
				int64 Type = Cmd.PopInt();
				int64 ThreadNum = Cmd.PopInt();
				m_PerformanceView.m_ThreadNumView.AppendLineData(Type,ThreadNum);

			}
			break;
		case GUI_LOGIC_OUTPUT:
			{
				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}

				m_ConvView.SetLogicView(Cmd);
			}
			break;
		case GUI_OBJECT_OUTPUT:
			{
				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}
				
				m_ConvView.SetObjectView(Cmd);
			
			}
			break;
		case GUI_LINKVIEW_OUTPUT:
			{

				int64  Act = Cmd.PopInt();
				if(Act == ADD_ITEM)
				{
					int64 SourceID = Cmd.PopInt();
					int64 DialogID = Cmd.PopInt();
					int64 ParentID = Cmd.PopInt();
					tstring Name   = Cmd.PopString();
					m_LinkerView.AddDialog(SourceID,DialogID,ParentID,Name.c_str());

				}else if (Act == DEL_ITEM)
				{
					int64 SourceID = Cmd.PopInt();
					int64 DialogID = Cmd.PopInt();
					m_LinkerView.DeleteDialog(SourceID,DialogID);
				}else if (Act == INIT_LIST)
				{					
					m_LinkerView.ClearAll();					
					ePipeline* DialogList = (ePipeline*)Cmd.GetData(0);
					for (int i=0; i<DialogList->Size(); i++)
					{
						int64 SourceID = DialogList->PopInt();
						int64 DialogID = DialogList->PopInt();
						int64 ParentID = DialogList->PopInt();
						tstring Name;    DialogList->PopString(Name);
						int64 Type = DialogList->PopInt();
						
						m_LinkerView.AddDialog(SourceID,DialogID,ParentID,Name);
					} 	

					m_LinkerView.SetCurDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
					ePipeline Pipe;
					m_ConvView.SetCurDialog(SYSTEM_SOURCE,DEFAULT_DIALOG,Pipe);	

				}
			}
			break;
		case GUI_SET_CUR_DIALOG:
			{
				int64 SourceID = Cmd.PopInt();
				int64 DialogID = Cmd.PopInt();

				m_LinkerView.SetCurDialog(SourceID,DialogID);
				m_ConvView.SetCurDialog(SourceID,DialogID,Cmd);
			}
			break;
		case GUI_VIEW_LAYOUT:
			{
				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}
				int64 State = Cmd.PopInt();
				m_ConvView.SetCurDialogState((TASK_STATE)State);
			}
			break;
		case GUI_STATUS_PROGRESS:
			{
				int64 Per =  Cmd.PopInt();
                m_Status.SetProgressPer(Per);
				
			}
			break;
		case GUI_TASK_TOOL_BAR:
			{		
				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}

				int64 bRun   = Cmd.PopInt();
				int64 bPause = Cmd.PopInt();
				int64 bStop  = Cmd.PopInt();
				
				m_ConvView.SetTaskToolbarState(bRun,bPause,bStop);

				if(!bPause && !bRun && !bStop){
//					m_ConvView.EnableDebugBnt(FALSE);
					m_ConvView.m_OutputWin.ShowView(BNT_DEBUG,FALSE);
				}
			}
			break;

		case GUI_DEBUGVIEW_OUTPUT:
			{
				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}

				m_ConvView.DebugViewProc(Cmd);
			}
			break;
		case GUI_SET_WORKMODE:
			{

				if (!GetGUI()->IsFocusDialog(SourceID,DialogID))
				{
					break;
				}

				int64 WordMode = Cmd.PopInt();
				switch(WordMode){
				case WORK_TASK:
					m_ConvView.SetInputTip(_T("Cmd>"));
					break;
				case WORK_THINK:
					m_ConvView.SetInputTip(_T("Think>"));
					break;
				case WORK_DEBUG:
					m_ConvView.SetInputTip(_T("Debug>"));
					break;
				case WORK_CHAT:
					m_ConvView.SetInputTip(_T("Chat>"));
					break;
				case WORK_LEARN:
					m_ConvView.SetInputTip(_T("Learn>"));
					break;
				default:
					break;

				}
			}
			break;
		case GUI_SPACE_OUTPUT:
			{
				OnSpaceOutput(Cmd);
			}
			break;
		case GUI_CONNECT_STATE:
			{
				int64 State = Cmd.PopInt();
				if (State == CON_START)
				{
				    tstring ip = Cmd.PopString();
					tstring tip = Format1024(_T("Connect...%s"),ip.c_str());
					
					m_MapView.m_MapView.EnableNewConnect(FALSE);	
					m_Status.SetTip(tip);
					m_AddressBar.SetConnectState(TRUE);

				}else if (State == CON_END)
				{
                    tstring ip = Cmd.PopString();
					//tstring tip = tformat(_T("Connect...%s"),ip.c_str());
					
					int64 ret = Cmd.PopInt();
					
					m_WorldShow.SetRoomTitle(_T(""));
					m_Status.SetTip(_T(""));

					m_MapView.m_MapView.EnableNewConnect(TRUE);
					
					if (ret) //成功
					{
						m_AddressBar.SetConnectState(TRUE);
                        if(m_MapView.GetHwnd()){
							::SendMessage(m_MapView.GetHwnd(),WM_CLOSE,0,0);
						};
					}else{
						m_AddressBar.SetConnectState(FALSE);
						m_MapView.Reset();
						m_WorldShow.ConnectRoomFail(_T(""));
					}
				} 
			}
			break;
		case GUI_SET_DIALOG_MORE_ITEE:
			{
				ePipeline* LastItem = (ePipeline*)Cmd.GetData(0);			
				m_ConvView.AddMoreDialogItem(SourceID,DialogID,*LastItem);
			}
			break;
		default:
			assert(0);
			break;
		}
	}

	if (m_MsgList.DataNum())
	{
		m_MsgList.Pop(Msg);
        goto REPEAT;
	}
}

void CMainFrame::OnSpaceOutput(ePipeline& Letter){
	
    int64 Cmd = Letter.PopInt();
	
	if (Cmd == SPACE_CATALOG)
	{
		OnSpaceOutput_Catalog(Letter);
	}else if (Cmd == SPACE_ADDED)
	{
		OnSpaceOutput_Added(Letter);
	}else if (Cmd == SPACE_DELETED)
	{
		OnSpaceOutput_Deleted(Letter);
	}
	
}

void CMainFrame::OnSpaceOutput_Catalog(ePipeline& Data){
	

	ePipeline* SpaceList = (ePipeline*)Data.GetData(0);	
	
    int64 RoomID = SpaceList->GetID();
   	tstring RoomName = SpaceList->GetLabel();
	
	MapItem* RoomItem = m_MapView.FindChildItem(RoomID);
	assert(RoomItem);
    bool insertmap = false;
	if(RoomItem->m_ChildList.size()==0)insertmap=true;
	
	if(RoomID == LOCAL_SPACEID){
		m_MapView.InitLocalRoom(RoomName);
	}else {
        assert(RoomItem);
		RoomName = RoomItem->m_Text;
	}
	
	ePipeline Addr;
	m_MapView.m_MapView.GetSpacePath(Addr);
	m_AddressBar.SetCurSpaceAddress(Addr);

	int n = SpaceList->Size()/3;
	m_WorldShow.BeginUpdateRoom(RoomName,RoomID,n);
	
	//	m_MapView.DeleteAllChild(RoomID); 地图只构建新的不删除旧的
	
	eElectron E1;
	for(int i=0; i<n; i++){
			
		int64 ChildID = AbstractSpace::CreateTimeStamp();
		tstring  Name = SpaceList->PopString();
		
		int32    Type = SpaceList->PopInt();
	    tstring  Fingerprint = SpaceList->PopString();

		HICON hIcon = NULL;	
		
		if(Type==LOCAL_SPACE || Type == OUTER_SPACE) {//如果是一个房间
			hIcon = MapItem::hRoom;
			if(insertmap)m_MapView.AddItem(RoomID,ChildID,Name,hIcon);
		}else if(Type == CONTAINER_SPACE){
			hIcon = MapItem::hContainer;
			if(insertmap)m_MapView.AddItem(RoomID,ChildID,Name,hIcon);
		}else if(Type == ROBOT_VISITER){
			hIcon = MapItem::hPeople;
			if(insertmap)m_MapView.AddItem(RoomID,ChildID,Name,hIcon);
		}
		else{
			hIcon = CUseShGetFileInfo::GetFileIconHandle(Name.c_str(),TRUE);
		} 
		m_WorldShow.AddObject(RoomID,ChildID,Name,(SPACETYPE)Type,Fingerprint,hIcon);
	}
	m_WorldShow.EndUpdateRoom(RoomID);
}

void CMainFrame::OnSpaceOutput_Added(ePipeline& Data){
	
	assert(Data.Size()==1);		
	
    eElectron E1;
	Data.Pop(&E1);
	ePipeline* ChildPipe = (ePipeline*)E1.Value();	
	
	
	int64 ParentID  = ChildPipe->GetID();
	int64 ChildID = AbstractSpace::CreateTimeStamp();
	
	//MapItem* ParentRoom = m_MapView.FindChildItem(RoomID);
	//assert(ParentRoom != NULL);	
	
    tstring Name  = ChildPipe->PopString();
	int32   Type  = ChildPipe->PopInt();
	tstring Fingerprint = ChildPipe->PopString();
	
	HICON hIcon = NULL;	
	
	if(Type==LOCAL_SPACE || Type == OUTER_SPACE) {//如果是一个房间
		hIcon = MapItem::hRoom;
		m_MapView.AddItem(ParentID,ChildID,Name,hIcon);
	}else if(Type == CONTAINER_SPACE){
		hIcon = MapItem::hContainer;
		m_MapView.AddItem(ParentID,ChildID,Name,hIcon);
	}else if(Type == ROBOT_VISITER){
		hIcon = MapItem::hPeople;
		m_MapView.AddItem(ParentID,ChildID,Name,hIcon);
	}
	else{
		hIcon = CUseShGetFileInfo::GetFileIconHandle(Name.c_str(),TRUE);
	} 
	m_WorldShow.AddObject(ParentID,ChildID,Name,(SPACETYPE)Type,Fingerprint,hIcon);
}

void CMainFrame::OnSpaceOutput_Deleted(ePipeline& Data){
	
	ePipeline* Path = (ePipeline*)Data.GetData(0);
	tstring Name = *(tstring*)Data.GetData(1);
    int64  Type  = *(int64*)Data.GetData(2);
	tstring Fingerprint  = *(tstring*)Data.GetData(3);

	//是当前焦点空间
	int64 MapItemID = Path->GetID();
	
	MapItem* Item = m_MapView.m_MapView.GetCurRoom();
	if (Item && Item->m_Alias == MapItemID)
	{
		m_WorldShow.DeleteObject(Fingerprint);	
	}
	
	if (Type == CONTAINER_SPACE)
	{		
		m_MapView.DeleteItem(Path,Name); 
	}

};

void CMainFrame::OnSpaceOutput_SendObject(ePipeline& Data){


};

void CMainFrame::OnSpaceOutput_AccessDenied(ePipeline& Data){
	tstring Name = Data.PopString();
    tstring s = Format1024(_T("Space %s access denied"),Name.c_str());
	::MessageBox(GetHwnd(),s.c_str(),_T("Warning"),MB_OK);
}

void CMainFrame::Layout(bool Redraw /* = true */){
	CWSFrameView::Layout(Redraw);

	if(m_Created){
		RECT rc = m_rcClient;	
		if (m_bOpenPerformanceView)
		{
			rc.bottom -= 220;
		}else{
			rc.bottom -= 20;
		}

		if(rc.bottom<rc.top)rc.bottom=rc.top;
		
		::MoveWindow(m_AddressBar.GetHwnd(),rc.left,rc.top,RectWidth(rc),28,TRUE);
        rc.top+=28;

		//m_Splitter.Layout(rc);
		
		
		if (m_bShowWorld)
		{
			m_OldSplitterPos = m_Splitter.GetFirstSplitterPos();
			::MoveWindow(m_WorldShow.GetHwnd(),rc.left,rc.top,RectWidth(rc),RectHeight(rc),TRUE);
			RECT rc1;
			::SetRect(&rc1,0,0,1,1);
			m_Splitter.Layout(rc1);
		}else{
			m_Splitter.Layout(rc);
			m_Splitter.SetFirstSplitterPos(m_OldSplitterPos);

			RECT rc1;
			::SetRect(&rc1,0,0,0,0);
			::MoveWindow(m_WorldShow.GetHwnd(),rc1.left,rc1.top,RectWidth(rc1),RectHeight(rc1),TRUE);
		}
		
		/*
		::MoveWindow(m_LinkerView.GetHwnd(),rc.left,rc.top,120,RectHeight(rc),TRUE);
		::MoveWindow(m_ConvView.GetHwnd(),rc.left+120,rc.top,RectWidth(rc)-120,RectHeight(rc),TRUE);
 		*/
		rc = rc;
		rc.top   = rc.bottom-1;

		if (m_bOpenPerformanceView)
		{
			rc.bottom = rc.top +20;
			::MoveWindow(m_Status.GetHwnd(),rc.left,rc.top,RectWidth(rc),RectHeight(rc),TRUE);    
            rc.top = rc.bottom+1;
			rc.bottom = m_rcClient.bottom;
			::MoveWindow(m_PerformanceView.GetHwnd(),rc.left,rc.top,RectWidth(rc),RectHeight(rc),TRUE);    
		}else{
			rc.bottom=m_rcClient.bottom;
			::MoveWindow(m_Status.GetHwnd(),rc.left,rc.top,RectWidth(rc),RectHeight(rc),TRUE);    
		}
	}
}
  
LRESULT CMainFrame::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam,lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam,lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam,lParam);
	case WM_TIMER:
		return OnTimer(wParam,lParam);
	case WM_CREATE:
		return OnCreate(wParam,lParam);
	case WM_CLOSE:
		//m_MsgProcObject.Dead();
		KillTimer(GetHwnd(),GUI_MSG_PROC_TIMER);
	default:
		return CWSFrameView::Reaction(message,wParam,lParam);
	}
};
LRESULT CMainFrame::OnTimer(WPARAM wParam, LPARAM lParam){
	if (wParam == GUI_MSG_PROC_TIMER)
	{
		GUIMsgProc();
	}
	else {
		return CWSFrameView::OnTimer(wParam,lParam);
	}
	return 0;
}
LRESULT CMainFrame::OnCreate( WPARAM wParam, LPARAM lParam) 
{
	DWORD style = GetWindowLong(m_hWnd,GWL_STYLE);
	style |=WS_CLIPCHILDREN;
	SetWindowLong(m_hWnd,GWL_STYLE,style);
	
	RECT rc;
	::SetRect(&rc,0,0,0,0);
	
	SS.Init();

	if(!m_AddressBar.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}
	
	if(!m_ConvView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}
	
	if(!m_LinkerView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE,rc,GetHwnd())){
		return -1;
	}
	
	if(!m_Status.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE,rc,GetHwnd())){
		return -1;
	}

	if(!m_WorldShow.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd())){
		return -1;
	}

    m_Status.SetProgressAlpha(127);

	//m_LinkerView.AddDialog(SYSTEM_SOURCE,DEFAULT_DIALOG,_T("System"),DIALOG_SYSTEM_MAIN);

	//m_LinkerView.SetCurDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
	//ePipeline Pipe;
	//m_ConvView.SetCurDialog(SYSTEM_SOURCE,DEFAULT_DIALOG,Pipe);	
	

	m_Splitter.AddView(&m_LinkerView);
	m_Splitter.AddView(&m_ConvView);


	/*
	if (!m_MsgProcObject.Activation())
	{
		return -1;
	}
	*/

	SetTimer(GetHwnd(),GUI_MSG_PROC_TIMER,100,NULL);
	m_Created = TRUE;
    return 0;
};


LRESULT CMainFrame::OnLButtonDown(WPARAM wParam, LPARAM lParam) 
{
    POINTS p = MAKEPOINTS(lParam);
	
	POINT point;
	point.x = p.x;
	point.y = p.y;	
	m_SpaceSelected = Hit(point);
	if(m_SpaceSelected){
		m_SpaceSelected->m_State |= SPACE_SELECTED;
		if(m_SpaceSelected->m_Alias == ID_SPLITTER){	
			m_Splitter.OnLButtonDown((SplitterItem*)m_SpaceSelected);
		}
	}
	return CWSFrameView::OnLButtonDown(wParam,lParam);
};

LRESULT CMainFrame::OnLButtonUp(WPARAM wParam, LPARAM lParam) 
{
	
	if(m_SpaceSelected){
		if(m_SpaceSelected->m_Alias == ID_SPLITTER) {
			m_Splitter.OnLButtonUp((SplitterItem*)m_SpaceSelected);
			m_SpaceSelected->m_State &= ~SPACE_SELECTED;
            m_SpaceSelected = NULL; 	
		}
	}
	return 0;
};


LRESULT CMainFrame::OnMouseMove(WPARAM wParam, LPARAM lParam) 
{
	
	POINTS point = MAKEPOINTS(lParam);
	if(m_SpaceSelected){
		if(m_SpaceSelected->m_Alias == ID_SPLITTER){
            m_Splitter.OnMouseMove(point.x,point.y,(SplitterItem*)m_SpaceSelected);
		}
	}		
	return CWSFrameView::OnMouseMove(wParam,lParam);
};


LRESULT CMainFrame::ParentReaction(SpaceRectionMsg* SRM){

	if (SRM->Msg == CM_CONNECT)
	{
		int64  ID = SRM->lParam;
		IP ip(ID);
        AnsiString Ipstr = ip.Get();
		ePipeline  Msg(GUI_CONNECT_TO);
		Msg.PushString(Ipstr);
		Msg.PushInt(SPACE_PORT);
		
		int64 EventID = AbstractSpace::CreateTimeStamp();
		GetGUI()->SendMsgToBrain(SYSTEM_SOURCE,DEFAULT_DIALOG,Msg,EventID);
	}
    else if (SRM->Msg == CM_DISCONNECT)
	{
//        OUTPUT_LOG(this,_T("用户请求终止连接.."));

		ePipeline  Msg(GUI_DISCONNECT);
		
		int64 EventID = AbstractSpace::CreateTimeStamp();
		GetGUI()->SendMsgToBrain(SYSTEM_SOURCE,DEFAULT_DIALOG,Msg,EventID);

	}else if (SRM->Msg == BNT_PERFORMANCE_VIEW)
	{
		if (m_bOpenPerformanceView)
		{
            KillTimer(GetHwnd(),2010);

			m_bOpenPerformanceView = FALSE;
			m_PerformanceView.Destroy();

			RECT rc;
			GetClientRect(GetHwnd(),&rc);
			rc.bottom -=200;
			SetWindowPos(GetHwnd(),NULL,0,0,rc.right-rc.left,rc.bottom-rc.top,SWP_NOMOVE);
         
		}else{
			m_bOpenPerformanceView = TRUE;
			RECT rc={0};
			BOOL ret = m_PerformanceView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,rc,GetHwnd());    
			assert(ret);

			GetClientRect(GetHwnd(),&rc);
			rc.bottom+=200;
			SetWindowPos(GetHwnd(),NULL,0,0,rc.right-rc.left,rc.bottom-rc.top,SWP_NOMOVE);

		//	SetTimer(GetHwnd(),2010,500,NULL);

		}

	}else if (SRM->Msg == BNT_OPTION_VIEW)
	{	
		if (m_OptionView.GetHwnd())
		{
			m_OptionView.Destroy();
		}else{
			RECT rc;
			rc.left = 100;
			rc.top  = 100;
			rc.right = 500;
			rc.bottom = 600;
			BOOL ret = m_OptionView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_POPUP|WS_CLIPCHILDREN|WS_EX_TOOLWINDOW,rc,GetHwnd());		
		}
	}else if (SRM->Msg == BNT_LOG_VIEW)
	{
		if (m_LogView.GetHwnd())
		{
			m_LogView.Destroy();
		}else{
			RECT rc;
			rc.left = 100;
			rc.top  = 100;
			rc.right = 900;
			rc.bottom = 700;
			BOOL ret = m_LogView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_POPUP|WS_CLIPCHILDREN|WS_EX_TOOLWINDOW,rc,GetHwnd());		
		}
	}
	else if (SRM->Msg == BNT_SPACE_VIEW)
	{
        m_bShowWorld = !m_bShowWorld;
		Layout();
	}else if (SRM->Msg == CM_OPENVIEW)
	{
		RECT rc;
		rc.left = 100;
		rc.top  = 100;
		rc.right = 400;
		rc.bottom = 300;
		BOOL ret = m_MapView.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE|WS_POPUP|WS_CLIPCHILDREN|WS_EX_TOOLWINDOW,rc,GetHwnd());		
		
		CenterWindow(m_MapView.GetHwnd(),GetHwnd());

//		assert(0);
	}else if (SRM->Msg == CM_CLOSEVIEW)
	{
		m_MapView.Destroy();
	}
	else if (SRM->Msg == WM_DESTROY)
	{
		if(SRM->ChildAffected == &m_OptionView)
		{
			m_Status.SetBntState(BNT_OPTION_VIEW,0);
		}else if (SRM->ChildAffected == &m_LogView)
		{
			m_LogView.ResetFlag();
			m_Status.SetBntState(BNT_LOG_VIEW,0);
		}
	}
    return 0;
}

