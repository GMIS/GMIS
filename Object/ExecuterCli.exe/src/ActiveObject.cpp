#include "ActiveObject.h"
#include "SpaceMsgList.h"
#include <tchar.h>



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CActiveObject::CActiveObject(int argc, _TCHAR* argv[])
	:Model(NULL,NULL)
{
	m_ID=0;
	m_LogFlag =  LOG_TIP|LOG_WARNING|LOG_MSG_PROC_BEGIN;//|LOG_MSG_IO_PUSH|LOG_MSG_I0_RECEIVED|LOG_MSG_IO_SENDED;
	m_bAutoWork = false;  //avoid auto running Do() in a new thread, we will run it in main thread of process

	static CUserTimer     Timer;
	InitTimer(&Timer);

	ePipeline CmdPipe;	
	for(int i=1; i<argc; i++)
	{
		tstring s = argv[i];
		CmdPipe.PushString(s);
	}

	if (CmdPipe.Size()!=2)
	{
		return;
	}		
	m_CryptText = CmdPipe.PopString();
	tstring s = CmdPipe.PopString();
	m_ID = _ttoi64(s.c_str());

	tstring tip = Format1024(_T("%s:%I64ld activate ok"),GetName().c_str(),m_ID);
	OutputLog(LOG_TIP,tip.c_str());

	tstring ip = _T("127.0.0.1");
	CMsg Msg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_CONNECT_TO,DEFAULT_DIALOG,0);
	ePipeline& Letter = Msg.GetLetter(false);
	Letter.PushInt(SYSTEM_SOURCE);
	Letter.PushString(ip);
	Letter.PushInt(SPACE_PORT);
	Letter.PushInt(5);  //等待5秒

	PushCentralNerveMsg(Msg,false,false);
}

CActiveObject::~CActiveObject()
{
};

bool	CActiveObject::Activate(){
	if(m_ID==0)return false;
	m_ModelData.SetMaxNerveWorkerNum(2);
	m_Alive = true;
	return m_Alive;
}



