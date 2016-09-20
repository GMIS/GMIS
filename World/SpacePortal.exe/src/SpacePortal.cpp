// SpacePortal.cpp: implementation of the CSpacePortal class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "SpacePortal.h"
#include <direct.h>
#include "SpaceMsgList.h"
#include "resource.h"
#include "Space.h"
#include "sqlite\CppSQLite3.h"
#include "Poco/Checksum.h"
#include "DLL_Load.h"
#include "zip_utils\unzip.h"
#include <process.h>
#include "Win32Tool.h"

#include "MainFrame.h"

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
		int n = Dir.find_last_of(_T("\\"),tstring::npos);
		if(n!=-1){
			Dir=Dir.substr(0,n);
		}
	}
	return Dir;
};

tstring GetTempDir(){
	
	static tstring Dir;

	if (Dir.size()==0){
		Dir = GetCurrentDir();
		Dir +=_T("Temp\\");
		_tmkdir(Dir.c_str());
	}
	return Dir;
}


tstring SpacePath2FileAddress(ePipeline& Path)
{
	tstring CurDir = GetCurrentDir();
	
	while(Path.Size())
	{
		tstring s = Path.PopString();
		CurDir += _T("\\")+s;
	}
	return CurDir;
}

Dll_Object::Dll_Object()
:m_hDll(NULL),m_ID(0)
{
}

Dll_Object::Dll_Object(int64 ObjectID,tstring& FileName)
{	
	m_ID = ObjectID;
	m_FilePath = FileName;
	m_hDll  =	LoadLibraryEx(FileName.c_str(),0,LOAD_WITH_ALTERED_SEARCH_PATH);
}
Dll_Object::~Dll_Object(){
    if(m_hDll){
		::FreeLibrary(m_hDll);
	}
};
Dll_Object& Dll_Object::operator =(const Dll_Object& dll)
{

	m_ID = dll.m_ID;
	m_FilePath = dll.m_FilePath;
	m_hDll = dll.m_hDll;

	return *this;
}

int32 Dll_Object::GetDllType(){
	if(m_hDll == NULL)return 0;

	pGetObjectType pFunc = NULL;
	pFunc = ( pGetObjectType) GetProcAddress(m_hDll, "GetDllType");
	
	if(pFunc ==NULL )return 0;
	
    int32 Type =(pFunc)();
	return Type;
}


