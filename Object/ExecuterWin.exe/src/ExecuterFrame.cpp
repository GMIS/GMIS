// ExecuterFrame.cpp: implementation of the CMainFrame class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786 4244)

#include "UserLinkerPipe.h"
//#include <winsock2.h>
//#include <process.h>    
#include "ExecuterFrame.h"
#include <iostream>
#include "DLL_Load.h"
#include "SpaceMsgList.h"

#include <tchar.h>

tstring GetCurrentDir(){
	static tstring Dir;
	if (Dir.size()==0)
	{
		TCHAR buf[512];
		::GetModuleFileNameW(   
			GetModuleHandle(NULL),         
			buf,       
			512                 
			);   
		
		Dir = buf;
		int n = Dir.find_last_of(_T("\\"));
		Dir=Dir.substr(0,n+1);
	}
	return Dir;
};

tstring CSetCurWorkDir::m_DefaultCurDir = GetCurrentDir();

tstring GetTempDir(){
	
	static tstring Dir;

	if (Dir.size()==0){
		Dir = GetCurrentDir();
		Dir +=_T("Temp\\");
		_tmkdir(Dir.c_str());
	}
	return Dir;
}

Mass* CExObject::LoadObject(tstring DllFileName){
	DeleteObject();

	tstring ObjectName = GetFileNoPathName(DllFileName);

	//m_DLLHandle  =	LoadLibrary(DllFileName.c_str());
	m_DLLHandle = LoadLibraryEx(DllFileName.c_str(),0,LOAD_WITH_ALTERED_SEARCH_PATH);
	if(m_DLLHandle == NULL)return NULL;
	pGetObjectFunc pFunc = NULL;
	pFunc = ( pGetObjectFunc) GetProcAddress(m_DLLHandle, "CreateObject");
	
	int64 ID = (int64)m_DLLHandle;
	if(pFunc !=NULL ){
		m_Object =(pFunc)(ObjectName,ID);
	}
	return m_Object;
}
bool CExObject::GetObjectDoc(tstring& Doc){
	if(m_DLLHandle == NULL || m_Object==NULL)return false;
	pGetObjectDocFunc pFunc = NULL;
	pFunc = ( pGetObjectDocFunc) GetProcAddress(m_DLLHandle, "GetObjectDoc");		 
	if(pFunc !=NULL ){
		/*奇怪了，这样居然只会输出第一个字符
		const TCHAR* s = ( pFunc)();
		if (s)
		{
			Doc = *s;
			return true;
		}*/
		//改为现在这样反而正常
		Doc =  ( pFunc)();
		return true;
	}
	return false;
}
void  CExObject::DeleteObject()
{
	if(m_DLLHandle == NULL || m_Object==NULL)return;
	pDeleteObjectFunc pFunc = NULL;
	pFunc = ( pDeleteObjectFunc) GetProcAddress(m_DLLHandle, "DestroyObject");		 
	if(pFunc !=NULL )( pFunc)(m_Object);
	FreeLibrary(m_DLLHandle);
}

CExObject::CExObject()
:m_Object(NULL),m_DLLHandle(NULL),m_bRuning(false){
  
}
CExObject::CExObject(int64 ID,tstring& File)
:m_ID(ID),m_FileAddress(File),m_bRuning(false),m_Object(NULL)
{

}

CExObject::~CExObject(){
	WaitForStop(100);
	if (m_Object)
	{
		DeleteObject();
	}	
}


bool CExObject::WaitForStop(int64 Count){
	if (!m_Object)
	{
		return true;
	}

	if (m_Object->MassType()>=MASS_OBJECT )
	{
		PHYSIC::Object* Ob = (PHYSIC::Object*)m_Object;
		Ob->Dead();	
	}
	
	while(m_bRuning && Count-- > 0){
		SLEEP_MILLI(100); //100毫秒一个周期
	 }
	return !m_bRuning;

};

TypeAB   CExObject::GetTypeAB(){
	assert(m_Object);
	return m_Object->GetTypeAB();
}


