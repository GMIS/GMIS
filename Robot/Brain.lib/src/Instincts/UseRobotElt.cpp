#include "..\Brain.h"
#include "..\LogicDialog.h"
#include "UseRobotElt.h"




//////////////////////////////////////////////////////////////////////////
CCallRobot::CCallRobot(int64 ID,ePipeline* Robot)
	:CElement(ID,_T("call robot")),m_Robot(Robot),m_bResponded(false)
{

};

CCallRobot::~CCallRobot(){
	

	if(m_Robot){
		delete m_Robot;
		m_Robot = NULL;
	}
}

bool  CCallRobot::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){ 

	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);

	if(!Msg.IsReaded()){
		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_REQUEST_RUNTIME){

			int64 EventID = Msg.GetEventID();
			if (EventID != GetEventID())
			{
				return true;
			}

			ePipeline& Letter = Msg.GetLetter(true);

			REQUEST_RUNTIME_CODE code = (REQUEST_RUNTIME_CODE)Letter.PopInt();
			if (code == REQUEST_CREATED)
			{
				//只是在空间登记了事件，对方还没回应，这里预先登记，避免无法取消空间事件。
				int64 InstanceID = Letter.PopInt();
				m_Robot->SetID(InstanceID);

			}
			else if (code == REQUEST_ACCEPTED)
			{
				m_bResponded = true;
				int64 InstanceID = Letter.PopInt();
				assert(m_Robot->Size()==5);
				m_Robot->SetID(InstanceID);
				Dialog->m_RequestFocus = InstanceID;
				Dialog->AddRequestInstance(InstanceID,*m_Robot);

				CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
				nf.PushInt(INSTANCE_OBJECT);
				nf.PushInt(CREATE_INSTANCE);
				nf.Push_Directly(m_Robot->Clone());
				nf.Notify(Dialog);

				ePipeline OldExePipe;
				ExePipe.Clear();
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);

				ExePipe.PushInt(1);
				ExePipe<<OldExePipe;

			}else if(code == REQUEST_STOP){
				m_bResponded = true;
				ePipeline OldExePipe;
				ExePipe.Clear();
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);

				ExePipe.PushInt(0);
				ExePipe<<OldExePipe;

				m_Robot->SetID(0); //避免析构时向对方重复发STOP信息（空间事件已经被删除，再发也无意义）
			}
			return true;
		}else{
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			
			if (MsgID == MSG_EVENT_TICK)
			{
				//检查空间链接是否有效
				CLinker Linker;
				GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);
				if(!Linker.IsValid()){
					ePipeline OldExePipe;
					Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
					ExeError(ExePipe,_T("Space disconnected."));
				}
			}else if(MsgID == MSG_ELT_TASK_CTRL){
				if(Dialog->GetTaskState()==TASK_STOP) //我方主动STOP
				{
					if(!m_bResponded){  
						int64 ReceiverID = m_Robot->GetID();
						if(ReceiverID!=0){
							CMsg NewMsg(SYSTEM_SOURCE,ReceiverID,MSG_CLOSE_REQUEST,DEFAULT_DIALOG,GetEventID());

							CLinker Linker;
							GetBrain()->GetLinker(SPACE_SOURCE,Linker);
							if (Linker.IsValid())
							{
								Linker().PushMsgToSend(NewMsg);
							}
						}
					}
				}
			}

			
			
			if(ret == RETURN_DIRECTLY){
				return true;
			}
		}	
	}
	
	//如果没有启动外部器官，则启动
	CLinker Linker;
	GetBrain()->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{				
		return ExeError(ExePipe,_T("Space disconnected."));
	}

	//动态版本
	if(m_Robot==NULL){
		tstring RobotName = ExePipe.PopString();

		//优先在临时物体列表里找
		vector<CObjectData> ObjectList;
		int n = Dialog->FindObject(RobotName,ObjectList);

		ePipeline* ObjectInfo = NULL;

		if( n == 1 ){
			CObjectData& Ob = ObjectList.front();

			if(Ob.m_Type <ROBOT_VISITER){
				return ExeError(ExePipe,Format1024(_T("the %s is not a robot"),RobotName.c_str()));
			}
			int64 InstanceID = AbstractSpace::CreateTimeStamp();
			Ob.m_ID = InstanceID;

			m_Robot = Ob.Clone();
			//Dialog->m_ObjectFocus = InstanceID;

			if(m_Robot==NULL){
				return ExeError(ExePipe,Format1024(_T("out of memory when calling robot %s "),RobotName.c_str()));
			}
		}
		else
		{
			//原本应该让用户选择，这里暂时简化处理
			return ExeError(ExePipe,Format1024(_T("robot %s is not online or more than one"),RobotName.c_str()));
		}
	}


	//准备好一个请求事件
	UpdateEventID();

	//把请求发给对方
	Reset();
	CMsg NewMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_START_RESQUEST,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter(false);
	Letter.Push_Directly(m_Robot->Clone());
	Letter.PushString(GetBrain()->GetName());

	tstring Right = ExePipe.PopString();
	Letter.PushString(Right);
	
	if(Right == _T("task")){
		tstring LogicText = ExePipe.PopString();
		Letter.PushString(LogicText);
	}

	tstring Name = *(tstring*)m_Robot->GetData(0);

	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = Format1024(_T("Waiting for % replying"),Name.c_str());
	tstring Title      = Format1024(_T("call robot (%d)"),m_ID);
	CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),Title,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,false,false,false);
	if(!Dlg){
		ExeError(ExePipe,_T("create event dialog fail"));
	}

	Dlg->SaveDialogItem(DialogText,Dialog->m_DialogName,0);
	Linker().PushMsgToSend(NewMsg);
	return true;
};

