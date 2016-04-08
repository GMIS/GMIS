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

DLL_TYPE Dll_Object::GetDllType(){
	if(m_hDll == NULL)return DLL_INVALID;

	pGetObjectType pFunc = NULL;
	pFunc = ( pGetObjectType) GetProcAddress(m_hDll, "GetDllType");
	
	if(pFunc ==NULL )return DLL_INVALID;
	
    DLL_TYPE Type =(pFunc)();
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
	DeleteAllExecuter();

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

CExecuter*  CSpacePortal::AddExecuter(DLL_TYPE Type,int64 ExecuterLinkerID){
	_CLOCK(&m_Mutex);
	CExecuter* Executer = new  CExecuter(Type,ExecuterLinkerID);
	if(!Executer)return NULL;

	m_ExecuterList[Type] = Executer;
	return Executer;
};
CExecuter*  CSpacePortal::FindExecuter(DLL_TYPE Type){
	_CLOCK(&m_Mutex);
	map<int32,CExecuter*>::iterator it = m_ExecuterList.find(Type);
	if (it != m_ExecuterList.end())
	{
		CExecuter* Executer = it->second;
		return Executer;
	}
	return NULL;
};
CExecuter*  CSpacePortal::FindExecuterByLinker(int64 ExecuterLinkerID){
	_CLOCK(&m_Mutex);
	map<int32,CExecuter*>::iterator it = m_ExecuterList.begin();
	while (it != m_ExecuterList.end())
	{
		CExecuter* Executer = it->second;
		if (Executer->m_ID == ExecuterLinkerID)
		{
			return Executer;
		}
		it++;
	}
	return NULL;
};

void CSpacePortal::DeleteExecuter(int64 ExecuterLinkerID){
	_CLOCK(&m_Mutex);
	map<int32,CExecuter*>::iterator it = m_ExecuterList.begin();
	while (it != m_ExecuterList.end())
	{
		CExecuter* Executer = it->second;
		if (Executer->m_ID = ExecuterLinkerID)
		{
			delete Executer;
			m_ExecuterList.erase(it);
			break;
		}
		it++;
	}
};
void CSpacePortal::DeleteAllExecuter(){
	_CLOCK(&m_Mutex);
	map<int32,CExecuter*>::iterator it = m_ExecuterList.begin();
	while (it != m_ExecuterList.end())
	{
		CExecuter* Executer = it->second;
		delete Executer;
		it++;
	}
	m_ExecuterList.clear();
};
/*
void  CSpacePortal::PushExecuterEvent(int64 ExecuterID,int64 RobotID,int64 EventID){
	assert(ExecuterID);
	assert(RobotID);
	assert(EventID);

	_CLOCK(&m_Mutex);
	map<int64,int64>& EventList = m_ExecuterEventList[ExecuterID];
	EventList[EventID] = RobotID;
};
int64 CSpacePortal::PopExecuterEvent(int64 ExecuterID,int64 EventID){
	assert(ExecuterID);
	assert(EventID);
	
	_CLOCK(&m_Mutex);
	map<int64,map < int64,int64 > >::iterator it = m_ExecuterEventList.find(ExecuterID);
	if(it == m_ExecuterEventList.end()){
		return 0;
	}
	
	map<int64,int64>& EventList = it->second;
	map < int64,int64 >::iterator it2 = EventList.find(EventID);
	if (it2 == EventList.end())
	{
		return 0;
	}

	int64 RobotID = it2->second;
	EventList.erase(it2);

	assert(RobotID);
	return RobotID;
}

void  CSpacePortal::RegisterExecuterUser(int64 RobotID,int64 ExecuterID){
	assert(ExecuterID);
	assert(RobotID);

	_CLOCK(&m_Mutex);
	set < int64 >& ExecuterList = m_ExecuterUserList[RobotID];
	ExecuterList.insert(ExecuterID);
}
*/
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
/*
void   CSpacePortal::UserLinkerClosedProc(int64 RobotID)
{
	_CLOCK(&m_Mutex);
	
	//According to RobotID find  all  ExecuterID
    map<int64,set < int64 > >::iterator it = m_ExecuterUserList.find(RobotID);
    if (it == m_ExecuterUserList.end())
    {
		return;
    }

	//And then notify each ExecuterID close object
	set < int64 >& ExecutIDList = it->second;
	set < int64 >::iterator ExecuterIt = ExecutIDList.begin();
	while(ExecuterIt != ExecutIDList.end())
	{
		int64 ExecuterID = *ExecuterIt;
		
		//Delete this ExecuterID involved in all events
		m_ExecuterEventList.erase(ExecuterID);
		
		CLinker ExecuterLinker;
		GetLinker(ExecuterID,ExecuterLinker);

		//To notify executer stay of execution, here EventID=0
		CMsg NewMsg(MSG_OBJECT_CLOSE,NULL,0);
		ePipeline& NewLetter = NewMsg.GetLetter();

		ePipeline ExePipe;
		NewLetter.PushPipe(ExePipe);

		ExecuterLinker().PushMsgToSend(NewMsg);


		ExecuterIt++;
	}

	//Delete this RobotID registry records
    m_ExecuterUserList.erase(it);

};


void   CSpacePortal::ExecuteLinkerClosedProc(int64 ExecuterID){
	_CLOCK(&m_Mutex);

	//send feedback for executer execution fail
    map< int64, map < int64, int64 > >::iterator it = m_ExecuterEventList.find(ExecuterID);
    if (it == m_ExecuterEventList.end())
    {
		return;
    }
	
	map < int64,int64 >& EventList = it->second;
	map < int64,int64 >::iterator it2 = EventList.begin();

	while(it2 != EventList.end()){
		int64 EventID = it2->first;
		int64 RobotID = it2->second;

		CLinker RobotLinker;
		GetLinker(RobotID,RobotLinker);

		if (RobotLinker.IsValid())
		{
			ePipeline ExePipe(RETURN_ERROR);
			ExePipe.GetLabel() = Format1024(_T("Object running be breaked;"));

			CMsg FeedbackMsg(MSG_TASK_FEEDBACK,NULL,EventID);
			ePipeline& rLetter = FeedbackMsg.GetLetter();
			rLetter.PushPipe(ExePipe);  
			RobotLinker().PushMsgToSend(FeedbackMsg);
		}
		it2++;
	}	

	m_ExecuterEventList.erase(it);

	//delete invalid executer （Not sure whether it is necessary to）
	map<int64 ,set < int64 > >::iterator it3 = m_ExecuterUserList.begin();
	while (it3 != m_ExecuterUserList.end())
	{
		set < int64 >& ExecuterList = it3->second;
		ExecuterList.erase(ExecuterID);
		it3++;
	}
	
}
*/
void   CSpacePortal::UserLinkerClosedProc(int64 RobotID)
{
	_CLOCK(&m_Mutex);
	map<int32,CExecuter*>::iterator it = m_ExecuterList.begin();
	while(it != m_ExecuterList.end()){
		CExecuter* Executer = it->second;
		
		CLinker ExecuterLinker;
		GetLinker(Executer->m_ID,ExecuterLinker);
	
		map<int64, CSpaceEvent>::iterator ita = Executer->m_EventList.begin();
		while (ita != Executer->m_EventList.end())
		{
			int64 SpaceEventID = ita->first;
			CSpaceEvent& Event = ita->second;
			if(Event.m_ClientLinkerID == RobotID){

				//To notify executer stay of execution, here EventID=0
				CMsg NewMsg(MSG_OBJECT_CLOSE,DEFAULT_DIALOG,SpaceEventID);
				ePipeline& NewLetter = NewMsg.GetLetter();

				ePipeline ExePipe;
				NewLetter.PushPipe(ExePipe);
				ExecuterLinker().PushMsgToSend(NewMsg);
			}
			ita++;
		}
		it++;
	}
}
void   CSpacePortal::ExecuteLinkerClosedProc(int64 ExecuterID){

	CExecuter* Executer = FindExecuterByLinker(ExecuterID);
	if(Executer == NULL)return;

	
	_CLOCK(&m_Mutex);
	map<int64, CSpaceEvent>::iterator it = Executer->m_EventList.begin();

	while (it != Executer->m_EventList.end())
	{
		int64 SpaceEventID = it->first;
		CSpaceEvent& Event = it->second;
		CLinker RobotLinker;
		GetLinker(Event.m_ClientLinkerID,RobotLinker);

		if (RobotLinker.IsValid())
		{
			ePipeline ExePipe(RETURN_ERROR);
			ExePipe.GetLabel() = Format1024(_T("lost executer connection;"));

			CMsg FeedbackMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,Event.m_ClientEventID);
			ePipeline& rLetter = FeedbackMsg.GetLetter();
			rLetter.PushPipe(ExePipe);  
			RobotLinker().PushMsgToSend(FeedbackMsg);
		}

		it++;
	}
	Executer->m_EventList.clear();
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

					PrintMsg(MsgID,true);
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
				{
					int32 RecoType = Info.PopInt();
					int32 LinkerType = Info.PopInt();

					tstring Error = Info.PopString();
					tstring s = Format1024(_T("LINKER_EXCEPTION_ERROR:%s SourceID=%I64ld was closed"),Error.c_str(),SourceID);

					OutputLog(LOG_WARNING,s.c_str());

					//由于SpacePort负责帮Client执行Executer，所以其中一方关闭都要通知另一方（不会引起锁死）
					if (RecoType == LINKER_ORGAN)//Executer linker
					{   
						ExecuteLinkerClosedProc(SourceID);
					}else{ 
						CheckoutVisitor(SourceID);
						UserLinkerClosedProc(SourceID);
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
			case LINKER_IO_ERROR:
				{
					int32 RecoType = Info.PopInt();
					int32 LinkerType = Info.PopInt();

					//通常是远端关闭
					tstring s = Format1024(_T("LINKER_IO_ERROR: SourceID=%I64ld may be closed by remote"),SourceID);

					OutputLog(LOG_WARNING,s.c_str());

					//由于SpacePort负责帮Client执行Executer，所以其中一方关闭都要通知另一方（不会引起锁死）
					if (RecoType == LINKER_ORGAN)//Executer linker
					{   
						ExecuteLinkerClosedProc(SourceID);
					}else{ 
						CheckoutVisitor(SourceID);

						UserLinkerClosedProc(SourceID);
					}

					if (LinkerType == CLIENT_LINKER)
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


void  CSpacePortal::OnObjectFeedback(CMsg& Msg)
{
	int64 ExecuterID  = Msg.GetSourceID();
    int64 SpaceEventID = Msg.GetEventID();
    ePipeline& Letter = Msg.GetLetter();
	
	assert (SpaceEventID!=0);

	CExecuter* Executer = FindExecuterByLinker(ExecuterID);
	assert (Executer);
    
	CSpaceEvent SpaceEvent;
	bool ret = Executer->PopEvent(SpaceEventID,SpaceEvent);
    assert(ret);
	if (!ret)
    {
		return;
    }

	CLinker RobotLinker;
	GetLinker(SpaceEvent.m_ClientLinkerID,RobotLinker);
	if (RobotLinker.IsValid())
	{
		CMsg FeedbackMsg(MSG_TASK_FEEDBACK,DEFAULT_DIALOG,SpaceEvent.m_ClientEventID);
		ePipeline& rLetter = FeedbackMsg.GetLetter();
		rLetter << Letter;  
		RobotLinker().PushMsgToSend(FeedbackMsg);
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

	int64 EventID = Msg.GetEventID();
	ePipeline& Letter = Msg.GetLetter();

	Letter.AutoTypeAB();	
	if(Letter.GetTypeAB() != 0x33100000 ){ //two string+one ID
		assert(0);
		GetClientLinkerList()->DeleteLinker(SourceID);
		return ;
	};

	tstring  Name        = Letter.PopString();  
	tstring  Cryptograhp = Letter.PopString();  
		
    if (Name == _T("executer"))//From executer
	{
		Who().SetRecoType(LINKER_ORGAN);
		
		CMsg rMsg(MSG_CONNECT_OK,DEFAULT_DIALOG,0);
		rMsg.GetLetter().PushString(GetName());
		Who().PushMsgToSend(rMsg);

		int64 ExecuterLinkerID =  Who().GetSourceID();
		int64 Type = Letter.PopInt();

		CExecuter* Executer = FindExecuter((DLL_TYPE)Type);
		if (Executer) 
		{
			Executer->m_ID = ExecuterLinkerID;
			Executer->ProcessPendingEvent(Who);
		}else{//手动启动
			Executer = AddExecuter((DLL_TYPE)Type,ExecuterLinkerID);
		}
		return;
	}

	bool Delete = false;
	
	//Try to login
	//NOTE: In fact it may be a outer space to  require a connection, but here to ignore 
    bool ret = false;
	
	People*  p = CheckinVisitor(Who().GetSourceID(),Name,Cryptograhp);
	
	if (!p)
	{
		Who().Close();
		return;
	}
	try
    {	
		ROOM_SPACE& RootRoom = GetRootRoom();
		ret = RootRoom.Logon(SourceID,*p);   	
    }catch (...) {

    }

 	if(!ret){
		tstring info = Format1024(_T("The linker(%s) be closed"),Name.c_str());
		OutputLog(LOG_WARNING,info.c_str());
		delete p;
		GetClientLinkerList()->DeleteLinker(SourceID);
		return;
	}  

	CMsg rMsg(MSG_CONNECT_OK,DEFAULT_DIALOG,0);
	rMsg.GetLetter().PushString(GetName());
	Who().PushMsgToSend(rMsg);
	
}
   
void CSpacePortal::OnGotoSpace(CMsg& Msg){

	int64 SourceID = Msg.GetSourceID();

	CLinker Who;
	GetLinker(SourceID,Who);
    if (!Who.IsValid())
    {
		return;
    }
	
	People* Robot = GetVisitor(SourceID);
	if(Robot == NULL){
		Who().Close();	
		return;
	};


	ePipeline& Letter = Msg.GetLetter();
	
	SpaceAddress Addr;

	ePipeline* Path = (ePipeline*)Letter.GetData(0);

	int64 SpaceID = Path->GetID();


	Addr = ::FindSpace(*Path);
	if (!Addr.IsValid())
	{
		CMsg rMsg(MSG_SPACE_ACCESS_DENIED,NULL,NULL);
		
		ePipeline& rLetter = rMsg.GetLetter();
		rLetter.PushInt(SpaceID);
		rLetter.PushString(_T("Address invalid"));
		Who().PushMsgToSend(rMsg);	
		return;
	};
	
	
	ROOM_SPACE Room(Addr.ParentID,Addr.ChildID);
	
	if(Room.AllowInto(*Robot)){
		Robot->GoInto(SourceID,Room);
	}else{
		CMsg rMsg(MSG_SPACE_ACCESS_DENIED,NULL,NULL);
		ePipeline& rLetter = rMsg.GetLetter();
		rLetter.PushInt(SpaceID);
		rLetter.PushString(_T("right invalid"));
		
		Who().PushMsgToSend(rMsg);	
		return;
	}
	

	ePipeline Pipe;
	GetAllChildList(Robot->GetParentID(),Pipe,Robot->GetSpaceID());
		
    if (Addr.ChildID == LOCAL_SPACEID)
    {
		tstring Name = GetRootRoom().GetName();
		
		Pipe.SetLabel(Name.c_str());
    }

	CMsg rMsg(MSG_SPACE_CATALOG,NULL,NULL);
	ePipeline& rLetter = rMsg.GetLetter();

	Pipe.SetID(SpaceID); //Inner space ID
	rLetter.PushPipe(Pipe);
    Who().PushMsgToSend(rMsg);	

	//Notify the other CLinkers of this space , Send  MSG_ADD_SPACE
	vector<int64> SiblingList;
	GetAllVisiter(Robot->GetParentID(),SiblingList,Robot->GetSpaceID());
	for(uint32 i=0; i<SiblingList.size(); i++){
		int64 WhoID = SiblingList[i];

		CLinker Linker;
		GetLinker(WhoID,Linker);
		if (!Linker.IsValid())
		{
			continue;
		}

		People* Owner = GetVisitor(WhoID);
		assert(Owner != NULL);
        assert(Owner->IsValid());
		
		ePipeline* ChildPipe  = new ePipeline(Owner->GetParentID());

		tstring Name = Robot->GetName();
		//Name = GetFileNoPathName(Name);
		ChildPipe->PushString(Name);
		ChildPipe->PushInt(ROBOT_VISITER);
		ChildPipe->PushString(_T("")); //crc

		CMsg rMsg(MSG_SPACE_ADDED,NULL,NULL);
		ePipeline& rLetter = rMsg.GetLetter();
		rLetter.Push_Directly(ChildPipe);

        Linker().PushMsgToSend(rMsg);	
	}
}
   
void  CSpacePortal::OnAskforSpaceBody(CMsg& Msg){
	
	int64 SourceID = Msg.GetSourceID();
    
	CLinker Who;
	GetLinker(SourceID,Who);
    if (!Who.IsValid())
    {
		return;
    }
	
	ePipeline& Letter = Msg.GetLetter();

	int64 FatherID = Letter.PopInt();
	int64 ChildID  = Letter.PopInt();
	
	CMsg rMsg(MSG_SPACE_SEND_OBJECT,NULL,NULL);
	ePipeline& rLetter = rMsg.GetLetter();
	rLetter.PushInt(FatherID);
	rLetter.PushInt(ChildID);	
	
	/*
	ROOM_SPACE r(FatherID,ChildID);
	
	bool ret = r.LoadModelData();
          
	if(ret){
		CppSQLite3Binary Blob;
		Blob.setBinary(r.m_ModelData,r.m_ModelDataSize);
		const char* sBody = (const char*)Blob.getEncoded();
		assert(0); //等待修改
	//	eSTRING* Body  = new eSTRING(sBody);
	//	rLetter->Push_Directly(Body);

	}else{
		eSTRING* Body  = new eSTRING(_T(""));
		rLetter.Push_Directly(Body);
	}
	rLetter.PushString(r.m_CrcStr);
*/
    Who().PushMsgToSend(rMsg);	
}
	
void  CSpacePortal::OnCreateSpace(CMsg& Msg){
	int64 SourceID = Msg.GetSourceID();
    
	CLinker Who;
	GetLinker(SourceID,Who);
    if (!Who.IsValid())
    {
		return;
    }

	People* Robot = GetVisitor(SourceID);
	assert(Robot);

	int64 ParentID = Robot->GetParentID();

	ePipeline& Letter = Msg.GetLetter();
	SPACETYPE  Type = (SPACETYPE)Letter.PopInt();    

    if(Type == OBJECT_SPACE)
	{
		CreateObject(ParentID,Letter);
    }else{
        CreateRoom(ParentID,Type,Letter);
    }
	
};

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
	ROOM_SPACE NewObject(ParentID,ChildID,DLLFilePath.c_str(),iFileLength, OBJECT_SPACE,NO_RIGHT,Fingerprint);
		
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
		
		CMsg Msg(MSG_SPACE_ADDED,NULL,NULL);
		Msg.GetLetter().PushPipe(ChildPipe);		
		Linker().PushMsgToSend(Msg);	 
	}
		
};


void CSpacePortal::CreateRoom(int64 ParentID,SPACETYPE RoomType,ePipeline& Letter){

	tstring RoomName  = Letter.PopString();

	int64 ChildID = AbstractSpace::CreateTimeStamp();
	
	tstring Fingerprint = Format1024(_T("%I64ld"),ChildID);
	ROOM_SPACE Room(ParentID,ChildID,RoomName.c_str(),0,RoomType,NO_RIGHT,Fingerprint);

	//Notify all current CLinker update room info
	vector<int64> SiblingList;
	GetAllVisiter(ParentID,SiblingList);
	for(uint32 i=0; i<SiblingList.size(); i++){
		int64 WhoID = SiblingList[i];
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
		ChildPipe.PushString(RoomName);
		ChildPipe.PushInt(RoomType);
		ChildPipe.PushString(Fingerprint);

		CMsg Msg(MSG_SPACE_ADDED,NULL,NULL);

		Msg.GetLetter().PushPipe(ChildPipe);		
		Linker().PushMsgToSend(Msg);	 
	}
}

void CSpacePortal::OnDeleteSpace(CMsg& Msg)
{
	int64 SourceID = Msg.GetSourceID();
    
	CLinker Who;
	GetLinker(SourceID,Who);
    if (!Who.IsValid())
    {
		return;
    }
	
	ePipeline& Letter = Msg.GetLetter();

	ePipeline Path = *(ePipeline*)Letter.GetData(0);
	tstring Fingerprint = *(tstring*)Letter.GetData(1);

	int64 SpaceID = Path.GetID(); //GUI map item Id

	SpaceAddress Addr = FindChildSpace(Path,Fingerprint);

	if (!Addr.IsValid())
	{
		return;
	}

	CSpace Space(Addr.ParentID,Addr.ChildID);
	if (!Space.IsValid())
	{
		return ;
	}

	tstring Name = Space.GetName();
	
	//Deletes the specified directory
	tstring Dir = GetFilePath(Name);

	DeleteDir(Dir.c_str());

	SPACETYPE Type = Space.GetSpaceType();
	if (Type != CONTAINER_SPACE && Type != OBJECT_SPACE && Type != LOCAL_SPACE)	
	{
		return;
	}

	DeleteChild(Addr.ParentID,Addr.ChildID);

	Path = *(ePipeline*)Letter.GetData(0);

	//feedback
	CMsg rMsg(MSG_SPACE_DELETED,NULL,NULL);
	ePipeline& rLetter = rMsg.GetLetter();
	rLetter.PushPipe(Path);
	rLetter.PushString(Name);
	rLetter.PushInt(Type);
	rLetter.PushString(Fingerprint);

	Who().PushMsgToSend(rMsg);	

	//Notify all current CLinker update room info
	People* p = GetVisitor(SourceID);
	assert(p);
	vector<int64> SiblingList;
	GetAllVisiter(p->GetParentID(),SiblingList,p->GetSpaceID());
	for(uint32 i=0; i<SiblingList.size(); i++){
		int64 WhoID = SiblingList[i];
		
		Path = *(ePipeline*)Letter.GetData(0);
		
		CMsg rMsg1(MSG_SPACE_DELETED,NULL,NULL);

		ePipeline& rLetter1 = rMsg1.GetLetter();
		rLetter1.PushPipe(Path);
		rLetter1.PushString(Name);
		rLetter1.PushInt(Type);
		rLetter1.PushString(Fingerprint);

		CLinker Who1;
		GetLinker(WhoID,Who1);
		if (!Who1.IsValid())
		{
			continue;;
		}
		Who1().PushMsgToSend(rMsg1);	
	}
}


void CSpacePortal::OnExportObject(CMsg& Msg){
	int64 SourceID = Msg.GetSourceID();
    
	CLinker Who;
	GetLinker(SourceID,Who);
    if (!Who.IsValid())
    {
		return;
    }

	ePipeline& Letter = Msg.GetLetter();

	int64  FatherID = Letter.PopInt();
	int64  ChildID  = Letter.PopInt();

	tstring FileName;	 
    Letter.PopString(FileName);
	
    int64 Success=FALSE;

	/*
	CSpace R(FatherID, ChildID);
	if(R.LoadModelData()){
		if(R.SaveModelAsFile(FileName))Success = 1;
		else Success = 0;
	}
   */
	//feedback
	int64 EventID = Msg.GetEventID();
	CMsg rMsg(MSG_OPERATOR_FEEDBACK,NULL,EventID);
	rMsg.GetLetter().PushInt(Success);
	rMsg.GetReceiver() << Msg.GetSender();
	Who().PushMsgToSend(rMsg);
}

void CSpacePortal::OnTaskRequest(CMsg& Msg){
	int64 SourceID = Msg.GetSourceID();

	int64 EventID = Msg.GetEventID();
	ePipeline& Letter = Msg.GetLetter();
    ePipeline& RequestInfo = *(ePipeline*)Letter.GetData(0);
    int64 RequestID = RequestInfo.GetID();
	switch(RequestID){
	case REQUEST_START_OBJECT:
		{
			OnRequestStartObject(SourceID,EventID,RequestInfo);
		}
		break;
	case REQUEST_USE_OBJECT:
		{
			OnRequestUseObject(SourceID,EventID,RequestInfo);
		}
		break;
	case REQUEST_CLOSE_OBJECT:
		{
			OnRequestCloseObject(SourceID,EventID,RequestInfo);
		}
		break;
	case REQUEST_GET_OBJECT_DOC:
		{
			OnRequestGetObjectDoc(SourceID,EventID,RequestInfo);
		}
		break;
	default:
		{
			assert(0); //Currently there are no other
		}
	}

}

    
void CSpacePortal::PrintMsg(int64 MsgID,bool Send){
	tstring s = MsgID2Str(MsgID);
	if (Send)
	{
		s = Format1024(_T("Send : %s\n"),s.c_str());
		OutputLog(LOG_TIP,s.c_str());
	} 
	else
	{
		s = Format1024(_T("Get  : %s"),s.c_str());
		OutputLog(LOG_TIP,s.c_str());
	}		
}

void CSpacePortal::CentralNerveMsgProc(CMsg& Msg){
	int64 MsgID = Msg.GetMsgID();

	PrintMsg(MsgID,false);

	switch(MsgID){
	case MSG_OBJECT_FEEDBACK:
		OnObjectFeedback(Msg);
		break;
	case MSG_ROBOT_GOTO_SPACE: 
		OnGotoSpace(Msg);
		break;
	case MSG_ROBOT_CREATE_SPACE:
		OnCreateSpace(Msg);
		break;
	case MSG_ROBOT_DEL_SPACE:
		OnDeleteSpace(Msg);
		break;
	case MSG_ROBOT_EXPORT_OBJECT:
		OnExportObject(Msg);
		break;
	case MSG_ROBOT_GET_OBJECT:
		OnAskforSpaceBody(Msg);
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

void    CSpacePortal::NerveMsgProc(CMsg& Msg){
	int64 MsgID = Msg.GetMsgID();

	switch(MsgID){

	case MSG_TASK_REQUEST:
		OnTaskRequest(Msg);
		break;
	default:
		{
			tstring MsgStr = MsgID2Str(MsgID);
			tstring s = Format1024(_T("Unkown Msg: %I64ld(%s)"),MsgID,MsgStr.c_str());
			OutputLog(LOG_WARNING,s.c_str());
		}
	}	
}