void CExObject::Start(CMainFrame* Frame,int64 EventID,ePipeline* ExePipe){
	AutoBool bRuning(m_bRuning); 
	if (!m_Object)
	{
		//先试着Load object		
		m_Object = LoadObject(m_FileAddress);
		if (m_Object==NULL)
		{
			ExePipe->SetID(RETURN_ERROR);
			ExePipe->GetLabel() = Format1024(_T("Dll Load fail:%s"),m_FileAddress.c_str());
			Frame->OutputLog(LOG_TIP,ExePipe->GetLabel().c_str());
			Frame->SendFeedback(EventID,*ExePipe);	
			return ;
		}
		

	}
	//初始化
	if (m_Object->MassType()>=MASS_OBJECT)
	{
		PHYSIC::Object* Ob = (PHYSIC::Object*)m_Object;
		if(!Ob->IsAlive() && !Ob->Activation()){

			ExePipe->SetID(RETURN_ERROR);
			ExePipe->GetLabel() = Format1024(_T("Object activation fail:%s"),m_FileAddress.c_str());
			Frame->OutputLog(LOG_TIP,ExePipe->GetLabel().c_str());

			Frame->SendFeedback(EventID,*ExePipe);
			return ;
		}
	}		
	Frame->SendFeedback(EventID,*ExePipe);	
}

void  CExObject::Run(CMainFrame* Frame,int64 EventID,ePipeline* ExePipe){
 
	AutoBool bRuning(m_bRuning);

	tstring Error;
	try{

		if (m_Object==NULL)
		{
			ExePipe->SetID(RETURN_ERROR);
			ExePipe->GetLabel() = Format1024(_T("Object invalid:%s"),m_FileAddress.c_str());
			Frame->SendFeedback(EventID,*ExePipe);	
			return;
		}

		uint32 Type = m_Object->GetTypeAB();             		
		if( Type != (Type & ExePipe->GetTypeAB()))  
		{
			Error = Format1024(_T("Object(%s) input data type check fail. Data type:%u Require type:%u"),GetName().c_str(),ExePipe->GetTypeAB(),Type); 
			Frame->OutputLog(LOG_TIP,Error.c_str());
			ExePipe->SetLabel(Error.c_str()); 
			ExePipe->SetID((RETURN_ERROR));
			Frame->SendFeedback(EventID,*ExePipe);
			return;

		}else{

			if(!m_Object->Do(ExePipe)){
				ExePipe->SetID((RETURN_ERROR));
				Frame->SendFeedback(EventID,*ExePipe);
				return;
			}else{
				tstring s = Format1024(_T("Run Object(%s) ok \n"),m_FileAddress.c_str());
				Frame->OutputLog(LOG_TIP,s.c_str());		
				Frame->SendFeedback(EventID,*ExePipe);
				return;
			}   			
		}

	}catch(...)
	{

	}

	if(Error.size()==0){
		Error = Format1024(_T("Run Object(%s) fail:Exception error"),m_FileAddress.c_str());
	}

	Frame->OutputLog(LOG_TIP,Error.c_str());

	ExePipe->SetID(RETURN_ERROR);
	ExePipe->SetLabel(Error.c_str());	
	Frame->SendFeedback(EventID,*ExePipe);

	return;
}