//////////////////////////////////////////////////////////////////////////

CChatRobot::CChatRobot(int64 ID,ePipeline* Robot)
	:CCallRobot(ID,Robot)
{
	m_Name = _T("chat");
};

CChatRobot::~CChatRobot(){

	int64 ReceiverID = m_Robot->GetID();
	if(ReceiverID!=0){
		CMsg NewMsg(SYSTEM_SOURCE,ReceiverID,MSG_REQUEST_RUNTIME,DEFAULT_DIALOG,GetEventID());
		ePipeline& rLetter = NewMsg.GetLetter(false);
		rLetter.PushInt(REQUEST_STOP);

		CLinker Linker;
		GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);
		if (Linker.IsValid())
		{
			Linker().PushMsgToSend(NewMsg);
		}
	}

}

bool  CChatRobot::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){


	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);

	if(!Msg.IsReaded()){
		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_REQUEST_RUNTIME){

			int64 EventID = Msg.GetEventID();
			if (EventID != GetEventID())
			{
				return true;
			}

			ePipeline& Letter = Msg.GetLetter(true);

			REQUEST_RUNTIME_CODE code = (REQUEST_RUNTIME_CODE)Letter.PopInt();
			if (code == REQUEST_CREATED)
			{
				//只是在空间登记了事件，对方还没回应，这里预先登记，避免无法取消空间事件。
				int64 InstanceID = Letter.PopInt();
				m_Robot->SetID(InstanceID);

			}
			else if (code == REQUEST_ACCEPTED)
			{
				int64 InstanceID = Letter.PopInt();
				assert(m_Robot->Size()==5);
				m_Robot->SetID(InstanceID);

				CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());
				assert(EventDlg);
					
				tstring Answer = _T("now you can chat");
				EventDlg->SaveDialogItem(Answer,_T("System"),0);
				EventDlg->EnableInput(true);
				ExePipe.SetID(RETURN_WAIT);

			}else if(code == REQUEST_STOP){
				Dialog->RuntimeOutput(0,_T("chat stoped"));
				m_Robot->SetID(0); //避免析构时向对方重复发STOP信息（空间事件已经被删除，再发也无意义）

				ePipeline OldExePipe;
				ExePipe.Clear();
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
				Dialog->EnableInput(true);

				ExePipe.PushInt(0);
				ExePipe<<OldExePipe;

			}else if (code == REQUEST_OUTPUT)
			{
				ePipeline& Letter = Msg.GetLetter(true);
				if(!Letter.HasTypeAB(PARAM_TYPE1(TYPE_PIPELINE))){
					Dialog->RuntimeOutput(m_ID,_T("Receive a msg(%s) that include invalid data"),GetBrain()->MsgID2Str(MsgID).c_str());
				}else{

					CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());
					tstring Name = *(tstring*)m_Robot->GetData(0);
					ePipeline* Info = (ePipeline*)Letter.GetData(0);
					tstring text = Info->PopString();;	
					EventDlg->SaveDialogItem(text,Name,0);	
				}
			}
			return true;
		}
		else if(MsgID == MSG_TASK_RESULT)
		{	
			int64 EventID = Msg.GetEventID();
			if (EventID != GetEventID())
			{
				return true;
			}

			//来自EventDialog
			ePipeline& Letter = Msg.GetLetter(true);
			ePipeline* NewExePipe = (ePipeline*)Letter.GetData(0);

			ePipeline OldExePipe;

			if (!NewExePipe->IsAlive())
			{	
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
				return RETURN_DIRECTLY; //不在继续执行TaskProc()
			}

			int64 retTask = NewExePipe->GetID();
			if (retTask == RETURN_ERROR)
			{
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
				ExeError(ExePipe,NewExePipe->GetLabel());
				return RETURN_DIRECTLY;
			}

			CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());

			int64 ThinkID = NewExePipe->PopInt();
			CLogicThread* Think = GetBrain()->GetBrainData()->GetLogicThread(ThinkID);
			assert(Think);
			tstring Text = Think->GetUserInput();
			Text = TriToken(Text);

			EventDlg->SaveDialogItem(Text,Dialog->m_SourceName,0);
			EventDlg->ResetThink();
			EventDlg->EnableInput(true);

		
			int64 ReceiverID = m_Robot->GetID();
			CMsg NewMsg(SYSTEM_SOURCE,ReceiverID,MSG_REQUEST_RUNTIME,DEFAULT_DIALOG,GetEventID());
			ePipeline& rLetter = NewMsg.GetLetter(false);
			rLetter.PushInt(REQUEST_OUTPUT);
			ePipeline Info;
			Info.PushString(Text);
			rLetter.PushPipe(Info);

			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);
			if (!Linker.IsValid())
			{	
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
				ExeError(ExePipe,_T("Space disconnected."));		
			}else{
				Linker().PushMsgToSend(NewMsg);
			}
			return true;
		}
		else{
			
			int32 ChildIndex = IT_SELF;
			MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
			
			if (MsgID == MSG_EVENT_TICK)
			{
				//检查空间链接是否有效
				CLinker Linker;
				GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);
				if(!Linker.IsValid()){
					ePipeline OldExePipe;
					Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
					ExeError(ExePipe,_T("Space disconnected."));
				}

			}
			
			if(ret == RETURN_DIRECTLY){
				return true;
			}
		}	
	}

	//如果没有启动外部器官，则启动
	CLinker Linker;
	GetBrain()->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		ExeError(ExePipe,_T("Space disconnected."));
		return true;
	}


	//准备好一个请求事件
	UpdateEventID();

	tstring Name = *(tstring*)m_Robot->GetData(0);

	tstring DialogText = Format1024(_T("Waiting for % replying"),Name.c_str());
	tstring Title = Format1024(_T("chat to %s"),Name.c_str());
	CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),Title,TASK_OUT_THINK,ExePipe,LocalAddress,TIME_SEC,true,false,false);
	if(!Dlg){
		ExeError(ExePipe,_T("create event dialog fail"));
		false;
	}
	Dlg->SaveDialogItem(DialogText,Dialog->m_DialogName);

	Dialog->EnableInput(false);

	Reset();
	CMsg NewMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_START_RESQUEST,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter(false);
	Letter.Push_Directly(m_Robot->Clone());
	Letter.PushString(GetBrain()->GetName());
	Letter.PushString(_T("chat"));
	Linker().PushMsgToSend(NewMsg);


	return true;
}


