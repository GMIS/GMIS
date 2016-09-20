#pragma warning (disable:4786)

#include "SpacePortal.h"
#include "SpaceMsgList.h"
#include "Win32Tool.h"

void    CSpacePortal::NerveMsgProc(CMsg& Msg){
	int64 MsgID = Msg.GetMsgID();

	switch(MsgID){
	case MSG_ROBOT_START_OBJECT:
		OnStartObject(Msg);
		break;
	case MSG_ROBOT_USE_OBJECT:
		OnUseObject(Msg);
		break;
	case MSG_ROBOT_CLOSE_OBJECT:
		OnCloseObject(Msg);
		break;
	case MSG_ROBOT_GET_OBJECT_DOC:
		OnGetObjectDoc(Msg);
		break;
	case MSG_START_RESQUEST:
		OnStartRequest(Msg);
		break;
	case MSG_EXECUTE_REQUEST:
		OnExecuteRequest(Msg);
		break;
	case MSG_REQUEST_RUNTIME:
		OnRequestRuntime(Msg);
		break;
	case MSG_CLOSE_REQUEST:
		OnCloseRequest(Msg);
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
	default:
		{
			tstring MsgStr = MsgID2Str(MsgID);
			tstring s = Format1024(_T("Unknown Msg: %I64ld(%s)"),MsgID,MsgStr.c_str());
			OutputLog(LOG_WARNING,s.c_str());
		}
	}	
}

void   CSpacePortal::OnStartObject(CMsg& Msg){
	
	int64 SourceID = Msg.GetSourceID();
	int64 EventID = Msg.GetEventID();

	CLinker Requester;
	GetLinker(SourceID,Requester);

	ePipeline& Letter = Msg.GetLetter(true);
	if (!Letter.HasTypeAB(PARAM_TYPE2(TYPE_PIPELINE,TYPE_PIPELINE)))
	{
		CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = FeedbackMsg.GetLetter(false);
		ePipeline ExePipe(RETURN_ERROR);
		ExePipe.m_Label = _T("param error");
		Letter.PushPipe(ExePipe);  
		Requester().PushMsgToSend(FeedbackMsg);
	
		return;
	}
	ePipeline* ObjectInfo = (ePipeline*)Letter.GetData(0);
	ePipeline* ExePipe = (ePipeline*)Letter.GetData(1);

	CObjectData Object(*ObjectInfo);

	ePipeline AddrData = Object.m_Address;

	//Checks whether the specified object addresses is valid
	SpaceAddress Address = FindChildSpace(AddrData,Object.m_Fingerprint);
	if (!Address.IsValid())
	{

		CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = FeedbackMsg.GetLetter(false);
		ExePipe->SetID(RETURN_ERROR);
		ExePipe->m_Label = _T("object address invalid");
		Letter.PushPipe(*ExePipe);  
		Requester().PushMsgToSend(FeedbackMsg);

		return;
	}

	//Need to open the corresponding DLL object in advance, in order to determine the type of executer

	//CSpace Space(Address.ParentID,Address.ChildID);	  

	AddrData = Object.m_Address;

	tstring FilePath = SpacePath2FileAddress(AddrData);
	tstring ObjectName = GetFileName(Object.m_Name); //不含扩展名

	FilePath += _T("\\")+ObjectName;
	FilePath += _T("\\")+Object.m_Name+_T(".exe");

	

	ePipeline EventData;
	EventData.PushPipe(*ExePipe);

	int64 SpaceEventID = PushEvent(EXE_ROBOT,SourceID,EventID,EventData,EXE_SELF,0,0);

	
	OutputLog(LOG_TIP,FilePath.c_str());

	STARTUPINFO si;
	PROCESS_INFORMATION  pi;	
	::ZeroMemory(&si, sizeof(si));	
	si.cb = sizeof(si);

	tstring CryptText = _T("GMIS_")+ObjectName;
	tstring CmdLine = Format1024(_T(" %s  %I64ld"),CryptText.c_str(),SpaceEventID);

	TCHAR CmdLineBuf[512];
	_tcscpy(CmdLineBuf,CmdLine.c_str());

	BOOL started = ::CreateProcessW(FilePath.c_str(),        // command is part of input string
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
		PopEvent(SpaceEventID);

		ExePipe->m_Label = Format1024(_T("Start Executer(%s) fail"),ObjectName.c_str());
		ExePipe->SetID(RETURN_ERROR);
	

		CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,EventID);
		ePipeline& Letter = FeedbackMsg.GetLetter(false);

		Letter.PushPipe(*ExePipe);  
		Requester().PushMsgToSend(FeedbackMsg);
		return;
	}

	tstring s = Format1024(_T("Wait object(%s) starting..."),ObjectName.c_str());
	OutputLog(LOG_TIP,s.c_str());

};