tstring CActiveObject::MsgID2Str(int64 MsgID){
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


void CActiveObject::OutputLog(uint32 Flag,const wchar_t* text){
	if (m_LogFlag & Flag)
	{
		int64 t=CreateTimeStamp();
		tstring s = GetTimer()->GetHMSM(t);

		std::wcout<<s.c_str()<<_T("   ")<<text<<endl;
	}

};

void CActiveObject::NotifyLinkerState(int64 SourceID,int64 NotifyID,STATE_OUTPUT_LEVEL Flag,ePipeline& Info){

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
					tstring tip = Format1024(_T("LINKER_PUSH_MSG: %s EventID:%I64ld PendingMsg=%s CurMsgNum=%I64ld"),MsgName.c_str(),EventID,PendingMsgName.c_str(),MsgNum+UrgMsgNum);
					OutputLog(LOG_MSG_IO_PUSH,tip.c_str());
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
					ePipeline& Letter = Msg.GetLetter(false)
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
						tstring tip = Format1024(_T("LINKER_MSG_SENDED: %s  EventID:%I64ld"),MsgName.c_str(),SendedMsg.GetEventID());
						OutputLog(LOG_MSG_IO_SENDED,tip.c_str());
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
							tstring tip = Format1024(_T("Remote received msg:%s "),PendingMsgName.c_str());
							OutputLog(LOG_MSG_IO_REMOTE_RECEIVED,tip.c_str());
						}
					} 
					else
					{	
						tstring MsgName = MsgID2Str(MsgID);
						tstring tip = Format1024(_T("Msg received: %s "),MsgName.c_str());
						OutputLog(LOG_MSG_I0_RECEIVED,tip.c_str());
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

void CActiveObject::GetSuperior(int64 ID,CLinker& Linker){
	CSuperiorLinkerList* List = GetSuperiorLinkerList();
	List->GetLinker(ID,Linker);
}

void  CActiveObject::SendRuntimeInfo(int64 ReceiverID,ePipeline& Info){
	CMsg rMsg(SYSTEM_SOURCE,ReceiverID,MSG_TASK_RUNTIME_INFO,DEFAULT_DIALOG,0);
	ePipeline& rLetter = rMsg.GetLetter(false);
	rLetter.PushPipe(Info);

	CLinker Linker;
	GetSuperior(SYSTEM_SOURCE,Linker);

	if (Linker.IsValid())
	{
		tstring MsgStr = MsgID2Str(MSG_TASK_RUNTIME_INFO);
		tstring s = Format1024(_T("Send Msg: %s\n"),MsgStr.c_str());
		OutputLog(LOG_TIP,s.c_str());
		Linker().PushMsgToSend(rMsg);
	} 
	else
	{
		tstring MsgStr = MsgID2Str(MSG_TASK_RUNTIME_INFO);
		tstring s = Format1024(_T("Msg(%s) send fail: Linker invalid\n"),MsgStr.c_str());
		OutputLog(LOG_TIP,s.c_str());
	}
}
void CActiveObject::SendRuntimeLogic(int64 ReceiverID,tstring LogicText){
	CMsg rMsg(SYSTEM_SOURCE,ReceiverID,MSG_TASK_RUNTIME_LOGIC,DEFAULT_DIALOG,0);
	ePipeline& rLetter = rMsg.GetLetter(false);
	rLetter.PushString(LogicText);

	CLinker Linker;
	GetSuperior(SYSTEM_SOURCE,Linker);

	if (Linker.IsValid())
	{
		tstring MsgStr = MsgID2Str(MSG_TASK_RUNTIME_LOGIC);
		tstring s = Format1024(_T("Send Msg: %s\n"),MsgStr.c_str());
		OutputLog(LOG_TIP,s.c_str());
		Linker().PushMsgToSend(rMsg);
	} 
	else
	{
		tstring MsgStr = MsgID2Str(MSG_TASK_RUNTIME_LOGIC);
		tstring s = Format1024(_T("Msg(%s) send fail: Linker invalid\n"),MsgStr.c_str());
		OutputLog(LOG_TIP,s.c_str());
	}
}
void  CActiveObject::SendFeedbackError(int64 ReceiverID,ePipeline& ExePipe,tstring Error){
	ExePipe.SetLabel(Error.c_str());
	ExePipe.SetID(RETURN_ERROR);
	SendFeedback(ReceiverID,ExePipe);
}

void  CActiveObject::SendFeedback(int64 ReceiverID,ePipeline& ExePipe){
	CMsg rMsg(SYSTEM_SOURCE,ReceiverID,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,0);
	ePipeline& rLetter = rMsg.GetLetter(false);
	rLetter.PushPipe(ExePipe);

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

bool CActiveObject::WaitingForLogicResult(int64 SpaceEventID,ePipeline& ExePipe){
	while (IsAlive())
	{
		CMsg Msg;
		PopCentralNerveMsg(Msg);

		if (Msg.IsValid() && Msg.GetMsgID() == MSG_TASK_RUNTIME_RESULT)
		{	
			int64 SenderID = Msg.GetSender().PopInt();
			if(SenderID != SpaceEventID){
				continue;
			}
			ePipeline& Letter = Msg.GetLetter(true);
			if(!Letter.HasTypeAB(PARAM_TYPE1(TYPE_PIPELINE))){
				continue;
			}
			
			ePipeline* ExePipe1 = (ePipeline*)Letter.GetData(0);
			ExePipe = *ExePipe1;
			return true;
		}
	}

	return false;
}

void CActiveObject::CentralNerveMsgProc(CMsg& Msg){

		int64 MsgID = Msg.GetMsgID();

		if (m_LogFlag & LOG_MSG_PROC_BEGIN)
		{
			
			int64 EventID = Msg.GetEventID();

			tstring CmdStr = MsgID2Str(MsgID);
			tstring& Tip = Msg.GetMsg().GetLabel();
			tstring s = Format1024(_T("Get Msg:%s EventID:%I64ld %s"),CmdStr.c_str(),EventID,Tip.c_str());	
			OutputLog(LOG_MSG_PROC_BEGIN,s.c_str());	
		}

		switch(MsgID){
		case MSG_OBJECT_RUN: 
			OnObjectRun(Msg);
			break;
		case MSG_OBJECT_GET_DOC:
			OnObjectGetDoc(Msg);
			break;
		case MSG_TASK_RUNTIME_INFO:
			OnTaskRuntimeInfo(Msg);
			break;
		case MSG_OBJECT_CLOSE:
			OnObjectClose(Msg);
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
void CActiveObject::OnConnectTo(CMsg& Msg){
	
	ePipeline& Letter = Msg.GetLetter(true);
	int64 ID = Letter.PopInt();
	tstring& ip = Letter.PopString();
    int32 Port = Letter.PopInt();
	int32 Second = Letter.PopInt();
		
	AnsiString ip1 = WStoUTF8(ip);
	
	OutputLog(LOG_TIP,_T("Begin connect space"));

	tstring error;
    
	if(!Connect(ID,ip1,Port,Second,error,TRUE)){
		m_Alive = false;
	}
}

void CActiveObject::OnObjectClose(CMsg& Msg){

	OutputLog(LOG_TIP,_T("Closed"));
	m_Alive = false;
}

void CActiveObject::OnWhoAreYou(CMsg& Msg){
	
	int64 SourceID = Msg.GetSourceID();

	CLinker Linker;
	GetSuperior(SourceID,Linker);
	
	if(!Linker.IsValid()){
		return;
	};

	CMsg rMsg(SYSTEM_SOURCE,DEFAULT_DIALOG,MSG_I_AM,DEFAULT_DIALOG,0);
	
	ePipeline& rLetter = rMsg.GetLetter(false);
	
	tstring Name = GetName();
	rLetter.PushInt(m_ID);
	rLetter.PushString(Name);
	rLetter.PushString(m_CryptText);


	Linker().PushMsgToSend(rMsg);	
}

void CActiveObject::OnTaskRuntimeInfo(CMsg& Msg){
	ePipeline& Letter = Msg.GetLetter(true);
	if(!Letter.HasTypeAB(PARAM_TYPE1(TYPE_PIPELINE))){
		return;
	}

	ePipeline* Info = (ePipeline*)Letter.GetData(0);
	CPipeView PipeView(Info);

	tstring RuntimeInfo = PipeView.GetString();
	OutputLog(LOG_TIP,RuntimeInfo.c_str());
}

void CActiveObject::OnConnectOK(CMsg& Msg){
	int64 SourceID = Msg.GetSourceID();
	ePipeline& Letter = Msg.GetLetter(true);
	tstring Name;
	Letter.PopString(Name);
    int64 ReceiverID = Msg.GetReceiverID();
	
	CLinker Linker;
	GetSuperior(SourceID,Linker);
	if(Linker.IsValid()){
		Linker().SetRecoType(LINKER_FRIEND);
		if (SourceID == SPACE_SOURCE_READY)
		{
			Linker().SetSourceID(SPACE_SOURCE);	
		}
		OutputLog(LOG_TIP,_T("Connect SpacePort... ok!"));	
	}
}

void CActiveObject::UnitTest(){
	
	//测试

	//tstring FileAddress =_T("D:\\work\\GMIS\\Compile\\bin\\Debug\\Home\\Poker\\Poker.dll");

	tstring FileAddress =_T("D:\\work\\GMIS\\Compile\\bin\\Debug\\createobjectwin.dll");

	//CExObject Object;
	//Object.LoadObject(FileAddress);
	//ePipeline ExePipe;
	//Object.Start(this,0,&ExePipe);

	//ExePipe.Clear();
	
	/*
	ExePipe.PushString(_T("command"));
	ExePipe.PushString(_T("--get \"http://apis.baidu.com/apistore/stockservice/stock?stockid=sz002230&list=1\"  -H \"apikey:fbee01324b74fe3945af710413f77062\""));
	Object.Run(&Executer,0,&ExePipe);

	tstring ss = ExePipe.PopString();
	*/

	//ExePipe.PushString(_T("header"));
	//ExePipe.PushString(_T("Accept:application/json"));
	//Object.Run(this,0,&ExePipe);
/*
	ExePipe.PushString(_T("header"));
	ExePipe.PushString(_T("Content-Type: application/json"));
	Object.Run(&Executer,0,&ExePipe);

	ExePipe.PushString(_T("header"));
	ExePipe.PushString(_T("charsets: utf-8"));
	Object.Run(&Executer,0,&ExePipe);
*/

	//ExePipe.PushString(_T("header"));
	//ExePipe.PushString(_T("apikey:fbee01324b74fe3945af710413f77062"));
	//Object.Run(this,0,&ExePipe);

	//ExePipe.PushString(_T("option"));
	//ExePipe.PushString(_T("CURLOPT_URL"));
	//ExePipe.PushString(_T("http://apis.baidu.com/apistore/stockservice/stock"));
	//Object.Run(this,0,&ExePipe);

	/*
	ExePipe.PushString(_T("option"));
	ExePipe.PushString(_T("CURLOPT_CUSTOMREQUEST"));
	ExePipe.PushString(_T("get"));
	Object.Run(&Executer,0,&ExePipe);
	*/

	/*
	ExePipe.PushString(_T("option"));
	ExePipe.PushString(_T("CURLOPT_HTTPHEADER"));
	Object.Run(&Executer,0,&ExePipe);
	

   */
	//ExePipe.PushString(_T("option"));
	//ExePipe.PushString(_T("CURLOPT_HTTPGET"));
	//ExePipe.PushString(_T("stockid=sz002230&list=1"));
	//Object.Run(this,0,&ExePipe);



	//ExePipe.PushString(_T("run"));
 //   Object.Run(this,0,&ExePipe);

	//tstring ss = ExePipe.PopString();
	//OutputLog(LOG_TIP,ss.c_str());

	//Ob->Do(&ExePipe);
	//Ob->Dead();
	//Ob->WaitForStopRun(50);
	//delete Ob;

}