bool Dll_Object::IsValid(){
	return m_hDll != NULL;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSpacePortal::CSpacePortal(CUserTimer* Timer,CUserSpacePool* Pool)
:System(Timer,Pool)
{

	m_LogFlag = LOG_WARNING|LOG_TIP;//|LOG_MSG_I0_RECEIVED|LOG_MSG_IO_PUSH|LOG_MSG_IO_SENDED|LOG_MSG_IO_REMOTE_RECEIVED;

	m_bWriteLogToDatabase = FALSE;

	tstring Dir = GetCurrentDir();
	tstring WorldDir= Dir+ _T("\\World.db");

	//AnsiString AnsiDir = eSTRING::UTF16TO8(Dir);
	GetWorldDB(WorldDir.c_str());

    tstring LogDir =  Dir+_T("\\Log.DB");

	m_LogDB.Open(LogDir);

	m_MaxEventNumPerLinker = 10;
}

CSpacePortal::~CSpacePortal()
{


	map<int64,People*>::iterator it = m_VisitorList.begin();

	while(it != m_VisitorList.end()){
		People* robot = it->second;
		robot->GoOut();
		delete robot;
		it++;
	}
	m_VisitorList.clear();

	deque<People*>::iterator ita = m_VisitorPool.begin();
	while(ita != m_VisitorPool.end()){
		People* robot = *ita;
		delete robot;
		ita++;
	}

	GetWorldDB().close();
}

tstring CSpacePortal::MsgID2Str(int64 MsgID){
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

int64  CSpacePortal::PushEvent(CLIENT_TYPE Type,int64 ClientLinkerID,int64 ClientEventID,ePipeline& EventData,int64 ExecuterType,int64 ExecuterLinkerID,int64 ExecuterEventID){
	int64 SpaceEventID   = CreateTimeStamp();
	_CLOCK(&m_Mutex);
	CSpaceEvent& SpaceEvent = m_EventList[SpaceEventID];

	SpaceEvent.m_ClientLinkerID = ClientLinkerID;
	SpaceEvent.m_ClientEventID = ClientEventID;
	SpaceEvent.m_ClientType  = Type;
	SpaceEvent.m_ExecuterLinkerID  = ExecuterLinkerID;
	SpaceEvent.m_ExecuterEventID   = ExecuterEventID;
	SpaceEvent.m_ExecuterType      = ExecuterType;
	SpaceEvent.m_EventData   = EventData;

	return SpaceEventID;
}

bool CSpacePortal::GetEvent(int64 SpaceEventID, CSpaceEvent& Event){
	_CLOCK(&m_Mutex);
	map<int64, CSpaceEvent>::iterator it = m_EventList.find(SpaceEventID);
	if(it == m_EventList.end())return false;
	CSpaceEvent& SpaceEvent = it->second;
	Event = SpaceEvent;
	return true;
}
void CSpacePortal::ModifyEvent(int64 SpaceEventID, CSpaceEvent& Event){
	_CLOCK(&m_Mutex);
	map<int64, CSpaceEvent>::iterator it = m_EventList.find(SpaceEventID);
	if(it == m_EventList.end())return;
	CSpaceEvent& SpaceEvent = it->second;
	SpaceEvent = Event;
}
void CSpacePortal::PopEvent(int64 SpaceEventID){
	_CLOCK(&m_Mutex);
	map<int64, CSpaceEvent>::iterator it = m_EventList.find(SpaceEventID);
	assert(it != m_EventList.end());
	if(it != m_EventList.end()){
		m_EventList.erase(it);;
	}
}
bool CSpacePortal::Activate(){
   

	if(!System::Activate())return false;

	tstring error;
	if(!OpenPort(SPACE_PORT,error,false)){
		OutputLog(LOG_WARNING,error.c_str());
		m_Alive = false;
		return false;	
	};

	return true;	
};

People*  CSpacePortal::CheckinVisitor(int64 SourceID,tstring& Name,tstring& CryptText){
	_CLOCK(&m_Mutex);
	People* robot = NULL;
	if(m_VisitorPool.size()){
		robot = *m_VisitorPool.begin();
		m_VisitorPool.pop_front();
	}
	if(robot==NULL){
		robot = new People(Name,CryptText);
		if(!robot)return NULL;
	}else{
		robot->Reset(Name,CryptText);
	}
	assert(m_VisitorList.find(SourceID)==m_VisitorList.end());
	m_VisitorList[SourceID]=robot;

	return robot;
}
People*  CSpacePortal::GetVisitor(int64 SourceID){
	_CLOCK(&m_Mutex);
	map<int64,People*>::iterator it = m_VisitorList.find(SourceID);
	if(it==m_VisitorList.end())return NULL;
	People* robot = it->second;
	return robot;
};

int64 CSpacePortal::GetVisitorLinkerID(tstring& Name,tstring& Fingerprint){
	_CLOCK(&m_Mutex);
	map<int64,People*>::iterator it = m_VisitorList.begin();

	while(it!=m_VisitorList.end()){
		People* p = it->second;
		if(p->GetName() == Name && p->GetFingerprint() == Fingerprint){
			return it->first;
		}
		it++;
	}
	return 0;
}
void     CSpacePortal::CheckoutVisitor(int64 SourceID){
	_CLOCK(&m_Mutex);
	map<int64,People*>::iterator it = m_VisitorList.find(SourceID);
	if(it==m_VisitorList.end())return;
	People* robot = it->second;
	robot->GoOut();
	m_VisitorPool.push_back(robot);
	m_VisitorList.erase(it);
}

/*
bool  CSpacePortal::StartExecuter(int64 ExecuterID,tstring FileName){
    
	while(m_ExecuterPool.size()){
		int64 SourceID = m_ExecuterPool.back();
		m_ExecuterPool.pop_back();
		CLinker Linker;
		m_ClientLinkerList.GetLinker(SourceID,Linker);
		if (Linker.IsValid())
		{
			Linker().SetSourceID(ExecuterID);
			return true;
		}
	}

	//Register a start event, no matter the startup is success or fail,  the event will be deleted, through it to verify if it is invited to connect
	PushExecuterEvent(ExecuterID,1,102);


	TCHAR buf[512];
	GetCurrentDirectoryW(512,buf);
	tstring ExecuterFile = buf;
	ExecuterFile += _T("\\");
	ExecuterFile += FileName;
	
	OutputLog(LOG_TIP,ExecuterFile.c_str());

	STARTUPINFO si;
	PROCESS_INFORMATION  pi;	
	::ZeroMemory(&si, sizeof(si));	
	si.cb = sizeof(si);
	

	tstring CmdLine = Format1024(_T(" %I64ld"),ExecuterID);

	TCHAR CmdLineBuf[512];
	_tcscpy(CmdLineBuf,CmdLine.c_str());

	BOOL started = ::CreateProcessW(ExecuterFile.c_str(),        // command is part of input string
		CmdLineBuf,  // (writeable) command string
		NULL,        // process security
		NULL,        // thread security
		FALSE,       // inherit handles flag
		0,           // flags
		NULL,        // inherit environment
		NULL,        // inherit directory
		&si,    // STARTUPINFO
		&pi);   // PROCESS_INFORMATION
	
	if(!started){
		PopExecuterEvent(ExecuterID,102);
		return false;
	}

	OutputLog(LOG_TIP,_T("Waite Executer start..."));

	//Then wait for the connection to success
	int n=50;
	
	CLinker ExecuterLinker;
	m_ClientLinkerList.GetLinker(ExecuterID,ExecuterLinker);
	while((!ExecuterLinker.IsValid() || ExecuterLinker().GetRecoType() != LINKER_ORGAN) && n-- >0 ){
		m_ClientLinkerList.GetLinker(ExecuterID,ExecuterLinker);
		SLEEP_MILLI(50);
	}
	
	if (!ExecuterLinker.IsValid())
	{
		PopExecuterEvent(ExecuterID,102);
		return false;
 	}
	return true;
}
*/

int32  CSpacePortal::CountingClientEventNum(int64 SourceID){
	_CLOCK(&m_Mutex);

	map<int64, CSpaceEvent>::iterator it = m_EventList.begin();
	int32 n = 0;
	while (it != m_EventList.end())
	{
		CSpaceEvent& SpaceEvent = it->second;
		if (SpaceEvent.m_ClientLinkerID == SourceID)
		{
			n++;
		}
		it++;
	}
	return n;
}
void   CSpacePortal::LinkerBreakEventProc(int64 SourceID){

	_CLOCK(&m_Mutex);

	map<int64, CSpaceEvent>::iterator it = m_EventList.begin();

	while (it != m_EventList.end())
	{
		CSpaceEvent& SpaceEvent = it->second;

		if(SpaceEvent.m_ExecuterLinkerID == SourceID){
			CLinker ClientLinker;
			GetClientLinkerList()->GetLinker(SpaceEvent.m_ClientLinkerID,ClientLinker);

			if (ClientLinker.IsValid() && SpaceEvent.m_ClientEventID)
			{
				CMsg rMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_REQUEST_RUNTIME,DEFAULT_DIALOG,SpaceEvent.m_ClientEventID);
				ePipeline& rLetter = rMsg.GetLetter(false);
				rLetter.PushInt(REQUEST_STOP);
				ClientLinker().PushMsgToSend(rMsg);
			}
			it = m_EventList.erase(it);
		}
		else if (SpaceEvent.m_ClientLinkerID == SourceID)
		{
			CLinker ExecuterLinker;
			GetClientLinkerList()->GetLinker(SpaceEvent.m_ExecuterLinkerID,ExecuterLinker);

			if (ExecuterLinker.IsValid() && SpaceEvent.m_ExecuterEventID)
			{
				CMsg rMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_CLOSE_REQUEST,DEFAULT_DIALOG,SpaceEvent.m_ExecuterEventID);
				ExecuterLinker().PushMsgToSend(rMsg);

			}
			it = m_EventList.erase(it);
		}
		else {
			it++;
		}
	}
}