void CExObject::GetDoc(CMainFrame* Frame,int64 EventID,ePipeline* ExePipe){

	try{
		tstring Doc;
		bool ret = GetObjectDoc(Doc);
		if (!ret)
		{
			ExePipe->SetID(RETURN_ERROR);
			ExePipe->GetLabel() = Format1024(_T("Object invalid:%s"),m_FileAddress.c_str());
			Frame->SendFeedback(EventID,*ExePipe);	
			return;
		}
		ExePipe->PushString(Doc);

		tstring s = Format1024(_T("Get Object Doc(%s) ok \n"),m_FileAddress.c_str());
		Frame->OutputLog(LOG_TIP,s.c_str());		
				
		Frame->SendFeedback(EventID,*ExePipe);
		return;

	}catch(...)
	{

	}

	tstring Error = Format1024(_T("Run Object(%s) fail:Exception error"),m_FileAddress.c_str());

	Frame->OutputLog(LOG_TIP,Error.c_str());

	ExePipe->SetID(RETURN_ERROR);
	ExePipe->SetLabel(Error.c_str());	
	Frame->SendFeedback(EventID,*ExePipe);

	return;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMainFrame::CMainFrame(int64 Alias,CModelInitData* InitData)
:m_Alias(Alias),
 m_EditStrBuf(m_AppLog),
 m_EditStrBufW(m_AppLog),
 //m_Log("Executer.Log"),
Model(InitData)
{
	m_LogFlag =  LOG_TIP;//|LOG_MSG_PROC_BEGIN|LOG_MSG_IO_PUSH|LOG_MSG_I0_RECEIVED|LOG_MSG_IO_SENDED|LOG_MSG_CENTRL_NERVE_PUSH;
	m_Created = FALSE;
	m_Name = _T("Hand");

}

CMainFrame::~CMainFrame()
{
	cout.rdbuf( m_OldBuf );
	wcout.rdbuf(m_OldBufW );
	DeleteObject(m_BKBrush);	
   
}


tstring CMainFrame::MsgID2Str(int64 MsgID){
	static map<int64,tstring>  MsgTextList;
	if (MsgTextList.size()==0)
	{
		int n = ( sizeof( (_SpaceMsgTextList) ) / sizeof( (_SpaceMsgTextList[0]) ) ) ;

		for (int i=0; i<n; i++)
		{
			MsgTextList[_SpaceMsgTextList[i].msg_id] =_SpaceMsgTextList[i].msg_text;
		}		
	}
	tstring MsgStr;
	map<int64,tstring>::iterator it = MsgTextList.find(MsgID);
	if(it!=MsgTextList.end()){
		MsgStr = it->second;		
	}else
	{
		MsgStr = Format1024(_T("%I64ld"),MsgID);
	}
	return MsgStr;
};

void  CMainFrame::OutputLog(uint32 Type,const TCHAR* text){
	if(m_LogFlag & Type){
		std::wcout<<text<<endl;
	}
};
void CMainFrame::OutputLog(uint32 Flag,TCHAR* Format, ...){
	if (m_LogFlag & Flag)
	{
		TCHAR Buffer [512] ;
		va_list ArgList ;		
		va_start (ArgList, Format) ;
		_vsntprintf(Buffer, 256, Format, ArgList) ;		
		va_end (ArgList) ;

		std::wcout<<Buffer<<endl;
	}

};
void CMainFrame::NotifySysState(int64 NotifyID,ePipeline* Data /*= NULL*/){
	
}

void CMainFrame::NotifyLinkerState(CLinkerPipe* LinkerPipe,int64 NotifyID,ePipeline& Info){
	CUserLinkerPipe* Linker = (CUserLinkerPipe*)LinkerPipe;

	STATE_OUTPUT_LEVEL Flag = Linker->GetOutputLevel();
	switch(Flag){
	case WEIGHT_LEVEL:
	{
		switch(NotifyID)
		{
		case LINKER_RECEIVE_STEP:
			{
				//目前没有处理必要
				int32 DataSize = Info.PopInt();
				int32 ParentSize  = Info.PopInt();
                ePipeline* Data = (ePipeline*)Info.GetData(0);
			}
			return;
		case LINKER_SEND_STEP:
			{
				int64 MsgID   = Info.PopInt();
				int32 MsgSize = Info.PopInt();
				int32 SendNum = Info.PopInt();
				
				int32 Per = SendNum*100/MsgSize;

				tstring MsgName = MsgID2Str(MsgID);				
			}
			return;
		default:
			break;
		}
	}
	case LIGHT_LEVEL:
	{
		switch(NotifyID)
		{
		case LINKER_PUSH_MSG:
			{
				assert(Info.Size()==6);
				int64 MsgID = Info.PopInt();
				int64 EventID = Info.PopInt();
				int64 TimeStamp = Info.PopInt();
				int64 PendingMsgID = Info.PopInt();
				int64 MsgNum = Info.PopInt();
				int64 UrgMsgNum = Info.PopInt();

				tstring MsgName = MsgID2Str(MsgID);
				tstring PendingMsgName = PendingMsgID==0?_T("NULL"):MsgID2Str(PendingMsgID);
				OutputLog(LOG_MSG_IO_PUSH,_T("Msg push: %s EventID:%I64ld PendingMsg=%s CurMsgNum=%I64ld"),MsgName.c_str(),EventID,PendingMsgName.c_str(),MsgNum+UrgMsgNum);
			}
			return;
		case LINKER_MSG_SENDED:
			{		
				/*
				int64 SenderID = (int64)Linker->m_CurSendMsg.GetSenderID();

				CMsg Msg(SYSTEM_SOURCE,SenderID,MSG_LINKER_NOTIFY,DEFAULT_DIALOG);
                
				//通知原始信息的发送者
				ePipeline LocalAddress;
				Linker->ReceiverID2LocalAddress(SenderID,LocalAddress);
				Msg.GetReceiver() = LocalAddress;

				//附上原始发送信息
				ePipeline& Letter = Msg.GetLetter();
				Letter.PushInt32(NotifyID);
				Letter.Push_Directly(Linker->m_CurSendMsg.Release());
				*/
				assert (Info.Size());
				
				assert (Info.Size());

				ePipeline* Msg = (ePipeline*)Info.GetData(0);
				if (Msg->GetID()<100)
				{
					//WriteLogDB(_T("Msg Sended: internal feedback sended"));
				}else{
					CMsg SendedMsg;
					Info.PopMsg(SendedMsg);
					int64 MsgID = SendedMsg.GetMsgID();
					tstring MsgName = MsgID2Str(MsgID);
					OutputLog(LOG_MSG_IO_SENDED,_T("Msg Sended: %s  EventID:%I64ld"),MsgName.c_str(),SendedMsg.GetEventID());
				}
			}
			return;
		case LINKER_MSG_RECEIVED:
			{
				int64 MsgID = Info.PopInt();
				if (MsgID == LINKER_FEEDBACK)
				{
					int64 ReceiveResult = Info.PopInt();				
					int64 PendingMsgID =  Info.PopInt();
					tstring PendingMsgName = MsgID2Str(PendingMsgID);
					if (ReceiveResult == RECEIVE_ERROR)
					{
						tstring s = Format1024(_T("ERROR: Remote receive %s"),PendingMsgName.c_str());

						OutputLog(LOG_MSG_IO_REMOTE_RECEIVED,s.c_str());
					} 
					else
					{	
						OutputLog(LOG_MSG_IO_REMOTE_RECEIVED,_T("Remote received msg:%s "),PendingMsgName.c_str());
					}
				} 
				else
				{	
					tstring MsgName = MsgID2Str(MsgID);
					OutputLog(LOG_MSG_I0_RECEIVED,_T("Msg received: %s "),MsgName.c_str());
				}	
	
			}
			return;
		default:
		    break;
		}

	}
	case NORMAL_LEVEL:
	{
		switch(NotifyID)
		{
		case LINKER_COMPILE_ERROR:
			{
				int64 ErrorType = Info.PopInt();
				tstring s = Format1024(_T("ERROR: linker compile msg error(%I64ld)"),ErrorType);

				OutputLog(LOG_ERROR,s.c_str());
			}
			return;
		case LINKER_RECEIVE_RESUME:
			{
				OutputLog(LOG_TIP,_T("Linker Notify: receive resumed"));
			}
			return;
		case LINKER_INVALID_ADDRESS:
			{
				CMsg Msg;
				Info.PopMsg(Msg);
				int64 MsgID = Msg.GetMsgID();
				tstring MsgName = MsgID2Str(MsgID);

				tstring s = Format1024(_T("WARNING: %s receiver address invalid"),MsgName.c_str());

				OutputLog(LOG_WARNING,s.c_str());
			}
			return;
		case LINKER_ILLEGAL_MSG:
			{
				ePipeline* Msg = (ePipeline*)Info.GetData(0);

				CPipeView PipeView(Msg);

				tstring text = PipeView.GetString();

				tstring s = Format1024(_T("Linker Notify: receive illegal msg \n<<\n%s\n>>\n"),text.c_str());

				OutputLog(LOG_WARNING,s.c_str());
				
			}
			return;
			
		default:
			return;
		}
	} 
	break;
	default:
		break;
	}
	
};

void CMainFrame::GetSuperior(int64 ID,CLinker& Linker){
	CLockedLinkerList* List = GetSuperiorLinkerList();
	List->GetLinker(ID,Linker);
}


void CMainFrame::SendMsg(CMsg& Msg){
	
	CLinker Linker;
	GetSuperior(SYSTEM_SOURCE,Linker);
	
	if (Linker.IsValid())
	{
		int64 MsgID = Msg.GetMsgID();

		tstring s = Format1024(_T("Send Msg ID=%I64ld"),MsgID);
		OutputLog(LOG_TIP,s.c_str());
		Linker().PushMsgToSend(Msg);
	} 
	else
	{
		tstring s = _T("Msg send fail: Linker invalid");
		OutputLog(LOG_TIP,s.c_str());
	}
}

void  CMainFrame::SendFeedback(int64 EventID,ePipeline& Param){
	CMsg rMsg(MSG_OBJECT_FEEDBACK,DEFAULT_DIALOG,EventID);
	ePipeline& rLetter = rMsg.GetLetter();
	rLetter.PushPipe(Param);
	SendMsg(rMsg);	
}

bool CMainFrame::Do(Energy* E /*=NULL*/){

		if (E== NULL)
	    {
			return true;
	    }

		ePipeline* Pipe = (ePipeline*)E;
		CMsg Msg(Pipe);

		uint32 LogFlag = m_LogFlag;
		if (LogFlag & LOG_MSG_PROC_BEGIN)
		{
			int64 MsgID = Msg.GetMsgID();
			int64 EventID = Msg.GetEventID();

			tstring CmdStr = MsgID2Str(MsgID);
			tstring& Tip = Msg.GetMsg().GetLabel();
			OutputLog(LOG_MSG_PROC_BEGIN,_T("-->Dialog Pop Msg:%s EventID:%I64ld %s"),CmdStr.c_str(),EventID,Tip.c_str());
			
		}

	
		int64 MsgID=Msg.GetMsgID();
		switch(MsgID){
		case MSG_OBJECT_START:
			OnObjectStart(Msg);
			break;
		case MSG_OBJECT_RUN: 
			OnObjectRun(Msg);
			break;
		case MSG_OBJECT_GET_DOC:
			OnObjectGetDoc(Msg);
			break;
		case MSG_OBJECT_CLOSE:
			OnObjectClose(Msg);
			break;
/*	
		case MSG_APP_RESET:
			OnAppReset(Msg);
			break;
*/
		case MSG_BROADCAST_MSG:
			OnBroadcasgMsg(Msg);
			break;
		case MSG_LINKER_ERROR:
			OnLinkerError(Msg);
			break;
		case MSG_WHO_ARE_YOU:
			OnWhoAreYou(Msg);
			break;
		case MSG_CONNECT_OK:
			OnConnectOK(Msg);
			break;
		case MSG_CONNECT_TO:
			OnConnectTo(Msg);
			break;
		default:
			{
				tstring tip = Format1024(_T("Unkown Msg ID: %I64ld"),MsgID);
				OutputLog(LOG_TIP,tip.c_str());
			}
			break;
		}

	return false;
}
void CMainFrame::OnConnectTo(CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter();
	int64 ID = Letter.PopInt();
	tstring& ip = Letter.PopString();
    int32 Port = Letter.PopInt();
	int32 Second = Letter.PopInt();
		
	AnsiString ip1 = WStoUTF8(ip);
	
	tstring error;
    
	if(!Connect(ID,ip1,Port,Second,error,TRUE)){
		//m_AddressBar.SetConnectState(FALSE);
		::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_CLOSE, 0);

	}else{
		//m_AddressBar.SetConnectState(TRUE);
	}
	
}