//////////////////////////////////////////////////////////////////////////

bool CFocusRobotRequest::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	
	tstring RequestName = ExePipe.PopString();
	int64 InstanceID = Dialog->m_NamedRequestList.GetInstanceID(RequestName);
	Dialog->m_RequestFocus = InstanceID;

	if (InstanceID==0)
	{
		ExeError(ExePipe,Format1024(_T("%I64ld: the request %s is not exist"),m_ID,RequestName.c_str()));
	}

	return true;
}


bool  CFocusRobotRequest_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	int64 InstanceID = Dialog->m_NamedRequestList.GetInstanceID(m_FocusName);
	Dialog->m_RequestFocus = InstanceID;
	if (InstanceID==0)
	{
		ExeError(ExePipe,Format1024(_T("%I64ld: the request %s is not exist"),m_ID,m_FocusName.c_str()));
	}

	return true;
};
//////////////////////////////////////////////////////////////////////////

CNameRobotRequest::CNameRobotRequest(int64 ID)
	:CElement(ID,_T("name  request"))
{

}

CNameRobotRequest::~CNameRobotRequest(){

};

bool CNameRobotRequest::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	ePipeline* RequestInstance = Dialog->FindRequestInstance(Dialog->m_RequestFocus);
	if (RequestInstance==NULL)
	{
		return ExeError(ExePipe,Format1024(_T("%I64ld: the focuse request is invalid"),m_ID));
	}

	tstring RobotName = *(tstring*)RequestInstance->GetData(0);
	tstring RequestInstanceName = ExePipe.PopString();

	CNameUser& User =  Dialog->m_NamedRequestList.GetNameUser(RequestInstanceName);
	if (User.m_InstanceID)
	{
		if(User.m_InstanceID != Dialog->m_RequestFocus){
			return ExeError(ExePipe,Format1024(_T("%I64ld: the request instance name %s has be existed"),m_ID,RequestInstanceName.c_str()));
		}
		else{
			return true;
		}
	}

	//如果这个实例之前已经命名则先注销
	Dialog->m_NamedRequestList.UnregisterNameByTask(Dialog->m_RequestFocus);

	Dialog->m_NamedRequestList.RegisterNameByTask(RequestInstanceName,Dialog->m_TaskID,Dialog->m_RequestFocus);

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_OBJECT);
	nf.PushInt(NAME_INSTANCE);
	nf.PushString(RobotName);
	nf.PushInt(Dialog->m_RequestFocus);
	nf.PushString(RequestInstanceName);
	nf.Notify(Dialog);

	return true;
}

