#include "Executer.h"
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
	m_FileAddress = DllFileName;

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
	m_Object = NULL;
}

CExObject::CExObject()
:m_Object(NULL),m_DLLHandle(NULL),m_bRuning(false){
  
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


void CExObject::Start(CExecuter* Executer,int64 EventID,ePipeline* ExePipe){
	AutoBool bRuning(m_bRuning); 
    assert(m_Object);
	
	//初始化
	if (m_Object->MassType()>=MASS_OBJECT)
	{
		PHYSIC::Object* Ob = (PHYSIC::Object*)m_Object;
		assert(!Ob->IsAlive());
		if(!Ob->Activation()){

			ExePipe->SetID(RETURN_ERROR);
			ExePipe->GetLabel() = Format1024(_T("Object activation fail:%s"),m_FileAddress.c_str());
			Executer->OutputLog(LOG_TIP,ExePipe->GetLabel().c_str());

			Executer->SendFeedback(EventID,*ExePipe);
			return ;
		}
	}
	int64 ObjectInstanceID = EventID;
	ExePipe->PushInt(Executer->m_ID); //=DllType
	ExePipe->PushInt(ObjectInstanceID);  
	Executer->SendFeedback(EventID,*ExePipe);	
}

void  CExObject::Run(CExecuter* Executer,int64 EventID,ePipeline* ExePipe){
 
	AutoBool bRuning(m_bRuning);

	tstring Error;
	try{

		if (m_Object==NULL)
		{
			ExePipe->SetID(RETURN_ERROR);
			ExePipe->GetLabel() = Format1024(_T("Object invalid:%s"),m_FileAddress.c_str());
			Executer->SendFeedback(EventID,*ExePipe);	
			return;
		}

		uint32 Type = m_Object->GetTypeAB();             		
		if( !ExePipe->HasTypeAB(Type))  
		{
			Error = Format1024(_T("Object(%s) input data type check fail. Data type:%u Require type:%u"),GetName().c_str(),ExePipe->GetTypeAB(),Type); 
			Executer->OutputLog(LOG_TIP,Error.c_str());
			ExePipe->SetLabel(Error.c_str()); 
			ExePipe->SetID((RETURN_ERROR));
			Executer->SendFeedback(EventID,*ExePipe);
			return;

		}else{

			if(!m_Object->Do(ExePipe)){
				ExePipe->SetID((RETURN_ERROR));
				Executer->SendFeedback(EventID,*ExePipe);
				return;
			}else{
				tstring s = Format1024(_T("Run Object(%s) ok \n"),m_FileAddress.c_str());
				Executer->OutputLog(LOG_TIP,s.c_str());		
				Executer->SendFeedback(EventID,*ExePipe);
				return;
			}   			
		}

	}catch(...)
	{
		Error = Format1024(_T("Run Object(%s) fail:Exception error"),m_FileAddress.c_str());
	}

	Executer->OutputLog(LOG_TIP,Error.c_str());

	ExePipe->SetID(RETURN_ERROR);
	ExePipe->SetLabel(Error.c_str());	
	Executer->SendFeedback(EventID,*ExePipe);

	return;
}

void CExObject::GetDoc(CExecuter* Executer,int64 EventID,ePipeline* ExePipe){

	try{
		tstring Doc;
		bool ret = GetObjectDoc(Doc);
		if (!ret)
		{
			ExePipe->SetID(RETURN_ERROR);
			ExePipe->GetLabel() = Format1024(_T("Object invalid:%s"),m_FileAddress.c_str());
			Executer->SendFeedback(EventID,*ExePipe);	
			return;
		}
		ExePipe->PushString(Doc);

		//tstring s = Format1024(_T("Get Object Doc(%s) ok \n"),m_FileAddress.c_str());
		//Executer->OutputLog(LOG_TIP,s.c_str());		
				
		Executer->SendFeedback(EventID,*ExePipe);
		return;

	}catch(...)
	{

	}

	tstring Error = Format1024(_T("Run Object(%s) fail:Exception error"),m_FileAddress.c_str());

	Executer->OutputLog(LOG_TIP,Error.c_str());

	ExePipe->SetID(RETURN_ERROR);
	ExePipe->SetLabel(Error.c_str());	
	Executer->SendFeedback(EventID,*ExePipe);

	return;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExecuter::CExecuter(int64 ID,tstring CryptText,CUserTimer* Timer,CUserSpacePool* Pool)
:Model(Timer,Pool)
{
	
	m_LogFlag =  LOG_TIP|LOG_WARNING|LOG_MSG_PROC_BEGIN;//|LOG_MSG_IO_PUSH|LOG_MSG_I0_RECEIVED|LOG_MSG_IO_SENDED;
	m_ID = ID;
	m_Name = _T("Executer");

}

CExecuter::~CExecuter()
{
	DeleteAllObject();
};

CExObject& CExecuter::CreateNewObject(int64 EventID){
	_CLOCK(&m_ObjectListMutex);
	assert(m_ObjectList.find(EventID) == m_ObjectList.end());
	CExObject& Object = m_ObjectList[EventID];
	return Object;
};

CExObject& CExecuter::FindObject(int64 EventID){
	_CLOCK(&m_ObjectListMutex);
	static CExObject EmptyObject;
	map<int64,CExObject>::iterator it = m_ObjectList.find(EventID);
	if(it== m_ObjectList.end()){
		return EmptyObject;
	}
	CExObject& Object = it->second;
	return Object;
};

void CExecuter::DeleteObject(int64 EventID){
	_CLOCK(&m_ObjectListMutex);
	map<int64,CExObject>::iterator it = m_ObjectList.find(EventID);
	if(it!= m_ObjectList.end()){
		m_ObjectList.erase(it);
	}
};

void CExecuter::DeleteAllObject(){
	_CLOCK(&m_ObjectListMutex);
	map<int64,CExObject>::iterator it = m_ObjectList.begin();
	
	try
	{
		while(it!= m_ObjectList.end()){
			CExObject& Object = it->second;
			bool ret = Object.WaitForStop(100);
			if(ret){
				it = m_ObjectList.erase(it);
			}else{
				it++;
			}
		}
		m_ObjectList.clear();
	}
	catch (...)
	{
		
	}	
};

tstring CExecuter::MsgID2Str(int64 MsgID){
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

void  CExecuter::OutputLog(uint32 Type,const TCHAR* text){
	int64 t=CreateTimeStamp();
	tstring s = GetTimer()->GetHMSM(t);

	if(m_LogFlag & Type){
		std::wcout<<s.c_str()<<_T("   ")<<text<<endl;
	}
};
void CExecuter::OutputLog(uint32 Flag,TCHAR* Format, ...){
	if (m_LogFlag & Flag)
	{
		int64 t=CreateTimeStamp();
		tstring s = GetTimer()->GetHMSM(t);

		TCHAR Buffer [512] ;
		va_list ArgList ;		
		va_start (ArgList, Format) ;
		_vsntprintf(Buffer, 256, Format, ArgList) ;		
		va_end (ArgList) ;

		std::wcout<<s.c_str()<<_T("   ")<<Buffer<<endl;
	}

};

void CExecuter::NotifyLinkerState(int64 SourceID,int64 NotifyID,STATE_OUTPUT_LEVEL Flag,ePipeline& Info){

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
			} //注意：这里不要break
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
					OutputLog(LOG_MSG_IO_PUSH,_T("LINKER_PUSH_MSG: %s EventID:%I64ld PendingMsg=%s CurMsgNum=%I64ld"),MsgName.c_str(),EventID,PendingMsgName.c_str(),MsgNum+UrgMsgNum);
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

					ePipeline* Msg = (ePipeline*)Info.GetData(0);
					if (Msg->GetID()<100)
					{
						//WriteLogDB(_T("Msg Sended: internal feedback sended"));
					}else{
						CMsg SendedMsg;
						Info.PopMsg(SendedMsg);
						int64 MsgID = SendedMsg.GetMsgID();
						tstring MsgName = MsgID2Str(MsgID);
						OutputLog(LOG_MSG_IO_SENDED,_T("LINKER_MSG_SENDED: %s  EventID:%I64ld"),MsgName.c_str(),SendedMsg.GetEventID());
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

					tstring s = Format1024(_T("LINKER_BEGIN_ERROR: SourceID=%I64ld ErrorType=%I64ld CurrentRevMsg:%s"),SourceID,ErrorType,UTF8toWS(text).c_str());

					OutputLog(LOG_ERROR,s.c_str());
				}
				return;
			case LINKER_END_ERROR_STATE:
				{
					tstring s = Format1024(_T("LINKER_END_ERROR: SourceID=%I64ld"),SourceID);
					OutputLog(LOG_ERROR,s.c_str());
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
				}
				return;
			case LINKER_ILLEGAL_MSG:
				{
					ePipeline* CurRevMsg = (ePipeline*)Info.GetData(0);

					AnsiString text;
					CurRevMsg->ToString(text);

					tstring s = Format1024(_T("LINKER_ILLEGAL_MSG: SourceID=%I64ld Msg:%s "),SourceID,UTF8toWS(text).c_str());

					OutputLog(LOG_WARNING,s.c_str());

				}
				return;
			case LINKER_EXCEPTION_ERROR:
				{
					int32 RecoType = Info.PopInt();
					int32 LinkerType = Info.PopInt();

					tstring Error = Info.PopString();
					tstring s = Format1024(_T("LINKER_EXCEPTION_ERROR: SourceID=%I64ld %s will be closed "),SourceID,Error.c_str());

					OutputLog(LOG_WARNING,s.c_str());

					GetSuperiorLinkerList()->DeleteLinker(SourceID);
					m_Alive=false;
				}
				return;
			case LINKER_IO_ERROR:
				{
					//通常是远端关闭

					int32 RecoType = Info.PopInt();
					int32 LinkerType = Info.PopInt();

					tstring s = Format1024(_T("LINKER_IO_ERROR: SourceID=%I64ld may be closed by remote"),SourceID);

					OutputLog(LOG_WARNING,s.c_str());

					assert (LinkerType==SERVER_LINKER);	

					GetSuperiorLinkerList()->DeleteLinker(SourceID);
					m_Alive=false;

				}
				return;
			case LINKER_CONNECT_ERROR:
				{
					tstring Error = Info.PopString();
					tstring s = Format1024(_T("LINKER_CONNECT_ERROR: SourceID=%I64ld %s "),SourceID,Error.c_str());

					OutputLog(LOG_WARNING,s.c_str());

					GetSuperiorLinkerList()->DeleteLinker(SourceID);
					m_Alive=false;
				}
				return;
			case LINKER_CREATE_ERROR:
				{
					tstring Error = Info.PopString();
					tstring s = Format1024(_T("LINKER_CREATE_ERROR: %s "),Error.c_str());

					OutputLog(LOG_WARNING,s.c_str());	
				}

			}
		}
	}
};