void CMainFrame::OnAppReset(CMsg& Msg){
/*暂时无用
	ePipeline& Letter = Msg.GetLetter();
   
	m_ObjectList.clear();

	int64 SourceID = Msg.GetSourceID();
    OBJECT_LIST::iterator SourceIt = m_ObjectList.find(SourceID);
    if ( SourceIt == m_ObjectList.end() )
	{
	   return;
	}		

	map< int64, map < int64,CExObject > >& DialogObjectList = SourceIt->second;

	for (int i=0; i<Letter.Size(); i+=4)
	{
	
		int64   DialogID  = Letter.PopInt64();
		int64   ObjectID = Letter.PopInt64();
		
		tstring Address = Letter.PopString();
		tstring Name    = Letter.PopString();

		map < int64,CExObject >& EventObjectList  = DialogObjectList[DialogID];
	
		CExObject& Object = EventObjectList[ObjectID]; 
		Object.SetName(m_Name);
	//	Object.m_Address = Address;
    //    Object.m_DialogID = DialogID;
	//	Object.m_ObjectID = ObjectID;
		Object.m_Frame = this;
	}
*/
};

void CMainFrame::OnObjectStart(CMsg& Msg)
{
	int64 EventID  = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();

    tstring ObjectFileName = Letter.PopString();	
	
	eElectron E;
	Letter.Pop(&E);
	ePipeline* ExePipe     = (ePipeline*)E.Value();
	
	m_Object.LoadObject(ObjectFileName);
	
	m_Object.Start(this,EventID,ExePipe);					
};