CNameRobotRequest_Static::CNameRobotRequest_Static(int64 ID,tstring Name)
	:CElement(ID,_T("name request")),m_Name(Name)
{

}

CNameRobotRequest_Static::~CNameRobotRequest_Static(){

};

bool CNameRobotRequest_Static::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	ePipeline* RequestInstance = Dialog->FindRequestInstance(Dialog->m_RequestFocus);
	if (RequestInstance==NULL)
	{
		return ExeError(ExePipe,Format1024(_T("%I64ld: the focuse request is invalid"),m_ID));
	}

	tstring RobotName = *(tstring*)RequestInstance->GetData(0);
	tstring RequestInstanceName = m_Name;

	CNameUser& User = Dialog->m_NamedRequestList.GetNameUser(RequestInstanceName);
	
	if (User.m_InstanceID)
	{
		if(User.m_InstanceID != Dialog->m_RequestFocus){
			return ExeError(ExePipe,Format1024(_T("%I64ld: the request instance name %s has be existed"),m_ID,RequestInstanceName.c_str()));
		}
		else{
			return true;
		}
	}

	//如果这个实例之前已经命名则先注销
	Dialog->m_NamedRequestList.UnregisterNameByTask(Dialog->m_RequestFocus);

	Dialog->m_NamedRequestList.RegisterNameByTask(RequestInstanceName,Dialog->m_TaskID,Dialog->m_RequestFocus);

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_OBJECT);
	nf.PushInt(NAME_INSTANCE);
	nf.PushString(RobotName);
	nf.PushInt(Dialog->m_RequestFocus);
	nf.PushString(RequestInstanceName);
	nf.Notify(Dialog);

	return true;
}