void CExecuter::GetSuperior(int64 ID,CLinker& Linker){
	CSuperiorLinkerList* List = GetSuperiorLinkerList();
	List->GetLinker(ID,Linker);
}


void  CExecuter::SendFeedback(int64 EventID,ePipeline& Param){
	CMsg rMsg(MSG_OBJECT_FEEDBACK,DEFAULT_DIALOG,EventID);
	ePipeline& rLetter = rMsg.GetLetter();
	rLetter.PushPipe(Param);

	CLinker Linker;
	GetSuperior(SYSTEM_SOURCE,Linker);

	if (Linker.IsValid())
	{
		int64 MsgID = rMsg.GetMsgID();
		tstring MsgStr = MsgID2Str(MsgID);
		tstring s = Format1024(_T("Send Msg: %s\n"),MsgStr.c_str());
		OutputLog(LOG_TIP,s.c_str());
		Linker().PushMsgToSend(rMsg);
	} 
	else
	{
		int64 MsgID = rMsg.GetMsgID();
		tstring MsgStr = MsgID2Str(MsgID);

		tstring s = Format1024(_T("Msg(%s) send fail: Linker invalid\n"),MsgStr.c_str());
		OutputLog(LOG_TIP,s.c_str());
	}
}

void CExecuter::CentralNerveMsgProc(CMsg& Msg){

		int64 MsgID = Msg.GetMsgID();

		if (m_LogFlag & LOG_MSG_PROC_BEGIN)
		{
			
			int64 EventID = Msg.GetEventID();

			tstring CmdStr = MsgID2Str(MsgID);
			tstring& Tip = Msg.GetMsg().GetLabel();
			OutputLog(LOG_MSG_PROC_BEGIN,_T("Get Msg:%s EventID:%I64ld %s"),CmdStr.c_str(),EventID,Tip.c_str());		
		}

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
		case MSG_BROADCAST_MSG:
			OnBroadcasgMsg(Msg);
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

	return ;
}
void CExecuter::OnConnectTo(CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter();
	int64 ID = Letter.PopInt();
	tstring& ip = Letter.PopString();
    int32 Port = Letter.PopInt();
	int32 Second = Letter.PopInt();
		
	AnsiString ip1 = WStoUTF8(ip);
	
	OutputLog(LOG_TIP,_T("Begin connect spaceport"));

	tstring error;
    
	if(!Connect(ID,ip1,Port,Second,error,TRUE)){
		m_Alive = false;
	}
}