void CSpacePortal::WriteLogDB(TCHAR* Format, tstring& s){
	if (!m_bWriteLogToDatabase)
	{
		return;
	}
	int64 TimeStamp = AbstractSpace::CreateTimeStamp();
	m_LogDB.WriteItem(TimeStamp,_T("SpacePortal"),s);
};

void CSpacePortal::GetLinker(int64 SourceID,CLinker& Linker){
	GetClientLinkerList()->GetLinker(SourceID,Linker);
	if (!Linker.IsValid())
	{
		GetSuperiorLinkerList()->GetLinker(SourceID,Linker);
	}
};

void  CSpacePortal::OutputLog(uint32 Type,const TCHAR* text){
	if(m_LogFlag & Type){
		GetGUI()->AddRTInfo(text);
	}

};



void  CSpacePortal::NotifyLinkerState(int64 SourceID,int64 NotifyID,STATE_OUTPUT_LEVEL Flag,ePipeline& Info){

	switch(Flag){
	case WEIGHT_LEVEL:
		{
			switch(NotifyID)
			{
			case LINKER_RECEIVE_STEP:
				{
					//目前没有处理必要
					int64 DataSize = Info.PopInt();
					int64 ParentSize  = Info.PopInt();
					ePipeline* Data = (ePipeline*)Info.GetData(0);
				}
				return;
			case LINKER_SEND_STEP:
				{
					int64 MsgID   = Info.PopInt();
					int64 MsgSize = Info.PopInt();
					int64 SendNum = Info.PopInt();

					int64 Per = SendNum/MsgSize*100;

					tstring MsgName = MsgID2Str(MsgID);				
				}
				return;
			}
		} //注意这里不要break,
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

					tstring s = Format1024(_T("LINKER_PUSH_MSG: %s EventID:%I64ld PendingMsg=%s CurMsgNum=%I64ld"),MsgName.c_str(),EventID,PendingMsgName.c_str(),MsgNum+UrgMsgNum);
					OutputLog(LOG_MSG_IO_PUSH,s.c_str());

					//PrintMsg(MsgID,true);
				}
				return;
			case LINKER_MSG_SENDED:
				{		

					assert (Info.Size());

					ePipeline* Msg = (ePipeline*)Info.GetData(0);
					if (Msg->GetID()<100)
					{
						//OutputLog(LOG_MSG_IO_SENDED,_T("LINKER_MSG_SENDED: internal feedback send"));
					}else{
						CMsg SendMsg;
						Info.PopMsg(SendMsg);
						int64 MsgID = SendMsg.GetMsgID();
						tstring MsgName = MsgID2Str(MsgID);
						tstring s  = Format1024(_T("LINKER_MSG_SENDED: MsgID:%s  EventID:%I64ld"),MsgName.c_str(),SendMsg.GetEventID());
						OutputLog(LOG_MSG_IO_SENDED,s.c_str());
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
							tstring s = Format1024(_T("LINKER_MSG_RECEIVED: Remote receive %s fail"),PendingMsgName.c_str());

							OutputLog(LOG_MSG_IO_REMOTE_RECEIVED,s.c_str());
							//AddRTInfo(s.c_str());
						} 
						else
						{	
							tstring s = Format1024(_T("LINKER_MSG_RECEIVED: Remote received msg:%s ok"),PendingMsgName.c_str());
							OutputLog(LOG_MSG_IO_REMOTE_RECEIVED,s.c_str());
							
						}
					} 
					else
					{	
						tstring MsgName = MsgID2Str(MsgID);
						tstring s = Format1024(_T("LINKER_MSG_RECEIVED:Msg received: %s ok"),MsgName.c_str());
						OutputLog(LOG_MSG_I0_RECEIVED,s.c_str());
						//AddRTInfo(s.c_str());
					}	
				}
				return;
			}
		}
	case NORMAL_LEVEL:
		{
			switch(NotifyID)
			{
			case LINKER_BEGIN_ERROR_STATE:
				{
					int64 ErrorType = Info.PopInt();
					eElectron CurRevMsg;
					Info.Pop(&CurRevMsg);
					AnsiString text;
					CurRevMsg.ToString(text);

					tstring s = Format1024(_T("LINKER_BEGIN_ERROR: SourceID=%I64ld ErrorType=%I64ld\n CurrentRevMsg:%s \n"),SourceID,ErrorType,UTF8toWS(text).c_str());

					OutputLog(LOG_ERROR,s.c_str());
					//AddRTInfo(s.c_str());
				}
				return;
			case LINKER_END_ERROR_STATE:
				{
					tstring s = Format1024(_T("LINKER_END_ERROR: SourceID=%I64ld"),SourceID);
					OutputLog(LOG_TIP,s.c_str());
					//AddRTInfo(s.c_str());
				}
				return;
			case LINKER_INVALID_ADDRESS:
				{
					CMsg Msg;
					Info.PopMsg(Msg);
					int64 MsgID = Msg.GetMsgID();
					tstring MsgName = MsgID2Str(MsgID);

					tstring s = Format1024(_T("LINKER_INVALID_ADDRESS: SourceID=%I64ld MsgID:%s "),SourceID,MsgName.c_str());

					OutputLog(LOG_WARNING,s.c_str());
					//AddRTInfo(s.c_str());
				}
				return;
			case LINKER_ILLEGAL_MSG:
				{
					ePipeline* CurRevMsg = (ePipeline*)Info.GetData(0);

					AnsiString text;
					CurRevMsg->ToString(text);

					tstring s = Format1024(_T("LINKER_ILLEGAL_MSG: SourceID=%I64ld Msg:%s "),SourceID,UTF8toWS(text).c_str());

					OutputLog(LOG_WARNING,s.c_str());
					//AddRTInfo(s.c_str());	
				}
				return;
			case LINKER_EXCEPTION_ERROR:
			case LINKER_IO_ERROR:
				{
					int32 RecoType = Info.PopInt();
					int32 LinkerType = Info.PopInt();

					if (NotifyID==LINKER_EXCEPTION_ERROR)
					{
						tstring Error = Info.PopString();
						OutputLog(LOG_WARNING,Format1024(_T("LINKER_EXCEPTION_ERROR:%s SourceID=%I64ld was closed"),Error.c_str(),SourceID).c_str());
					} 
					else
					{
						OutputLog(LOG_WARNING,Format1024(_T("LINKER_IO_ERROR: SourceID=%I64ld may be closed by remote"),SourceID).c_str());
					}
					

					LinkerBreakEventProc(SourceID);

					//由于SpacePort负责帮Client执行Executer，所以其中一方关闭都要通知另一方（不会引起锁死）
					if (RecoType != LINKER_ORGAN)//not Executer linker
					{   
						CheckoutVisitor(SourceID);
					}

					if (LinkerType  == CLIENT_LINKER)
					{
						bool ret = m_ClientLinkerList.DeleteLinker(SourceID);
						if(!ret)return; //链接并不存在

					}else /*if(Type == SERVER_LINKER)*/{
						bool ret = m_SuperiorList.DeleteLinker(SourceID);
						if(!ret)return; //链接并不存在
					}
				}
				return;
			
		
			case LINKER_CONNECT_ERROR:
				{
					tstring Error = Info.PopString();
					tstring s = Format1024(_T("LINKER_CONNECT_ERROR: SourceID=%I64ld %s "),SourceID,Error.c_str());

					OutputLog(LOG_WARNING,s.c_str());
					//AddRTInfo(s.c_str());	

					//assert(Linker->GetLinkerType()== SERVER_LINKER);
					GetSuperiorLinkerList()->DeleteLinker(SourceID);
				}
				return;
			}
		} 
		break;
	default:
		break;
	}

};


