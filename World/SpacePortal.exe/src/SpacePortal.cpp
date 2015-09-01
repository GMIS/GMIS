// SpacePortal.cpp: implementation of the CSpacePortal class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "SpacePortal.h"
#include <direct.h>
#include "SpaceMsgList.h"
#include "Win32Tool.h"
#include "resource.h"
#include "Space.h"
#include "sqlite\CppSQLite3.h"
#include "Poco/Checksum.h"
#include "DLL_Load.h"
#include "zip_utils\unzip.h"
#include <process.h>

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

struct SPACE_ACCOUNT{
			tstring Name;
			tstring Password;
			tstring Confirm;
			tstring CrypText;
			tstring LocalName;
			uint32  OuterIP;
};

SPACE_ACCOUNT     AfxAccount;

BOOL CALLBACK SetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK GetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);



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

CSpacePortal::CSpacePortal(CSystemInitData* InitData)
:System(InitData)
{
	m_InitThread=0;
	m_SecondHide = -1;
	m_Second = m_SecondHide;

	m_LogFlag = LOG_MSG_I0_RECEIVED|LOG_MSG_IO_PUSH|LOG_MSG_IO_SENDED|LOG_MSG_IO_REMOTE_RECEIVED;

	m_Created = FALSE;
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

UINT CSpacePortal::InitThreadFunc (LPVOID pParam){

	CSpacePortal* World= (CSpacePortal*)pParam;
	try{
		World->CheckWorldDB();
	}catch (CppSQLite3Exception e) {
		
#ifdef _UNICODE
		AnsiString s = e.errorMessage();
		tstring ws = UTF8toWS(s);
		World->AddRTInfo(ws);
#else
        World->AddRTInfo(e.errorMessage());
#endif
		World->AddRTInfo(_T("WARNING:  Space database check fail."));
	}catch(...){
        World->AddRTInfo(_T("WARNING:  unkown exception occur ,Space database check fail."));
	};
	World->AddRTInfo(_T("SpacePortal check Ok !"));
	if(World->m_SecondHide != -1){
		tstring s = Format1024(_T("Hide window after %d second..."),World->m_SecondHide);	
		World->AddRTInfo(s);	
		::SetTimer(World->GetHwnd(),TIMER_HIDEWIN,1000,NULL);
	};
	return 0;
}

void    CSpacePortal::CheckWorldDB(){

	return ; //Seems to be unnecessary

	CppSQLite3Buffer  SQL;
    char Name[30];
    int64 ID = 2;
	vector<int64> TableList;
	bool FindEnd = false;
		
	m_Status.SetTip(_T("Checking Space Database...")); 
			
	SQL.format("select  count(*) from sqlite_master where type='table'");
	CppSQLite3Query q = GetWorldDB().execQuery(SQL);	        
	if (q.eof())return;
	int32 Count = q.getIntField(0);
	if(Count==0)return;

    int32 n=0; 

	SPACETYPE RoomType = ROBOT_VISITER;
	while(m_Alive) 
	{
		//Each time we take out the 100-space checking, delete the ROBOT_VISIT residues 
        TableList.clear();    
	    int64toa(ID,Name); 	
		SQL.format("select name from sqlite_master where type='table' and name > \"%s\"",Name);
		q = GetWorldDB().execQuery(SQL);	        
		if (q.eof())break;
		
		while (m_Alive && !q.eof())
        {
			ID = q.getInt64Field(0);
			TableList.push_back(ID);
			if (TableList.size()== 100)break;
            q.nextRow();
        }
        
		vector<int64>::iterator It = TableList.begin();
		while (m_Alive && It != TableList.end())
		{
            int64toa(*It,Name); 	
			SQL.format("delete from \"%s\"  where %s = %d ",Name,ITEM_TYPE,RoomType);
			GetWorldDB().execDML(SQL);
			It++;
			n++;
			m_Status.SetProgressPer(n*100/Count);
		}
          
	};
	m_Status.SetProgressPer(0);
    m_Status.SetTip(_T(""));
}

bool CSpacePortal::Activation(){
    

	HINSTANCE hInstance = CWinSpace2::GetHinstance();
		

	if(!HasChild(ROOT_SPACE)){
		
		if(::MessageBox(NULL,_T("No Found World Database,Create new ?"),_T("Query"),MB_YESNO)==IDNO)return false;
		

		int ret = DialogBoxParam (hInstance,MAKEINTRESOURCE(IDD_SETACCOUNT), GetHwnd(), SetPasswordDlgProc, 0);
		if(ret>0){
			try{
	
				tstring Cryptograph = AfxAccount.Name+AfxAccount.Password;
								

				SpaceAddress Address(ROOT_SPACE,OUTER_SPACEID);
				tstring OuterName = _T("Unkown Name");  //Should be according to the Outer IP to get the name
                         				
				tstring Fingerprint = Format1024(_T("%I64ld"),OUTER_SPACEID); //Current space eigenvalue replaced by its subspace's ID for simplifying 

				ROOM_SPACE OuterRoom(ROOT_SPACE,OUTER_SPACEID,OuterName,0,OUTER_SPACE,NO_RIGHT,Fingerprint);

				Fingerprint = Format1024(_T("%I64ld"),LOCAL_SPACEID);
				ROOM_SPACE LocalRoom(ROOT_SPACE,LOCAL_SPACEID,AfxAccount.LocalName, 0,LOCAL_SPACE,NO_RIGHT,Fingerprint);

				LocalRoom.AddOwner(AfxAccount.Name.c_str(),Cryptograph,USABLE);

				People& Host = GetHost();
				Host.SetName(AfxAccount.Name);
				Host.m_Cryptograhp = Cryptograph;
				Host.SetSpaceType(ROBOT_HOST);
				
				ROOM_SPACE& RootRoom = GetRootRoom();
				bool ret = RootRoom.Logon(0,Host);
				assert(ret);
			}    
			catch (...) {
				return false;		
            }			
		}
		else return false;		
	}

	else 
	{
		bool bHostValid = false;
		if(m_CmdLine.Size()==2)
		{
		
			AfxAccount.Name = m_CmdLine.PopString();
			tstring CryptText = m_CmdLine.PopString();

			People& Host = GetHost();
			Host.SetName(AfxAccount.Name);
			Host.m_Cryptograhp = CryptText;
			Host.SetSpaceType(ROBOT_HOST);

			ROOM_SPACE& RootRoom = GetRootRoom();
			if(RootRoom.Logon(0,Host)){			                
				bHostValid = true;				
			}
		}

		int i=0;
		while(!bHostValid && i++<3){
		    SPACE_ACCOUNT param;
			int ret = ::DialogBoxParam (hInstance,MAKEINTRESOURCE(IDD_PASSWORD), 
                          GetHwnd(), GetPasswordDlgProc, (long)&param);
			if(ret==0)return false;
		   try{

                People& Host = GetHost();
				Host.SetName(AfxAccount.Name);
				Host.m_Cryptograhp = AfxAccount.Name+AfxAccount.Password; 
				Host.SetSpaceType(ROBOT_HOST);

				ROOM_SPACE& RootRoom = GetRootRoom();
				if(RootRoom.Logon(0,Host)){	
					bHostValid = true;
						break;				
				}
			}catch(exception& roException){
#ifdef _UNICODE
			   AnsiString str = roException.what();
			   tstring s = UTF8toWS(str);
				AddRTInfo(s);
#else
			    AddRTInfo(roException.what());
#endif
	        }
			catch (...) {
				AddRTInfo(_T("Unkown error"));		
            }
		}	
		if (!bHostValid)
		{
			return false;
		}
	}


	if(!System::Activation())return false;

	tstring error;
	if(!OpenPort(SPACE_PORT,error,false)){
		AddRTInfo(error);
		m_Alive = false;
		return false;	
	};

	//CheckWorldDB();
	uint32 ThreadID =0;
	m_InitThread = (HANDLE)_beginthreadex(NULL,   // Security
			0,							               // Stack size - use default
			InitThreadFunc,     		
			(void*)this,	      
			0,					                	   // Init flag
			&ThreadID);		
		
	if(m_InitThread == 0){
		AddRTInfo(_T("Error: can't create thread, space check fail"));  
		return false;
	}
	return true;	
};

void CSpacePortal::Dead(){

	int64 SourceID = 0;
	CLockedLinkerList* LinkerList = GetClientLinkerList();
	CLinker Linker;
	LinkerList->GetNextLinker(SourceID,Linker);
	
	while (Linker.IsValid())
	{
		People* Robot = (People*)Linker().m_Owner;
		if (Robot)
		{
			Robot->GoOut();
		}
		LinkerList->GetNextLinker(Linker().GetSourceID(),Linker);
	}

	System::Dead();

	if (m_InitThread==0)
	{
		return;
	}
	::WaitForSingleObject(m_InitThread, INFINITE);
	::CloseHandle(m_InitThread);
	
}

void  CSpacePortal::PushExecuterEvent(int64 ExecuterID,int64 RobotID,int64 EventID){
	assert(ExecuterID);
	assert(RobotID);
	assert(EventID);

	CLock lk(&m_Mutex);
	map<int64,int64>& EventList = m_ExecuterEventList[ExecuterID];
	EventList[EventID] = RobotID;
};
int64 CSpacePortal::PopExecuterEvent(int64 ExecuterID,int64 EventID){
	assert(ExecuterID);
	assert(EventID);
	
	CLock lk(&m_Mutex);
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

	CLock lk(&m_Mutex);
	set < int64 >& ExecuterList = m_ExecuterUserList[RobotID];
	ExecuterList.insert(ExecuterID);
}





bool  CSpacePortal::StartExecuter(int64 ExecuterID,tstring FileName){
    
	while(m_ExecuterPool.size()){
		int64 SourceID = m_ExecuterPool.back();
		m_ExecuterPool.pop_back();
		CLinker Linker;
		GetLinker(SourceID,Linker);
		if (Linker.IsValid())
		{
			GetClientLinkerList()->SetLinkerID(SourceID,ExecuterID);
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
	
	AddRTInfo(ExecuterFile);

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

	AddRTInfo(_T("Waite Executer start..."));

	//Then wait for the connection to success
	int n=50;
	
	CLinker ExecuterLinker;
	GetLinker(ExecuterID,ExecuterLinker);
	while((!ExecuterLinker.IsValid() || ExecuterLinker().GetRecoType() != LINKER_FRIEND) && n-- >0 ){
		GetLinker(ExecuterID,ExecuterLinker);
		SLEEP_MILLI(200);
	}
	
	if (!ExecuterLinker.IsValid())
	{
		PopExecuterEvent(ExecuterID,102);
		return false;
 	}
	return true;
}

void   CSpacePortal::UserLinkerClosedProc(int64 RobotID)
{
	CLock lk(&m_Mutex);
	
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
	CLock lk(&m_Mutex);

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


void CSpacePortal::AddRTInfo(tstring& s){
	InfoItem* Item = m_Log.AddInfo(s.c_str());
	
}

void CSpacePortal::AddRTInfo(const TCHAR* Text){
	InfoItem* Item = m_Log.AddInfo(Text);
};


void CSpacePortal::CloseLinker(int64 ID){
	GetClientLinkerList()->DeleteLinker(ID);
};



void  CSpacePortal::OutputLog(uint32 Type,const TCHAR* text){
	if(m_LogFlag & Type){
		AddRTInfo(text);
	}

};

void  CSpacePortal::NotifyLinkerState(CLinkerPipe* Linker,int64 NotifyID,ePipeline& Data){

};

System::CNerveWork* CSpacePortal::CreateNerveWorker(int64 ID,System* Parent,uint32 Reason){
	return NULL;
}
Model::CCentralNerveWork* CSpacePortal::CreateCentralNerveWorker(int64 ID,Model* Parent,uint32 Reason){
/*
	if (Reason == REASON_MSG_TOO_MUCH)
	{
		int32 NerveMsgNum = GetNerveMsgNum();
		int32 CentralMsgNum = GetCentralNerveMsgNum();
		float f = (float32)(CentralMsgNum-NerveMsgNum)/(float32)CentralMsgNum*100;
		if (f<10.0f)
		{
			return NULL;  //不批准
		}
	}
*/
	System::CCentralNerveWork* NerveWork = new System::CCentralNerveWork(ID,this);
	return NerveWork;
}



void  CSpacePortal::OnObjectFeedback(CMsg& Msg)
{
	int64 ExecuterID  = Msg.GetSourceID();
    int64 EventID = Msg.GetEventID();
    ePipeline& Letter = Msg.GetLetter();
	
	if (EventID==0) //Feedback from object
	{
		return;
	}

	int64 RobotID = PopExecuterEvent(ExecuterID,EventID);
    if (RobotID==0)
    {
		return;
    }

	WriteLogDB(_T("Executer Feedback Event:%I64ld"),EventID);

	CLinker RobotLinker;
	GetLinker(RobotID,RobotLinker);
	if (RobotLinker.IsValid())
	{
		CMsg FeedbackMsg(MSG_TASK_FEEDBACK,NULL,EventID);
		ePipeline& rLetter = FeedbackMsg.GetLetter();
		rLetter << Letter;  
		RobotLinker().PushMsgToSend(FeedbackMsg);
	}

}
void CSpacePortal::OnLinkerError(CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter();
	int64 SourceID = Letter.PopInt();
    		
    CLinker Linker;
	GetLinker(SourceID,Linker);
	
	if (!Linker.IsValid())
	{
		return;
	}
	
	if (Linker().GetRecoType()==LINKER_DEL)
	{
		return;
	}
	
	if (Linker().IsClientLinker())
	{
		GetClientLinkerList()->DeleteLinker(SourceID);
	}else{
		GetSuperiorLinkerList()->DeleteLinker(SourceID);
	}

	if (Linker().m_Owner == NULL)
	{   
		ExecuteLinkerClosedProc(SourceID);
	}else{ 
		People* Robot = (People*)Linker().m_Owner;
		if(Robot != NULL){
			delete Robot;  
			Linker().m_Owner = NULL;
		};
           
		UserLinkerClosedProc(SourceID);
	}
}


void CSpacePortal::OnI_AM(CMsg& Msg){

	int64 SourceID = Msg.GetSourceID();
    
	CLinker Who;
	GetLinker(SourceID,Who);
    if (!Who.IsValid())
    {
		return;
    }

	int64 EventID = Msg.GetEventID();
	ePipeline& Letter = Msg.GetLetter();

	Letter.AutoTypeAB();	
	if(Letter.GetTypeAB() != 0x33100000 ){ //one ID+ two string
		assert(0);
		CloseLinker(SourceID);
		return ;
	};

	tstring  Name        = *(tstring*)Letter.GetData(0);  
	tstring  Cryptograhp = *(tstring*)Letter.GetData(1);
		
    if (EventID == 102)//From executer
	{
		int64 ExecuterID = _ttoi64(Cryptograhp.c_str());

		if (ExecuterID==0)
		{  
			if (m_ExecuterPool.size()>19)
			{
				CloseLinker(SourceID);
			}
			m_ExecuterPool.push_back(SourceID);

			Who().SetRecoType(LINKER_FRIEND);
			CMsg rMsg(MSG_CONNECT_OK,NULL,0);
			rMsg.GetLetter().PushString(GetName());
			Who().PushMsgToSend(rMsg);
			return; 
		}else{
			int64 RobotID    = PopExecuterEvent(ExecuterID,EventID);

			if (RobotID != 1)
			{
				//Not invited to connect， illegally
				CloseLinker(SourceID);
			}
		}

		Who().SetRecoType(LINKER_FRIEND);
		GetClientLinkerList()->SetLinkerID(SourceID,ExecuterID);
		
		CMsg rMsg(MSG_CONNECT_OK,NULL,0);
		rMsg.GetLetter().PushString(GetName());
		Who().PushMsgToSend(rMsg);
		return;
	}

	bool Delete = false;
	
	//Try to login
	//NOTE: In fact it may be a outer space to  require a connection, but here to ignore 
    bool ret = false;
	
	People*  p = new People(Name,Cryptograhp);

	try
    {	
		ROOM_SPACE& RootRoom = GetRootRoom();
		ret = RootRoom.Logon(SourceID,*p);   	
    }catch (...) {

    }

 	if(!ret){
		delete p;
		CloseLinker(SourceID);
		return;
	}  

    Who().m_Owner = p;
    
	CMsg rMsg(MSG_CONNECT_OK,NULL,0);
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
	
	People* Body = (People*)Who().m_Owner;
	assert(Body != NULL);


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
	
	if(Room.AllowInto(*Body)){
		Body->GoInto(SourceID,Room);
	}else{
		CMsg rMsg(MSG_SPACE_ACCESS_DENIED,NULL,NULL);
		ePipeline& rLetter = rMsg.GetLetter();
		rLetter.PushInt(SpaceID);
		rLetter.PushString(_T("right invalid"));
		
		Who().PushMsgToSend(rMsg);	
		return;
	}
	

	ePipeline Pipe;
	GetAllChildList(Body->GetParentID(),Pipe,Body->GetSpaceID());
		
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
	GetAllVisiter(Body->GetParentID(),SiblingList,Body->GetSpaceID());
	for(uint32 i=0; i<SiblingList.size(); i++){
		int64 WhoID = SiblingList[i];

		CLinker Linker;
		GetLinker(WhoID,Linker);
		if (!Linker.IsValid())
		{
			continue;
		}

		People* Owner = (People*)Who().m_Owner;
		assert(Owner != NULL);
        assert(Owner->IsValid());
		
		ePipeline* ChildPipe  = new ePipeline(Owner->GetParentID());

		tstring Name = Body->GetName();
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

	int64 ParentID = ((CSpace*)Who().m_Owner)->GetParentID();

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
		People* Owner = (People*)Linker().m_Owner;
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
		People* Owner = (People*)Linker().m_Owner;
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
	People* p = (People*)Who().m_Owner;
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

	
	ePipeline& Letter = Msg.GetLetter();
    ePipeline& RequestInfo = *(ePipeline*)Letter.GetData(0);
    int64 RequestID = RequestInfo.GetID();
	switch(RequestID){
	case REQUEST_START_OBJECT:
		{
			OnRequestStartObject(SourceID,RequestInfo);
		}
		break;
	case REQUEST_USE_OBJECT:
		{
			OnRequestUseObject(SourceID,RequestInfo);
		}
		break;
	case REQUEST_CLOSE_OBJECT:
		{
			OnRequestCloseObject(SourceID,RequestInfo);
		}
		break;
	case REQUEST_GET_OBJECT_DOC:
		{
			OnRequestGetObjectDoc(SourceID,RequestInfo);
		}
		break;
	default:
		{
			assert(0); //Currently there are no other
		}
	}

}

void CSpacePortal::Layout(bool Redraw /* = true */){
	CWSFrameView::Layout(Redraw);
	if(m_Created){
		RECT rc = m_rcClient;
		rc.bottom -= 20;
		if(rc.bottom<rc.top)rc.bottom=rc.top;
		::MoveWindow(m_Log.GetHwnd(),rc.left,rc.top,RectWidth(rc),RectHeight(rc),TRUE);
 		rc = rc;
		rc.top   = rc.bottom-1;
		rc.bottom=m_rcClient.bottom;
		::MoveWindow(m_Status.GetHwnd(),rc.left,rc.top,RectWidth(rc),RectHeight(rc),TRUE);    
	}
}

LRESULT CSpacePortal::Reaction(UINT message, WPARAM wParam, LPARAM lParam){
	switch(message){
	case WM_TIMER:
		return OnTimer(wParam,lParam);
    case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_CREATE:
		return OnCreate(wParam,lParam);
	default:
		return CWSFrameView::Reaction(message,wParam,lParam);
	}
}
LRESULT CSpacePortal::OnCreate( WPARAM wParam, LPARAM lParam) 
{
	DWORD style = GetWindowLong(m_hWnd,GWL_STYLE);
	style |=WS_CLIPCHILDREN;
	SetWindowLong(m_hWnd,GWL_STYLE,style);
	
	RECT rc;
	::SetRect(&rc,0,0,0,0);
	if(!m_Log.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE,rc,GetHwnd())){
		return -1;
	}
	
	if(!m_Status.Create(GetHinstance(),NULL,WS_CHILD|WS_VISIBLE,rc,GetHwnd())){
		return -1;
	}
	
    m_Status.SetProgressAlpha(127);
	
	AddRTInfo(_T("Enter SpacePortal ... "));
	m_Created = TRUE;
    return 0;
}

LRESULT CSpacePortal::OnTimer(WPARAM wParam, LPARAM lParam) 
{
	long nIDEvent = wParam;
    if(nIDEvent == TIMER_HIDEWIN){
		m_Second--;
	    tstring s = Format1024(_T("%d second..."),m_Second);
		m_Status.SetTip(s.c_str());
		if(m_Second==0){
			m_Second = m_SecondHide;
			m_Status.SetTip(_T(""));
			::KillTimer(GetHwnd(),TIMER_HIDEWIN);
			::ShowWindow(GetHwnd(),SW_HIDE);
		}
	}
	else return CWSFrameView::OnTimer(wParam,lParam);
	return 0;
}

BOOL CALLBACK SetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message)
	{
		case WM_INITDIALOG:
			{
			 CenterWindow(hDlg,GetParent(hDlg));
             SPACE_ACCOUNT* ac = &AfxAccount; 
			::SetFocus(GetDlgItem(hDlg,IDC_SETNAME));
			 return TRUE;
            }
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK ) 
			{
				SPACE_ACCOUNT* ac = &AfxAccount; 
				HWND hName = GetDlgItem(hDlg,IDC_SETNAME);
				HWND hPassword = GetDlgItem(hDlg,IDC_SETPASSWORD);
				HWND hConfirm  = GetDlgItem(hDlg,IDC_SETCONFIRM);
				HWND hLocalName = GetDlgItem(hDlg,IDC_LOCALNAME);
				
				//目前不可用
				HWND hOuterIP  = GetDlgItem(hDlg,IDC_OUTER_IP);
				::EnableWindow(hOuterIP,FALSE);

				TCHAR buf[100];
				::GetWindowText(hName,buf,99);
			    ac->Name = buf;
				::GetWindowText(hPassword,buf,99);
			    ac->Password = buf;
				::GetWindowText(hConfirm,buf,99);
			    ac->Confirm = buf;
				::GetWindowText(hLocalName,buf,99);
				ac->LocalName  = buf;
				if(ac->Name.size()>0 && ac->Confirm.size()>0 && ac->Confirm == ac->Password){
					::SetWindowText(hName,_T(""));
					::SetWindowText(hPassword,_T(""));		
					::SetWindowText(hConfirm,_T(""));
					::SetWindowText(hLocalName,_T(""));
					if (ac->LocalName.size() == 0)
					{ 
						ac->LocalName == _T("");
					}
					::EndDialog(hDlg, 1);
				}
				return TRUE;
			}else if(LOWORD(wParam) == IDCANCEL){
                ::EndDialog(hDlg, 0);
				return TRUE;
			}
			break;
	}
    return FALSE;
};

BOOL CALLBACK GetPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	
	switch (message)
	{
		case WM_INITDIALOG:
			{
             CenterWindow(hDlg,GetParent(hDlg));
             SPACE_ACCOUNT* ac = (SPACE_ACCOUNT*)lParam;
			 AfxAccount = *ac;
			 ac = &AfxAccount;
			::SetFocus(GetDlgItem(hDlg,IDC_SETNAME));
			return TRUE;
			}
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK ) 
			{
				SPACE_ACCOUNT* ac = &AfxAccount; 
				HWND hName = GetDlgItem(hDlg,IDC_SETNAME);
				HWND hPassword = GetDlgItem(hDlg,IDC_EDITPASSWORD);
				TCHAR buf[100];
				::GetWindowText(hName,buf,99);
			    ac->Name = buf;
				::GetWindowText(hPassword,buf,99);
			    ac->Password = buf;
				if(ac->Name.size()>0 && ac->Password.size()>0 ){
					::SetWindowText(hName,_T(""));
					::SetWindowText(hPassword,_T(""));
					::EndDialog(hDlg, 1);
				}
				return TRUE;
			}else if(LOWORD(wParam) == IDCANCEL){
                ::EndDialog(hDlg, 0);
				return TRUE;
			}
	}
    return FALSE;
};
    