void CMainFrame::OnObjectRun(CMsg& Msg){

	int64 EventID  = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter();

	eElectron E;
	Letter.Pop(&E);
	ePipeline* ExePipe     = (ePipeline*)E.Value();

	m_Object.Run(this,EventID,ExePipe);								
}


void CMainFrame::OnObjectClose(CMsg& Msg){
	int64 EventID  = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();
		
	eElectron E;
	Letter.Pop(&E);
	ePipeline* ExePipe     = (ePipeline*)E.Value();
	

	if (!m_Object.WaitForStop(50)) //最多等候一秒
	{
		ExePipe->SetID(RETURN_ERROR);
		ExePipe->GetLabel() = Format1024(_T("Close object fail:%s"),m_Object.m_FileAddress.c_str());	
	}
    SendFeedback(EventID,*ExePipe);
	
//	CLinker Superior = GetSuperior(SYSTEM_SOURCE);
//	Superior().Close();

	::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_CLOSE, 0);
}
void CMainFrame::OnObjectGetDoc(CMsg& Msg)
{
	int64 EventID  = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter();

	eElectron E;
	Letter.Pop(&E);
	ePipeline* ExePipe     = (ePipeline*)E.Value();

	m_Object.GetDoc(this,EventID,ExePipe);								
}