void   CSpacePortal::OnUseObject(CMsg& Msg){

	int64 SourceID = Msg.GetSourceID();
	CLinker Requester;
	GetLinker(SourceID,Requester);

	ePipeline& Letter = Msg.GetLetter(true);
	if (!Letter.HasTypeAB(PARAM_TYPE2(TYPE_INT,TYPE_PIPELINE)))
	{

		CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,Msg.GetEventID());
		ePipeline& Letter = FeedbackMsg.GetLetter(false);
		ePipeline ExePipe( _T("param error"),RETURN_ERROR);
		Letter.PushPipe(ExePipe);  
		Requester().PushMsgToSend(FeedbackMsg);
		return;
	}

	int64 ObjectInstanceID = Letter.PopInt();

	ePipeline* ExePipe = (ePipeline*)Letter.GetData(0);

	CLinker ObjectLinker;
	GetLinker(ObjectInstanceID,ObjectLinker);

	if (ObjectInstanceID==0 || !ObjectLinker.IsValid())
	{		  
		ExePipe->SetID(RETURN_ERROR);
		ExePipe->m_Label = _T("object is not started.");

		CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,Msg.GetEventID());
		ePipeline& Letter = FeedbackMsg.GetLetter(false);

		Letter.PushPipe(*ExePipe);  
		Requester().PushMsgToSend(FeedbackMsg);

		return;
	}
	ePipeline EventData;
	int64 SpaceEventID = PushEvent(EXE_ROBOT,SourceID,Msg.GetEventID(),EventData,EXE_OBJECT,ObjectInstanceID,0);


	CMsg NewMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_OBJECT_RUN,SpaceEventID,0);
	ePipeline& NewLetter = NewMsg.GetLetter(false);
	NewLetter.PushPipe(*ExePipe);

	ObjectLinker().PushMsgToSend(NewMsg);

};

void    CSpacePortal::OnGetObjectDoc(CMsg& Msg)
{
	int64 SourceID = Msg.GetSourceID();
	CLinker Requester;
	GetLinker(SourceID,Requester);

	ePipeline& Letter = Msg.GetLetter(true);
	if (!Letter.HasTypeAB(PARAM_TYPE2(TYPE_INT,TYPE_PIPELINE)))
	{

		CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,Msg.GetEventID());
		ePipeline& Letter = FeedbackMsg.GetLetter(false);
		ePipeline ExePipe(_T("param error"),RETURN_ERROR);
		Letter.PushPipe(ExePipe);  
		Requester().PushMsgToSend(FeedbackMsg);
		return;
	}

	int64 ObjectInstanceID = Letter.PopInt();
	ePipeline* ExePipe = (ePipeline*)Letter.GetData(0);

	CLinker ObjectLinker;
	GetLinker(ObjectInstanceID,ObjectLinker);

	if (!ObjectLinker.IsValid())
	{		  
		ExePipe->SetID(RETURN_ERROR);
		ExePipe->m_Label = _T("executer not started.");
		CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,Msg.GetEventID());
		ePipeline& Letter = FeedbackMsg.GetLetter(false);

		Letter.PushPipe(*ExePipe);  
		Requester().PushMsgToSend(FeedbackMsg);
		return;
	}

	ePipeline EventData;
	int64 SpaceEventID = PushEvent(EXE_ROBOT,SourceID,Msg.GetEventID(),EventData,EXE_OBJECT,ObjectInstanceID,0);

	CMsg NewMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_OBJECT_GET_DOC,SpaceEventID,0);
	ePipeline& NewLetter = NewMsg.GetLetter(false);
	NewLetter.PushPipe(*ExePipe);

	ObjectLinker().PushMsgToSend(NewMsg);
};