void  CSpacePortal::OnTaskFeedback(CMsg& Msg)
{
	int64 ExecuterID  = Msg.GetSourceID();
    int64 SpaceEventID = Msg.GetEventID();
    ePipeline& Letter = Msg.GetLetter(true);
	
	assert (SpaceEventID!=0);

	CSpaceEvent SpaceEvent;
	bool ret = GetEvent(SpaceEventID,SpaceEvent);
	if (!ret)
    {
		assert(0);
		return;
    }

	//assert(SpaceEvent.m_bIsBusy); 启动对话出错时也会反馈
	SpaceEvent.m_bIsBusy= false;
	ModifyEvent(SpaceEventID,SpaceEvent);

	CLinker ClientLinker;
	GetClientLinkerList()->GetLinker(SpaceEvent.m_ClientLinkerID,ClientLinker);
	if (ClientLinker.IsValid())
	{
		CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,SpaceEvent.m_ClientEventID);
		ePipeline& rLetter = FeedbackMsg.GetLetter(false);
		rLetter << Letter;  
		ClientLinker().PushMsgToSend(FeedbackMsg);
	}
}


void CSpacePortal::OnI_AM(CMsg& Msg){

	int64 SourceID = Msg.GetSourceID();
    
	CLinker Who;
	m_ClientLinkerList.GetLinker(SourceID,Who);
    if (!Who.IsValid())
    {
		return;
    }

	ePipeline& Letter = Msg.GetLetter(true);
	
	if(!Letter.HasTypeAB(PARAM_TYPE3(TYPE_INT,TYPE_STRING,TYPE_STRING)) ){
		assert(0);
		GetClientLinkerList()->DeleteLinker(SourceID);
		return ;
	};

	int64    ID          = Letter.PopInt();
	tstring  Name        = Letter.PopString();  
	tstring  Cryptograhp = Letter.PopString();  
		
    if (ID>0)//From object
	{
		CSpaceEvent SpaceEvent;
		bool ret = GetEvent(ID,SpaceEvent);

		if(!ret){
			m_ClientLinkerList.DeleteLinker(SourceID);
		}

		Who().SetRecoType(LINKER_ORGAN);
		
		CMsg rMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_CONNECT_OK,DEFAULT_DIALOG,0);
		rMsg.GetLetter(false).PushString(GetName());
		Who().PushMsgToSend(rMsg);

		
		//给请求外部物体的robot回信物体启动成功
		CLinker Requester;
		m_ClientLinkerList.GetLinker(SpaceEvent.m_ClientLinkerID,Requester);

		ePipeline& ExePipe = *(ePipeline*)SpaceEvent.m_EventData.GetData(0);
		ExePipe.PushInt(SourceID); //外部物体的SourceID作为其实例ID

		CMsg FeedbackMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,SpaceEvent.m_ClientEventID);
		ePipeline& Letter = FeedbackMsg.GetLetter(false);

		Letter.PushPipe(ExePipe);  
		Requester().PushMsgToSend(FeedbackMsg);

		PopEvent(ID);
		return;
	}

	bool Delete = false;
	
	//Try to login
	//NOTE: In fact it may be a outer space required a connection, but here to ignore 
    bool ret = false;
	
	People*  p = CheckinVisitor(Who().GetSourceID(),Name,Cryptograhp);	
	if (!p)
	{
		Who().Close();
		return;
	}
	try
    {	
		ROOM_SPACE& RootSpace = GetRootSpace();
		ret = RootSpace.Logon(SourceID,*p);   	
    }catch (...) {
		ret = false;
    }

 	if(!ret){
		tstring info = Format1024(_T("The linker(%s) be closed"),Name.c_str());
		OutputLog(LOG_WARNING,info.c_str());
		CheckoutVisitor(Who().GetSourceID());
		GetClientLinkerList()->DeleteLinker(SourceID);
		return;
	}  

	CMsg rMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_CONNECT_OK,DEFAULT_DIALOG,0);
	rMsg.GetLetter(false).PushString(GetName());
	Who().PushMsgToSend(rMsg);
	
}
  