void CMainFrame::OnBroadcasgMsg(CMsg& Msg){

	ePipeline& Letter = Msg.GetLetter();

	ePipeline& BcsData = *(ePipeline*)Letter.GetData(0);

	int64 BcsID = BcsData.GetID();

}



LRESULT CMainFrame::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
	case WM_ERASEBKGND:
		return TRUE;
	case WM_CREATE:
		return OnCreate(wParam,lParam);
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	default:
		return CWSFrameView::Reaction(message,wParam,lParam);
	}
	return 0;
}

LRESULT CMainFrame::OnCreate(WPARAM wParam, LPARAM lParam){
	m_hEdit = CreateWindow( 
					_T("EDIT"),     // predefined class 
					NULL,       // no window title 
					WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
					ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,//|ES_READONLY, 
					0, 0, 0, 0,  // set size in WM_SIZE message 
					m_hWnd,        // parent window 
					(HMENU) 100, // edit control ID 
					(HINSTANCE) GetWindowLong(m_hWnd, GWL_HINSTANCE), 
					NULL);                // pointer not needed 
	if(m_hEdit==NULL)return -1;
				
	m_BKBrush = ::CreateSolidBrush(RGB(0,0,0));
				
	m_AppLog.SetEditCtrl(m_hEdit);
				
	m_OldBuf = cout.rdbuf( &m_EditStrBuf );
	// and also the unicode version
	m_OldBufW = wcout.rdbuf( &m_EditStrBufW );
	return 0;
}
void CMainFrame::Layout(bool Redraw /* = true */){

	//if(!m_Created)return;		
	CWSFrameView::Layout(FALSE);

	RECT rcClient = m_rcClient;
	::MoveWindow(m_hEdit,rcClient.left,rcClient.top,RectWidth(rcClient),RectHeight(rcClient),TRUE);
    rcClient.left +=4;
	::SendMessage(m_hEdit,EM_SETRECT,0,(LPARAM)&rcClient);
};