void CExecuter::OnAppReset(CMsg& Msg){
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

void CExecuter::OnObjectStart(CMsg& Msg)
{
	int64 EventID  = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();

    tstring ObjectFileName = Letter.PopString();	
	
	eElectron E;
	Letter.Pop(&E);
	ePipeline* ExePipe     = (ePipeline*)E.Value();
	
	int64 ObjectInstanceID = EventID;
	CExObject& Object =  CreateNewObject(ObjectInstanceID);

	//先试着Load object		
	Mass* m = Object.LoadObject(ObjectFileName);
	if(m==NULL){
		ExePipe->SetID(RETURN_ERROR);
		ExePipe->GetLabel() = Format1024(_T("Dll Load fail:%s"),ObjectFileName.c_str());
		OutputLog(LOG_TIP,ExePipe->GetLabel().c_str());
		SendFeedback(EventID,*ExePipe);	
		return ;
	};
	
	Object.Start(this,EventID,ExePipe);					
};


void CExecuter::OnObjectRun(CMsg& Msg){

	int64 EventID  = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter();

	int64 ObjectInstanceID = Letter.PopInt();
	eElectron E;
	Letter.Pop(&E);
	ePipeline* ExePipe     = (ePipeline*)E.Value();

	CExObject& Object = FindObject(ObjectInstanceID);
	if (!Object.IsValid())
	{
		tstring Error =_T("the object is invalid");
		ExePipe->SetID(RETURN_ERROR);
		ExePipe->SetLabel(Error.c_str());	
		SendFeedback(EventID,*ExePipe);
		return;
	}
	Object.Run(this,EventID,ExePipe);								
}


void CExecuter::OnObjectClose(CMsg& Msg){
	int64 EventID  = Msg.GetEventID();
	
	ePipeline& Letter = Msg.GetLetter();
		
	int64 ObjectInstanceID = Letter.PopInt();
	eElectron E;
	Letter.Pop(&E);
	ePipeline* ExePipe     = (ePipeline*)E.Value();
	
	CExObject& Object = FindObject(ObjectInstanceID);
	if (!Object.IsValid())
	{
		SendFeedback(EventID,*ExePipe);
		return;
	}

	if (!Object.WaitForStop(50)) //最多等候一秒
	{
		ExePipe->SetID(RETURN_ERROR);
		ExePipe->GetLabel() = Format1024(_T("Close object timeout:%s"),Object.m_FileAddress.c_str());	
	}
	DeleteObject(ObjectInstanceID);
    SendFeedback(EventID,*ExePipe);
}
void CExecuter::OnObjectGetDoc(CMsg& Msg)
{
	int64 EventID  = Msg.GetEventID();

	ePipeline& Letter = Msg.GetLetter();
	int64 ObjectInstanceID = Letter.PopInt();

	eElectron E;
	Letter.Pop(&E);
	ePipeline* ExePipe     = (ePipeline*)E.Value();

	CExObject& Object = FindObject(ObjectInstanceID);
	if (!Object.IsValid())
	{
		tstring Error =_T("the object is invalid");
		ExePipe->SetID(RETURN_ERROR);
		ExePipe->SetLabel(Error.c_str());	
		SendFeedback(EventID,*ExePipe);
		return;
	}

	Object.GetDoc(this,EventID,ExePipe);								
}

void CExecuter::OnBroadcasgMsg(CMsg& Msg){

	ePipeline& Letter = Msg.GetLetter();

	ePipeline& BcsData = *(ePipeline*)Letter.GetData(0);

	int64 BcsID = BcsData.GetID();

}


void CExecuter::OnWhoAreYou(CMsg& Msg){
	
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
/*
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
*/	
	rLetter.PushString(_T("executer"));
	rLetter.PushString(m_CryptText);
	rLetter.PushInt(m_ID);

	Linker().PushMsgToSend(rMsg);	
}

void CExecuter::OnConnectOK(CMsg& Msg){
	int64 SourceID = Msg.GetSourceID();
	ePipeline& Letter = Msg.GetLetter();
	tstring Name;
	Letter.PopString(Name);
    int64 ReceiverID = Msg.GetReceiverID();
	
	CLinker Linker;
	GetSuperior(SourceID,Linker);
	if(Linker.IsValid()){
		Linker().SetRecoType(LINKER_FRIEND);

		CSuperiorLinkerList* LinkerList = GetSuperiorLinkerList();
		if (SourceID == SPACE_SOURCE_READY)
		{
			Linker().SetSourceID(SPACE_SOURCE);
			OutputLog(LOG_TIP,_T("Connect SpacePort...ok!"));
		}else{
			OutputLog(LOG_TIP,_T("Connect SpacePort... ok!"));	
//			int64 SenderID = Msg.GetSenderID();
//			CMsg rMsg(SenderID,MSG_APP_ONLINE,ReceiverID,0);
//			Linker().PushMsgToSend(rMsg);		
		}
	}
}