void CSpacePortal::PrintMsg(CMsg& Msg,bool Send){
	int64 MsgID = Msg.GetMsgID();
	int64 SourceID = Msg.GetSourceID();

	tstring s = MsgID2Str(MsgID);
	if (Send)
	{
		s = Format1024(_T("Send : %s\n to %I64ld"),s.c_str(),SourceID);
		OutputLog(LOG_TIP,s.c_str());
	} 
	else
	{
		s = Format1024(_T("Get  : %s from %I64ld"),s.c_str(),SourceID);
		OutputLog(LOG_TIP,s.c_str());
	}		
}

void  CSpacePortal::CreateObject(int64 ParentID,ePipeline& Letter){
	tstring ObjectDllName = Letter.PopString(); //dll file name
	tstring FilePath = Letter.PopString(); //zip file path

	ePipeline* SpacePath = (ePipeline*)Letter.GetData(0);


	//Saved to  the specified location
	tstring SpaceDir = SpacePath2FileAddress(*SpacePath);

	tstring ObjectName = GetFileName(ObjectDllName);

	_tmkdir(SpaceDir.c_str());
	SpaceDir += _T("\\")+ObjectName;
	_tmkdir(SpaceDir.c_str());

	SpaceDir += _T("\\");

	//Extract to the specified directory
	HZIP hz = OpenZip(FilePath.c_str(),0);
	ZIPENTRY ze; GetZipItem(hz,-1,&ze); int numitems=ze.index;
	// -1 gives overall information about the zipfile
	for (int zi=0; zi<numitems; zi++)
	{
		ZIPENTRY ze; GetZipItem(hz,zi,&ze); // fetch individual details
		tstring FileName = SpaceDir+ze.name;
		UnzipItem(hz, zi, FileName.c_str());         // e.g. the item's name.
	}
	CloseZip(hz);


	//计算dll的crc32
	DWORD  			dwBytesRead ; 
	HANDLE 		    hFile ;
	int    			iFileLength;
	PBYTE  			pBuffer;	
	// Open the file.
	tstring DLLFilePath = SpaceDir + ObjectDllName;

	if (INVALID_HANDLE_VALUE == 
		(hFile = CreateFileW (DLLFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL))){
			return;
	}

	iFileLength = GetFileSize (hFile, NULL) ; 
	pBuffer = (PBYTE)malloc (iFileLength) ;

	// Read file and put terminating zeros at end.
	ReadFile (hFile, pBuffer, iFileLength, &dwBytesRead, NULL) ;
	CloseHandle (hFile) ;

	//VMCrc32 Crc32;
	//DWORD crc = Crc32.CalcCRC(pBuffer,iFileLength);

	Checksum  Crc(Checksum::TYPE_CRC32);
	Crc.update((const char*)pBuffer,iFileLength);
	DWORD crc = Crc.checksum();
	//create item
	int64 ChildID = AbstractSpace::CreateTimeStamp();

	tstring Fingerprint = Format1024(_T("%x"),crc);
	ROOM_SPACE NewObject(ParentID,ChildID,DLLFilePath.c_str(),iFileLength, OBJECT_SPACE,FREE,Fingerprint);

	//Notify all current CLinker update room info
	vector<int64> VisiterList;
	GetAllVisiter(ParentID,VisiterList);

	for(uint32 i=0; i<VisiterList.size(); i++){
		int64 WhoID = VisiterList[i];
		CLinker Linker;
		GetLinker(WhoID,Linker);
		if (!Linker.IsValid())
		{
			continue;
		}
		People* Owner = GetVisitor(WhoID);
		assert(Owner != NULL);
		assert(Owner->IsValid());

		ePipeline ChildPipe(Owner->GetParentID());
		ChildPipe.PushString(ObjectName);
		ChildPipe.PushInt(OBJECT_SPACE);
		ChildPipe.PushString(Fingerprint);

		CMsg Msg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_SPACE_ADDED,DEFAULT_DIALOG,0);
		Msg.GetLetter(false).PushPipe(ChildPipe);		
		Linker().PushMsgToSend(Msg);	 
	}

};

void CSpacePortal::CentralNerveMsgProc(CMsg& Msg){
	int64 MsgID = Msg.GetMsgID();

	PrintMsg(Msg,false);

	switch(MsgID){
	case MSG_TASK_FEEDBACK:
		OnTaskFeedback(Msg);
		break;
	case MSG_WHO_ARE_YOU:
		break;
	case MSG_I_AM: 
		OnI_AM(Msg);
		break;
	default:
		{
			PushNerveMsg(Msg,false,false);
		}
	}	
}