void    CSpacePortal::OnCloseObject(CMsg& Msg){
	int64 SourceID = Msg.GetSourceID();
	CLinker Requester;
	GetLinker(SourceID,Requester);

	ePipeline& Letter = Msg.GetLetter(true);
	if (!Letter.HasTypeAB(PARAM_TYPE2(TYPE_INT,TYPE_PIPELINE)))
	{

		CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,Msg.GetEventID());
		ePipeline& Letter = FeedbackMsg.GetLetter(false);
		ePipeline ExePipe(_T("param error"),RETURN_ERROR);
		Letter.PushPipe(ExePipe);  
		Requester().PushMsgToSend(FeedbackMsg);
		return;
	}

	int64 ObjectInstanceID = Letter.PopInt();

	CLinker ObjectLinker;
	GetLinker(ObjectInstanceID,ObjectLinker);
	if (!ObjectLinker.IsValid())
	{				

		CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,Msg.GetEventID());
		ePipeline& Letter = FeedbackMsg.GetLetter(false);
		
		ePipeline ExePipe;
		Letter.PushPipe(ExePipe);  
		Requester().PushMsgToSend(FeedbackMsg);
		
		return;
	}
	
	//通知物体关闭
	//ePipeline EventData = *ExePipe;
	//int64 SpaceEventID = PushEvent(Msg.GetEventID(),EXE_ROBOT,SourceID,ObjectInstanceID,EventData);

	CMsg NewMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_OBJECT_CLOSE,DEFAULT_DIALOG,0);
    ObjectLinker().PushMsgToSend(NewMsg);


	//直接给请求者回信表示关闭
	CMsg FeedbackMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,Msg.GetEventID());
	ePipeline& Letter1 = FeedbackMsg.GetLetter(false);
	
	ePipeline ExePipe;
	Letter1.PushPipe(ExePipe);  
	Requester().PushMsgToSend(FeedbackMsg);
}

void CSpacePortal::OnStartRequest(CMsg& Msg){

	int64 SourceID = Msg.GetSourceID();
	int64 EventID  = Msg.GetEventID();

	CLinker Requester;
	GetClientLinkerList()->GetLinker(SourceID,Requester);

	ePipeline& Letter = Msg.GetLetter(true);
	if (!Letter.HasTypeAB(PARAM_TYPE3(TYPE_PIPELINE,TYPE_STRING,TYPE_STRING)))
	{
		FeedbackError(Requester,EventID,_T("param error"),NULL);
		return;
	};

	ePipeline* ObjectInfo = (ePipeline*)Letter.GetData(0);
	tstring*   Caller     = (tstring*)Letter.GetData(1);

	CObjectData Object(*ObjectInfo);
	if(Object.m_Type > OBJECT_SPACE){ //robot

		CObjectData& Robot = Object;
		int64 RobotLinkerID = GetVisitorLinkerID(Robot.m_Name,Robot.m_Fingerprint);
		if(!RobotLinkerID){
			FeedbackError(Requester,EventID,Format1024(_T("the %s is not online"),Robot.m_Name.c_str()),NULL);
			return;
		}


		int32 EventNum = CountingClientEventNum(SourceID);
		if (EventNum>=m_MaxEventNumPerLinker)
		{
			FeedbackError(Requester,EventID,_T("the event num is out of the limit"),NULL);
			return;
		}


		tstring s = Format1024(_T("robot %s call %s"),Caller->c_str(), Robot.m_Name.c_str());
		OutputLog(LOG_TIP,s.c_str());

		ePipeline EventData;
		EventData.PushString(*Caller);
		EventData.PushPipe(*ObjectInfo);

		int64 SpaceEventID = PushEvent(EXE_ROBOT,SourceID,EventID,EventData,EXE_ROBOT,RobotLinkerID,0);

		eElectron e;
		Letter.Pop(&e);
		Letter.Pop(&e);

		CMsg NewMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_START_RESQUEST,DEFAULT_DIALOG,SpaceEventID);
		ePipeline& NewLetter = NewMsg.GetLetter(false);
		NewLetter.PushString(*Caller);
		NewLetter<<Letter;

		CLinker RobotLinker;
		GetClientLinkerList()->GetLinker(RobotLinkerID,RobotLinker);
		if(RobotLinker.IsValid()){
			RobotLinker().PushMsgToSend(NewMsg);
            
			//向请求发起者通报事件已经生成，这样才能避免如果对方没应答发起者就不能关闭事件的情况。
			CMsg rMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_REQUEST_RUNTIME,DEFAULT_DIALOG,EventID);
			ePipeline& rLetter = rMsg.GetLetter(false);
			rLetter.PushInt(REQUEST_CREATED);
			rLetter.PushInt(SpaceEventID);

			Requester().PushMsgToSend(rMsg);
		}else{
			PopEvent(SpaceEventID);
			FeedbackError(Requester,EventID,Format1024(_T("the robot %s is not online"),Robot.m_Name.c_str()),NULL);
		}
		return;
	}

}