//////////////////////////////////////////////////////////////////////////

CExecuteRobotRequest::CExecuteRobotRequest(int64 ID)
	:CElement(ID,_T("execute request")),m_WorkingRequestID(0)
{

};

CExecuteRobotRequest::~CExecuteRobotRequest(){
	
	if(m_WorkingRequestID){
		CLinker Linker;
		GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);

		CMsg Msg1(SYSTEM_SOURCE,m_WorkingRequestID,MSG_REQUEST_RUNTIME,DEFAULT_DIALOG,0);
		Msg1.GetLetter(false).PushInt(REQUEST_STOP);

		if(Linker.IsValid()){
			Linker().PushMsgToSend(Msg1);
		}
	}
};


MsgProcState CExecuteRobotRequest::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 MsgID = Msg.GetMsgID();
	if(MsgID == MSG_TASK_RESULT)
	{	
		int64 EventID = Msg.GetEventID();
		if (EventID != GetEventID())
		{
			return RETURN_DIRECTLY;
		}

		int64 SourceID = Msg.GetSourceID();
		if(SourceID == SPACE_SOURCE){

			m_WorkingRequestID = 0;
			ePipeline& Letter = Msg.GetLetter(true);
			ePipeline* NewExePipe= (ePipeline*)Letter.GetData(0);

			ExePipe.Clear();
			if (!NewExePipe->IsAlive())
			{
				ExePipe.PushInt(0);
				Dialog->RuntimeOutput(m_ID,_T("request has been stoped"));

			}else if (NewExePipe->GetID() == RETURN_ERROR)
			{
				ExePipe.PushInt(0);//execute fail
				tstring error = NewExePipe->m_Label;
				if(error.size()==0){
					error = _T("some unknown error occurred while executing request ");
				}
				Dialog->RuntimeOutput(m_ID,error.c_str());
			}else{

				ExePipe.PushInt(1);  //execute success
				ExePipe << *NewExePipe;
			}

			ePipeline OldExePipe;
			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
		}

		return RETURN_DIRECTLY; // 不在继续执行TaskProc
	}
	else if (MsgID == MSG_REQUEST_RUNTIME)
	{
		ePipeline& Letter = Msg.GetLetter(true);

		REQUEST_RUNTIME_CODE code = (REQUEST_RUNTIME_CODE)Letter.PopInt();
		if (code == REQUEST_OUTPUT)
		{
			ePipeline* Info = (ePipeline*)Letter.GetData(0);
			tstring text = Info->PopString();
			Dialog->RuntimeOutput(m_ID,text);
		} 
		else if (code == REQUEST_STOP) //对方终止了任务
		{
			//我方关闭对话实例
			int64 InstanceID = m_WorkingRequestID;
			ePipeline* RequestInstance = Dialog->FindRequestInstance(Dialog->m_RequestFocus);

			if(RequestInstance == NULL)
			{
				ExeError(ExePipe,_T("the focus request is not exist."));
				return RETURN_DIRECTLY;
			}

			tstring RobotName = *(tstring*)RequestInstance->GetData(0);

			CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
			nf.PushInt(INSTANCE_OBJECT);
			nf.PushInt(CLOSE_INSTANCE);
			nf.PushString(RobotName);
			nf.PushInt(InstanceID);
			nf.Notify(Dialog);

			Dialog->m_NamedRequestList.UnregisterNameByTask(InstanceID);

			Dialog->CloseRequestInstance(InstanceID);
			m_WorkingRequestID = 0;


			ExePipe.Clear();
			ExePipe.PushInt(0);  //execute fail
			
			ePipeline OldExePipe;
			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
		}		
	}
	else{
		ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);

		if (MsgID == MSG_EVENT_TICK)
		{
			//检查空间链接是否有效
			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);
			if(!Linker.IsValid()){

				//我方关闭对话实例
				int64 InstanceID = m_WorkingRequestID;
				ePipeline* RequestInstance = Dialog->FindRequestInstance(Dialog->m_RequestFocus);

				if(RequestInstance == NULL)
				{
					ExeError(ExePipe,_T("the focus request is not exist."));
					return RETURN_DIRECTLY;
				}

				tstring RobotName = *(tstring*)RequestInstance->GetData(0);

				CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
				nf.PushInt(INSTANCE_OBJECT);
				nf.PushInt(CLOSE_INSTANCE);
				nf.PushString(RobotName);
				nf.PushInt(InstanceID);
				nf.Notify(Dialog);

				Dialog->m_NamedRequestList.UnregisterNameByTask(InstanceID);
				Dialog->CloseRequestInstance(InstanceID);
				m_WorkingRequestID = 0;

				ePipeline OldExePipe;
				Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
				ExeError(ExePipe,_T("Space disconnected."));

			}

		}
		return ret;
	}

	return RETURN_DIRECTLY;
}