void CMainFrame::OnWhoAreYou(CMsg& Msg){
	
    int64 SenderID = Msg.GetSenderID();
	int64 SourceID = Msg.GetSourceID();

	CLinker Linker;
	GetSuperior(SourceID,Linker);
	
	if(!Linker.IsValid()){
		return;
	};

	int64 EventID = 102;
	CMsg rMsg(SenderID,MSG_I_AM,DEFAULT_DIALOG,EventID);
	
	ePipeline& rLetter = rMsg.GetLetter();
	
	//tstring Name = GetName();

	//根据不同编译器决定不同Cryp，以便调用不同版本的EXECUTER,适应不同的C运行时库
#ifdef _DEBUG
#if _MSC_VER<1300
	tstring Name =_T("EXE_VC6D");
#elif _MSC_VER<1400
	tstring Name = _T("EXE_VC7D");
#elif _MSC_VER<1500
	tstring Name = _T("EXE_VC8D");
#elif _MSC_VER<1600
	tstring Name = _T("EXE_VC9D");
#elif _MSC_VER<1700
	tstring Name = _T("EXE_VC10D");
#endif	
#else
#if _MSC_VER<1300
	tstring Name =_T("EXE_VC6");
#elif _MSC_VER<1400
	tstring Name = _T("EXE_VC7");
#elif _MSC_VER<1500
	tstring Name = _T("EXE_VC8");
#elif _MSC_VER<1600
	tstring Name = _T("EXE_VC9");
#elif _MSC_VER<1700
	tstring Name = _T("EXE_VC10");
#endif
#endif
	
	tstring Cryp = Format1024(_T("%I64ld"),m_Alias);

	rLetter.PushString(Name);
	rLetter.PushString(Cryp);
	rLetter.PushInt(DIALOG_SYSTEM_MODEL);


	Linker().PushMsgToSend(rMsg);	
}

void CMainFrame::OnConnectOK(CMsg& Msg){
	int64 SourceID = Msg.GetSourceID();
	ePipeline& Letter = Msg.GetLetter();
	tstring Name;
	Letter.PopString(Name);
    int64 ReceiverID = Msg.GetReceiverID();
	
	CLinker Linker;
	GetSuperior(SourceID,Linker);
	if(Linker.IsValid()){
		Linker().SetRecoType(LINKER_FRIEND);

		CLockedLinkerList* LinkerList = GetSuperiorLinkerList();
		if (SourceID == SPACE_SOURCE_READY)
		{
			LinkerList->SetLinkerID(SPACE_SOURCE_READY,SPACE_SOURCE);
			OutputLog(LOG_TIP,_T("Space Connection ok!"));
		}else{
			OutputLog(LOG_TIP,_T("Robot Connection ok!"));	
//			int64 SenderID = Msg.GetSenderID();
//			CMsg rMsg(SenderID,MSG_APP_ONLINE,ReceiverID,0);
//			Linker().PushMsgToSend(rMsg);		
		}
	}


  
}

void CMainFrame::OnLinkerError(CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	ePipeline& Receiver = Msg.GetReceiver();
	
	int64 SourceID  = Letter.PopInt();
	//int32 ErrorID   = Letter.PopInt();
		
    CLinker Linker;
	GetSuperiorLinkerList()->GetLinker(SourceID,Linker);
		
	if (!Linker.IsValid())
	{
		::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_CLOSE, 0);
		return;
	}
	
	if (Linker().GetRecoType()==LINKER_DEL)
	{
		::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_CLOSE, 0);
		return;
	}

	
	GetSuperiorLinkerList()->DeleteLinker(SourceID);
	
	tstring s;
	if (Letter.GetEnergy(0)->EnergyType() == TYPE_INT)
	{	
		int32 ErrorID   = Letter.PopInt();
		s = Format1024(_T("Linker Closed:%d"),ErrorID);
	}else {
		assert(Letter.GetEnergy(0)->EnergyType()  == TYPE_STRING);
		s = Letter.PopString();
	}	
    OutputLog(LOG_TIP,s.c_str());

	::SendMessage(m_hWnd,WM_SYSCOMMAND, SC_CLOSE, 0);
}