void CSpacePortal::OnRequestRuntime(CMsg& Msg){
	int64 SpaceEventID = Msg.GetReceiverID();
	int64 SourceID = Msg.GetSourceID();

	ePipeline& Letter = Msg.GetLetter(true);

	REQUEST_RUNTIME_CODE code = (REQUEST_RUNTIME_CODE)Letter.PopInt();
		
	CSpaceEvent SpaceEvent;

	switch(code){
	case REQUEST_STOP:
		{
		
			if(!GetEvent(SpaceEventID,SpaceEvent)){
				assert(0);
				return;
			}


			
			if(SpaceEvent.m_ClientLinkerID == SourceID){

				if(SpaceEvent.m_bIsBusy){
					SpaceEvent.m_bIsBusy = false;
					ModifyEvent(SpaceEventID,SpaceEvent);
				}
				if (SpaceEvent.m_ExecuterEventID!=0) //对方已经回复同意的才能通知到对方
				{
					CLinker  ExecuterLinker;
					GetClientLinkerList()->GetLinker(SpaceEvent.m_ExecuterLinkerID,ExecuterLinker);
					if(ExecuterLinker.IsValid()){
						CMsg rMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_REQUEST_RUNTIME,SpaceEventID,SpaceEvent.m_ExecuterEventID);
						ePipeline& rLetter = rMsg.GetLetter(false);
						rLetter.PushInt(code);
						rLetter<< Letter;
						ExecuterLinker().PushMsgToSend(rMsg);
					}
				}
				
			}else if(SpaceEvent.m_ExecuterLinkerID == SourceID){
				
				if(SpaceEvent.m_bIsBusy){
					SpaceEvent.m_bIsBusy = false;
					ModifyEvent(SpaceEventID,SpaceEvent);
				}

				CLinker ClientLinker;
				GetClientLinkerList()->GetLinker(SpaceEvent.m_ClientLinkerID,ClientLinker);
				if(ClientLinker.IsValid()){
					CMsg rMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_REQUEST_RUNTIME,SpaceEventID,SpaceEvent.m_ClientEventID);
					ePipeline& rLetter = rMsg.GetLetter(false);
					rLetter.PushInt(code);
					rLetter<< Letter;
					ClientLinker().PushMsgToSend(rMsg);
				}
			}
		}
		return;
	case REQUEST_ACCEPTED:
		{

			if(!GetEvent(SpaceEventID,SpaceEvent)){
				CLinker Linker;
				GetClientLinkerList()->GetLinker(SourceID,Linker);
				
				CMsg rMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_CLOSE_REQUEST,DEFAULT_DIALOG,Msg.GetEventID());
				Linker().PushMsgToSend(rMsg);
				return;
			}

			assert(SpaceEvent.m_ExecuterLinkerID == SourceID);

			CLinker ClientLinker;
			GetClientLinkerList()->GetLinker(SpaceEvent.m_ClientLinkerID,ClientLinker);
			if(ClientLinker.IsValid()){

				SpaceEvent.m_ExecuterEventID = Msg.GetEventID();
				ModifyEvent(SpaceEventID,SpaceEvent);

				CMsg rMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_REQUEST_RUNTIME,SpaceEventID,SpaceEvent.m_ClientEventID);
				ePipeline& rLetter = rMsg.GetLetter(false);
				rLetter.PushInt(code);
				rLetter<< Letter;
				ClientLinker().PushMsgToSend(rMsg);
			}//链接断开的情况会另有专门处理
		}
		break;
	case REQUEST_OUTPUT:
	case REQUEST_LOGIC_TASK:
	case REQUEST_LOGIC_RESULT:
	case REQUEST_DOC:
		{
			if(!GetEvent(SpaceEventID,SpaceEvent)){
				assert(0);
				return;
			}
			int64 SourceID = Msg.GetSourceID();
			if(SpaceEvent.m_ClientLinkerID == SourceID){
				CLinker  ExecuterLinker;
				GetClientLinkerList()->GetLinker(SpaceEvent.m_ExecuterLinkerID,ExecuterLinker);
				if(ExecuterLinker.IsValid() && SpaceEvent.m_ExecuterEventID){
					CMsg rMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_REQUEST_RUNTIME,SpaceEventID,SpaceEvent.m_ExecuterEventID);
					ePipeline& rLetter = rMsg.GetLetter(false);
					rLetter.PushInt(code);
					rLetter<< Letter;
					ExecuterLinker().PushMsgToSend(rMsg);
				}
			}else if(SpaceEvent.m_ExecuterLinkerID == SourceID){
				CLinker ClientLinker;
				GetClientLinkerList()->GetLinker(SpaceEvent.m_ClientLinkerID,ClientLinker);
				if(ClientLinker.IsValid() && SpaceEvent.m_ClientEventID){
					CMsg rMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_REQUEST_RUNTIME,SpaceEventID,SpaceEvent.m_ClientEventID);
					ePipeline& rLetter = rMsg.GetLetter(false);
					rLetter.PushInt(code);
					rLetter<< Letter;
					ClientLinker().PushMsgToSend(rMsg);
				}
			}
		}
		break;
	}
}