bool CExecuteRobotRequest::TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	
	ePipeline* RequestInstance = Dialog->FindRequestInstance(Dialog->m_RequestFocus);

	if (RequestInstance==NULL)
	{
		ExeError(ExePipe,_T("the focus request is not exist."));
		return true;
	}
	
	//这个子对话主要起提示作用，实际任务会交给远方执行
	tstring DialogText = _T("Waiting for the request result");
	UpdateEventID();
	CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),_T("execute request"),TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,true,false,true);
	if(!Dlg){
		ExeError(ExePipe,_T("create event dialog fail"));
		return false;
	}
	Dlg->SaveDialogItem(DialogText,Dialog->m_DialogName);

	m_WorkingRequestID = Dialog->m_RequestFocus;

	CMsg NewMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_EXECUTE_REQUEST,DEFAULT_DIALOG,GetEventID());
	ePipeline& Letter = NewMsg.GetLetter(false);
	Letter.PushPipe(*RequestInstance);
	Letter.PushPipe(ExePipe);

	//如果没有启动外部器官，则启动
	CLinker Linker;
	GetBrain()->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		m_WorkingRequestID = 0;
		ePipeline OldExePipe;
		Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);
		ExeError(ExePipe,_T("Spaceport disconnect."));
		return true;
	}
	Linker().PushMsgToSend(NewMsg);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CCloseRobotRequest::CCloseRobotRequest(int64 ID)
	:CElement(ID,_T("close request"))
{
}

CCloseRobotRequest::~CCloseRobotRequest(){

}

bool  CCloseRobotRequest::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){ 

	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);

	if(!Msg.IsReaded()){

		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}

	}

	ePipeline* RequestInstance = Dialog->FindRequestInstance(Dialog->m_RequestFocus);
	if (RequestInstance==NULL)
	{
		return true;
	}

	assert(RequestInstance->GetID() ==Dialog->m_RequestFocus );

	int64 InstanceID = Dialog->m_RequestFocus;
	tstring RobotName = *(tstring*)RequestInstance->GetData(0);

	CNotifyDialogState nf(NOTIFY_MEMORY_VIEW);
	nf.PushInt(INSTANCE_OBJECT);
	nf.PushInt(CLOSE_INSTANCE);
	nf.PushString(RobotName);
	nf.PushInt(InstanceID);
	nf.Notify(Dialog);

	Dialog->m_NamedRequestList.UnregisterNameByTask(Dialog->m_RequestFocus);

	Dialog->CloseRequestInstance(InstanceID);


	//如果没有启动外部器官，则启动
	CLinker Linker;
	GetBrain()->GetLinker(SPACE_SOURCE,Linker);
	if (!Linker.IsValid())
	{		
		ExePipe.GetLabel() = Format1024(_T("Space disconnected."));			
		return true;
	}

	//把请求发给对方
	int64 ReceiverID = InstanceID;

	CMsg NewMsg(SYSTEM_SOURCE,ReceiverID,MSG_CLOSE_REQUEST,DEFAULT_DIALOG,GetEventID());
	Linker().PushMsgToSend(NewMsg);
	return true;
};