void CSpacePortal::PrintMsg(int64 MsgID,bool Send){
	tstring s = MsgID2Str(MsgID);
	if (Send)
	{
		s = Format1024(_T("Send : %s"),s.c_str());
		AddRTInfo(s);
	} 
	else
	{
		s = Format1024(_T("Get  : %s"),s.c_str());
		AddRTInfo(s);
	}		
}

bool CSpacePortal::Do(Energy* E){


	   if (E==NULL)
	   {
		
		  return true;
	   }

	   ePipeline* Pipe = (ePipeline*)E;
	   CMsg Msg(Pipe);

		int64 MsgID = Msg.GetMsgID();

		PrintMsg(MsgID);
		
		switch(MsgID){

		case MSG_TASK_REQUEST:
			OnTaskRequest(Msg);
			break;
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
		case MSG_LINKER_ERROR:
			OnLinkerError(Msg);
			break;
		case MSG_WHO_ARE_YOU:
			return true;
		case MSG_I_AM: 
			OnI_AM(Msg);
			return true;
		default:
			{
				tstring MsgStr = MsgID2Str(MsgID);
				tstring s = Format1024(_T("Unkown Msg: %I64ld(%s)"),MsgID,MsgStr.c_str());
				AddRTInfo(s);
			}
		}	
	
	
	return true;
}