void CSpacePortal::OnExecuteRequest(CMsg& Msg){

	int64 SourceID = Msg.GetSourceID();
	int64 EventID = Msg.GetEventID();

	CLinker Requester;
	GetLinker(SourceID,Requester);

	ePipeline& Letter = Msg.GetLetter(true);

	if (!Letter.HasTypeAB(PARAM_TYPE2(TYPE_PIPELINE,TYPE_PIPELINE)))
	{
		FeedbackError(Requester,EventID,_T("param error"),NULL);
		return;
	}

	ePipeline* ObjectInfo = (ePipeline*)Letter.GetData(0);
	ePipeline* ExePipe   = (ePipeline*)Letter.GetData(1);

	CObjectData Object(*ObjectInfo);

	if (Object.m_Type>OBJECT_SPACE)
	{
		CObjectData& Robot = Object;
		tstring s = Format1024(_T("require %s to execute a request"),Robot.m_Name.c_str());
		OutputLog(LOG_TIP,s.c_str());

		int64 SpaceEventID = Robot.m_ID;
		CSpaceEvent SpaceEvent;
		if(!GetEvent(SpaceEventID,SpaceEvent)){
			FeedbackError(Requester,EventID,Format1024(_T("the request %I64ld is not exist"),Robot.m_ID),ExePipe);
			return;
		}

		if(SpaceEvent.m_bIsBusy){
			FeedbackError(Requester,EventID,Format1024(_T("the request %I64ld is executing"),Robot.m_ID),ExePipe);
			return;
		}


		SpaceEvent.m_bIsBusy = true;
		SpaceEvent.m_ClientEventID = EventID;

		ModifyEvent(SpaceEventID,SpaceEvent);

		CMsg NewMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_EXECUTE_REQUEST,SpaceEventID,SpaceEvent.m_ExecuterEventID);
		ePipeline& NewLetter = NewMsg.GetLetter(false);
		NewLetter.PushPipe(*ExePipe);

		CLinker RobotLinker;
		GetClientLinkerList()->GetLinker(SpaceEvent.m_ExecuterLinkerID,RobotLinker);
		if(RobotLinker.IsValid()){
			RobotLinker().PushMsgToSend(NewMsg);
		}else{
			FeedbackError(Requester,EventID,Format1024(_T("the robot %s is not online"),Robot.m_Name.c_str()),ExePipe);
		}
	} 
	else
	{

	}

	return;

}

void   CSpacePortal::OnCloseRequest(CMsg& Msg){
	
	int64 SourceID = Msg.GetSourceID();
	int64 SpaceEventID = Msg.GetReceiverID();

	CSpaceEvent SpaceEvent;
	if(!GetEvent(SpaceEventID,SpaceEvent)){
		assert(0);
		return;
	}

	if(SpaceEvent.m_ClientLinkerID == SourceID){

		CLinker ExecuterLinker;
		GetClientLinkerList()->GetLinker(SpaceEvent.m_ExecuterLinkerID,ExecuterLinker);
		if(ExecuterLinker.IsValid() && SpaceEvent.m_ExecuterEventID){
			CMsg Msg1(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_CLOSE_REQUEST,DEFAULT_DIALOG,SpaceEvent.m_ExecuterEventID);
			ExecuterLinker().PushMsgToSend(Msg1);
		}
		PopEvent(SpaceEventID);

	}else if (SpaceEvent.m_ExecuterLinkerID == SourceID)
	{
		CLinker ClientLinker;
		GetClientLinkerList()->GetLinker(SpaceEvent.m_ClientLinkerID,ClientLinker);
		if(ClientLinker.IsValid() && SpaceEvent.m_ClientEventID){
			CMsg Msg1(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_REQUEST_RUNTIME,DEFAULT_DIALOG,SpaceEvent.m_ClientEventID);
			ePipeline& Letter = Msg1.GetLetter(false);
			Letter.PushInt(REQUEST_STOP);
			Letter.PushInt(SpaceEventID);
			ClientLinker().PushMsgToSend(Msg1);
		}
		PopEvent(SpaceEventID);
	}
};

void CSpacePortal::OnGotoSpace(CMsg& Msg){

	int64 SourceID = Msg.GetSourceID();
	int64 EventID  = Msg.GetEventID();

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

	ePipeline& Letter = Msg.GetLetter(true);
	
	SpaceAddress Addr;

	ePipeline* Path = (ePipeline*)Letter.GetData(0);

	int64 SpaceID = Path->GetID();

	Addr = ::FindSpace(*Path);
	if (!Addr.IsValid())
	{
		CMsg rMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_SPACE_ACCESS_DENIED,DEFAULT_DIALOG,EventID);
		
		ePipeline& rLetter = rMsg.GetLetter(false);
		rLetter.PushInt(SpaceID);
		rLetter.PushString(_T("Address invalid"));
		Who().PushMsgToSend(rMsg);	
		return;
	};


	ROOM_SPACE Space(Addr.ParentID,Addr.ChildID);
	

	if(Space.AllowInto(*Robot)){
		Robot->GoInto(SourceID,Space);
	}else{
		CMsg rMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_SPACE_ACCESS_DENIED,DEFAULT_DIALOG,EventID);
		ePipeline& rLetter = rMsg.GetLetter(false);
		rLetter.PushInt(SpaceID);
		rLetter.PushString(_T("right invalid"));
		Who().PushMsgToSend(rMsg);	
		return;
	}
	

	ePipeline Pipe;
	GetAllChildList(Robot->GetParentID(),Pipe,Robot->GetSpaceID());
		
    if (Addr.ChildID == LOCAL_SPACEID)
    {
		tstring Name = GetRootSpace().GetName();
		
		Pipe.SetLabel(Name.c_str());
    }

	CMsg rMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_SPACE_CATALOG,DEFAULT_DIALOG,EventID);
	ePipeline& rLetter = rMsg.GetLetter(false);

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

		CMsg rMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_SPACE_ADDED,DEFAULT_DIALOG,0);
		ePipeline& rLetter = rMsg.GetLetter(false);
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
	
	ePipeline& Letter = Msg.GetLetter(true);

	int64 FatherID = Letter.PopInt();
	int64 ChildID  = Letter.PopInt();
	
	CMsg rMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_SPACE_SEND_OBJECT,DEFAULT_DIALOG,0);
	ePipeline& rLetter = rMsg.GetLetter(false);
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

	ePipeline& Letter = Msg.GetLetter(true);
	SPACETYPE  Type = (SPACETYPE)Letter.PopInt();    

    if(Type == OBJECT_SPACE)
	{
		CreateObject(ParentID,Letter);
    }else{
        CreateSpace(ParentID,Type,Letter);
    }
	
};



void CSpacePortal::CreateSpace(int64 ParentID,SPACETYPE SpaceType,ePipeline& Letter){

	tstring SpaceName  = Letter.PopString();

	int64 ChildID = AbstractSpace::CreateTimeStamp();
	
	tstring Fingerprint = Format1024(_T("%I64ld"),ChildID);
	ROOM_SPACE Space(ParentID,ChildID,SpaceName.c_str(),0,SpaceType,FREE,Fingerprint);

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
		ChildPipe.PushString(SpaceName);
		ChildPipe.PushInt(SpaceType);
		ChildPipe.PushString(Fingerprint);

		CMsg Msg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_SPACE_ADDED,DEFAULT_DIALOG,0);

		Msg.GetLetter(false).PushPipe(ChildPipe);		
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
	
	ePipeline& Letter = Msg.GetLetter(true);

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
	CMsg rMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_SPACE_DELETED,DEFAULT_DIALOG,0);
	ePipeline& rLetter = rMsg.GetLetter(false);
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
		
		CMsg rMsg1(SPACE_SOURCE,DEFAULT_DIALOG,MSG_SPACE_DELETED,DEFAULT_DIALOG,0);

		ePipeline& rLetter1 = rMsg1.GetLetter(false);
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

	ePipeline& Letter = Msg.GetLetter(true);

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
	CMsg rMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_OPERATOR_FEEDBACK,DEFAULT_DIALOG,EventID);
	rMsg.GetLetter(false).PushInt(Success);
	rMsg.GetReceiver() << Msg.GetSender();
	Who().PushMsgToSend(rMsg);
}