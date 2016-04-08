#pragma warning (disable:4786)

#include "Brain.h"
#include "LogicDialog.h"
#include "GUIMsgDefine.h"



void CBrain::NotifySysState(int64 NotifyType,int64 NotifyID,ePipeline* Data){
	ePipeline Receiver;
	Receiver.PushInt(SYSTEM_SOURCE);
	Receiver.PushInt(DEFAULT_DIALOG);
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
    //return;
	switch (NotifyType)
	{
	case NOTIFY_MODEL_SCENE:
		{
			
			switch(NotifyID){
			case NTID_NERVE_MSG_NUM:
				{
					ePipeline Cmd(GUI_PFM_MSG_UPDATE);
					Cmd.PushInt(CENTRALNERVE_MSG_NUM);
					Cmd.PushPipe(*Data);
					GuiMsg.GetLetter().PushPipe(Cmd);

					GetBrainData()->SendMsgToGUI(this,SYSTEM_SOURCE,GuiMsg);
				}
				break;	
	
			case NTID_NERVE_THREAD_JOIN:
				{
					int64 n = Data->PopInt();

					ePipeline Cmd(GUI_PFM_THREAD_UPDATE);
					Cmd.PushInt(THREAD_CENTRAL_NERVE);
					Cmd.PushInt(n);
					GuiMsg.GetLetter().PushPipe(Cmd);

					GetBrainData()->SendMsgToGUI(this,SYSTEM_SOURCE,GuiMsg);
				}
				break;
			case NTID_NERVE_THREAD_CLOSED:
				{
					int64 nThreadNum = Data->PopInt();
					int64 nClosedThreadID = Data->PopInt();
					CLockedModelData*  LockedData = GetModelData();
					LockedData->DeleteThreadWorker(this,nClosedThreadID,MODEL_CENTRAL_NEVER_WORK_TYPE);

					ePipeline Cmd(GUI_PFM_THREAD_UPDATE);
					Cmd.PushInt(THREAD_CENTRAL_NERVE);
					Cmd.PushInt(nThreadNum);
					GuiMsg.GetLetter().PushPipe(Cmd);

					GetBrainData()->SendMsgToGUI(this,SYSTEM_SOURCE,GuiMsg);
				}
				break;
			case NTID_NERVE_THREAD_LIMIT:
				{


				}
				break;
			case NTID_NERVE_THREAD_FAIL:
				{

				}
				break;
			case NTID_IO_WORKER_CLOSED:
				{
					int64 ID = Data->PopInt();
					CLockedModelData* ModelData = GetModelData();
					ModelData->DeleteThreadWorker(this,ID,MODEL_IO_WORK_TYPE);
				}
				break;
			case NTID_CONNECT_FAIL:
				{

				}
				break;
			case NTID_CONNECT_OK:
				{

				}
				break;
			case NTID_LISTEN_FAIL:
				{
					tstring s=Data->PopString();
					OutSysInfo(s.c_str());
				}
				break;
			}
			
		}
		break;
	case NOTIFY_SYSTEM_SCENE:
		{
			switch(NotifyID){
			case NTID_NERVE_MSG_NUM:
				{
					ePipeline Cmd(GUI_PFM_MSG_UPDATE);
					Cmd.PushInt(NERVE_MSG_NUM);
					Cmd.PushPipe(*Data);
					GuiMsg.GetLetter().PushPipe(Cmd);

					GetBrainData()->SendMsgToGUI(this,SYSTEM_SOURCE,GuiMsg);
				}
				break;
			case NTID_NERVE_THREAD_JOIN:
				{
					int64 n = Data->PopInt();
					ePipeline Cmd(GUI_PFM_THREAD_UPDATE);
					Cmd.PushInt(THREAD_NERVE);
					Cmd.PushInt(n);
					GuiMsg.GetLetter().PushPipe(Cmd);

					GetBrainData()->SendMsgToGUI(this,SYSTEM_SOURCE,GuiMsg);		
				}
				break;
			case NTID_NERVE_THREAD_CLOSED:
				{
					int64 nThreadNum = Data->PopInt();
					int64 nClosedThreadID = Data->PopInt();

					CLockedSystemData*  LockedData = GetSystemData();
					LockedData->DeleteThreadWorker(this,nClosedThreadID,SYSTEM_NEVER_WORK_TYPE);

					ePipeline Cmd(GUI_PFM_THREAD_UPDATE);
					Cmd.PushInt(THREAD_NERVE);
					Cmd.PushInt(nThreadNum);
					GuiMsg.GetLetter().PushPipe(Cmd);

					GetBrainData()->SendMsgToGUI(this,SYSTEM_SOURCE,GuiMsg);
				}
				break;
			case NTID_NERVE_THREAD_LIMIT:
				{


				}
				break;
			case NTID_NERVE_THREAD_FAIL:
				{

				}
				break;
			case NTID_IO_WORKER_CLOSED:
				{
					int64 ID = Data->PopInt();
					CLockedSystemData* SystemData = GetSystemData();
					SystemData->DeleteThreadWorker(this,ID,SYSTEM_IO_WORK_TYPE);
				}
				break;
			case NOTIFY_EXCEPTION_OCCURRED:
				{
					tstring ErrorInfo = Data->PopString();
					OutSysInfo(ErrorInfo.c_str());
			    }
				break;
			default:
				{

				}
			}
		}
		break;
	case NOTIFY_BRAIN_SCENE:
		{
			if(NotifyID == NTID_IO_WORKER_CLOSED)
			{
				int64 ID = Data->PopInt();
				m_BrainData.DeleteThreadWorker(this,ID,BRAIN_WEBSOCKET_IO_WORK_TYPE);
			}
		}
		break;
	case NOTIFY_ILLEGAL_MSG:
		{
			CMsg Msg(Data);
			int64 MsgID = Msg.GetMsgID();
			tstring MsgStr = MsgID2Str(MsgID);
			tstring s = Format1024(_T("Receive illegal msg: %I64ld(%s)"),MsgID,MsgStr.c_str());
			Msg.Release(); //避免delete Data;
			OutSysInfo(s.c_str());	
		}
		break;
	case NOTIFY_EXCEPTION_OCCURRED:
		{
			tstring ErrorInfo = Data->PopString();
			OutputLog(LOG_TIP,ErrorInfo.c_str());
		}
		break;
	}
	
};

void CBrain::NotifyLinkerState(int64 SourceID,int64 NotifyID,STATE_OUTPUT_LEVEL Flag,ePipeline& Info){
	
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
		}//注意：这里不要break
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
					OutSysInfo(s.c_str());
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
							OutSysInfo(s.c_str());
						} 
						else
						{	
							tstring s = Format1024(_T("LINKER_MSG_RECEIVED: Remote received msg:%s ok"),PendingMsgName.c_str());
							OutputLog(LOG_MSG_IO_REMOTE_RECEIVED,s.c_str());
							OutSysInfo(s.c_str());
						}
					} 
					else
					{	
						tstring MsgName = MsgID2Str(MsgID);
						tstring s = Format1024(_T("LINKER_MSG_RECEIVED:Msg received: %s ok"),MsgName.c_str());
						OutputLog(LOG_MSG_I0_RECEIVED,s.c_str());
						OutSysInfo(s.c_str());
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
					OutSysInfo(s.c_str());
				}
				return;
			case LINKER_END_ERROR_STATE:
				{
					tstring s = Format1024(_T("LINKER_END_ERROR: SourceID=%I64ld"),SourceID);
					OutputLog(LOG_TIP,s.c_str());
					OutSysInfo(s.c_str());
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
					OutSysInfo(s.c_str());
				}
				return;
			case LINKER_ILLEGAL_MSG:
				{
					ePipeline* CurRevMsg = (ePipeline*)Info.GetData(0);

					AnsiString text;
					CurRevMsg->ToString(text);

					tstring s = Format1024(_T("LINKER_ILLEGAL_MSG: SourceID=%I64ld Msg:%s "),SourceID,UTF8toWS(text).c_str());

					OutputLog(LOG_WARNING,s.c_str());
					OutSysInfo(s.c_str());	
				}
				return;
			case LINKER_EXCEPTION_ERROR:
				{
					int32 RecoType = Info.PopInt();
					int32 LinkerType = Info.PopInt();

					tstring Error = Info.PopString();
					tstring s = Format1024(_T("LINKER_EXCEPTION_ERROR: %s SourceID=%I64ld was closed "),Error.c_str(),SourceID);

					OutputLog(LOG_WARNING,s.c_str());
					OutSysInfo(s.c_str());	

					if (LinkerType  == CLIENT_LINKER)
					{
						bool ret = m_ClientLinkerList.DeleteLinker(SourceID);
						if(!ret)return; //链接并不存在

					}else if(LinkerType == SERVER_LINKER){
						bool ret = m_SuperiorList.DeleteLinker(SourceID);
						if(!ret)return; //链接并不存在
					}else if (LinkerType == WEBSOCKET_LINKER){
						bool ret = m_WebsocketClientList.DeleteLinker(SourceID);
						if(!ret)return; //链接并不存在
					}
					


					//得到链接失效的 Dialog
					CLogicDialog* InvalidDialog = GetBrainData()->GetDialog(SourceID,DEFAULT_DIALOG);

					if (InvalidDialog==NULL)
					{
						return;
					}

					if (SourceID==SPACE_SOURCE)
					{
						GetBrainData()->DeleteDialogOfSource(SourceID);

						tstring ip ;

						ePipeline Receiver;
						Receiver.PushInt(InvalidDialog->m_SourceID);
						Receiver.PushInt(InvalidDialog->m_DialogID);

						CMsg GuiMsg2(Receiver,MSG_BRAIN_TO_GUI,0);		
						ePipeline Cmd(GUI_CONNECT_STATE);
						Cmd.PushInt(CON_END);
						Cmd.PushString(ip);
						Cmd.PushInt(FALSE);	
						GuiMsg2.GetLetter().PushPipe(Cmd);

						GetBrainData()->SendMsgToGUI(this,GuiMsg2,-1);
						return;
					}

					GetBrainData()->DeleteDialogOfSource(SourceID);
			
					ePipeline Receiver;
					Receiver.PushInt(SYSTEM_SOURCE);
					Receiver.PushInt(DEFAULT_DIALOG);
					CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);

					ePipeline Cmd1(GUI_LINKVIEW_OUTPUT);
					Cmd1.PushInt(DEL_ITEM);
					Cmd1.PushInt(InvalidDialog->m_SourceID);
					Cmd1.PushInt(InvalidDialog->m_DialogID);
					Cmd1.PushString(InvalidDialog->m_DialogName);

					GuiMsg.GetLetter().PushPipe(Cmd1);
					//通知其它GUI删除此对话
					GetBrainData()->SendMsgToGUI(this,GuiMsg,SourceID);
					
				}
				return;
			case LINKER_IO_ERROR:
				{
					//通常是远端关闭

					int32 RecoType = Info.PopInt();
					int32 LinkerType = Info.PopInt();

					tstring s = Format1024(_T("LINKER_IO_ERROR: SourceID=%I64ld may be closed by remote"),SourceID);

					OutputLog(LOG_WARNING,s.c_str());
					OutSysInfo(s.c_str());	

					if (LinkerType == CLIENT_LINKER)
					{
						bool ret = m_ClientLinkerList.DeleteLinker(SourceID);
						if(!ret)return; //链接并不存在

					}else if(LinkerType == SERVER_LINKER){
						bool ret = m_SuperiorList.DeleteLinker(SourceID);
						if(!ret)return; //链接并不存在
					}else if (LinkerType == WEBSOCKET_LINKER){
						bool ret = m_WebsocketClientList.DeleteLinker(SourceID);
						if(!ret)return; //链接并不存在
					}


					//得到链接失效的 Dialog
					CLogicDialog* InvalidDialog = GetBrainData()->GetDialog(SourceID,DEFAULT_DIALOG);

					if (InvalidDialog==NULL)
					{
						return;
					}

					if (SourceID==SPACE_SOURCE)
					{
						GetBrainData()->DeleteDialogOfSource(SourceID);

						tstring ip ;

						ePipeline Receiver;
						Receiver.PushInt(InvalidDialog->m_SourceID);
						Receiver.PushInt(InvalidDialog->m_DialogID);

						CMsg GuiMsg2(Receiver,MSG_BRAIN_TO_GUI,0);		
						ePipeline Cmd(GUI_CONNECT_STATE);
						Cmd.PushInt(CON_END);
						Cmd.PushString(ip);
						Cmd.PushInt(FALSE);	
						GuiMsg2.GetLetter().PushPipe(Cmd);

						GetBrainData()->SendMsgToGUI(this,GuiMsg2,-1);
						return;
					}

					CNotifyDialogState nf(NOTIFY_DIALOG_LIST);
					nf.PushInt(DL_DEL_DIALOG);
					nf.Notify(InvalidDialog);

					GetBrainData()->DeleteDialogOfSource(SourceID);

				}
				return;
			case LINKER_CONNECT_ERROR:
				{
					tstring Error = Info.PopString();
					tstring s = Format1024(_T("LINKER_CONNECT_ERROR: SourceID=%I64ld %s "),SourceID,Error.c_str());

					OutputLog(LOG_WARNING,s.c_str());
					OutSysInfo(s.c_str());	

					GetSuperiorLinkerList()->DeleteLinker(SourceID);
				}
				return;
			case LINKER_CREATE_ERROR:
				{
					tstring Error = Info.PopString();
					tstring s = Format1024(_T("LINKER_CREATE_ERROR: %s "),Error.c_str());

					OutputLog(LOG_WARNING,s.c_str());
					OutSysInfo(s.c_str());	
				}
				return;
			} 
		}
	default:
		break;
	}

}




void CBrain::NoitfyDialogState(CLogicDialog* Dialog, ePipeline* NotifyInfo){
	//_CLOCK(&Dialog->m_DialogMutex,this); 避免大范围锁定，只在具体需要时加锁

	int64 NotifyID = NotifyInfo->GetID();
	
	switch(NotifyID)
	{
	case NOTIFY_DIALOG_ERROR:
		OnNotifyDialogError(Dialog,NotifyInfo);
		break;
	case NOTIFY_DIALOG_STATE:
		OnNotifyDialogState(Dialog,NotifyInfo);
		break;
	case NOTIFY_DIALOG_OUTPUT: 
		OnNotifyDialogOutput(Dialog,NotifyInfo);
		break;
	case NOTIFY_LOGIC_VIEW:
		OnNotifyLogicView(Dialog,NotifyInfo);
		break;
	case NOTIFY_DIALOG_LIST:
		OnNotifyDialogList(Dialog,NotifyInfo);
		break;
	case NOTIFY_PROGRESS_OUTPUT:
		OnNotifyProgressOutput(Dialog,NotifyInfo);
		break;
	case NOTIFY_DEBUG_VIEW:
		OnNotifyDebugView(Dialog,NotifyInfo);
		break;
	case NOTIFY_BRAIN_INIT:
		OnNotifyBrainInit(Dialog,NotifyInfo);
		break;
	case NOTIFY_OBJECT_VIEW:
		OnNotifyObjectList(Dialog,NotifyInfo);
		break;
	case NOTIFY_FIND_VIEW:
        OnNotifyFindInfo(Dialog,NotifyInfo);
		break;
	case NOTIFY_MEMORY_VIEW:
		OnNotifyMemoryList(Dialog,NotifyInfo);
		break;
	default:
		tstring s = Format1024(_T("The notify(%I64ld) has not been processed."),NotifyID);
		OutSysInfo(s.c_str());
		assert(0);
	};
}

void CBrain::OnNotifyDialogError(CLogicDialog* Dialog, ePipeline* NotifyInfo){


	int64 NotifyID = NotifyInfo->GetID();
	int64 ErrorType = NotifyInfo->PopInt();

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);

	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);

	switch(ErrorType){
	case THINK_ERROR:
		{
			tstring error = NotifyInfo->PopString();

			Dialog->RuntimeOutput(0,error);

			
			ePipeline Cmd1(GUI_CONTINUE_EDIT);
			Cmd1.PushInt(FALSE);
			GuiMsg.GetLetter().PushPipe(Cmd1);

			Dialog->m_bEditValid = true;
		}
		break;
	case COMPILE_ERROR:
		{
			tstring error = NotifyInfo->PopString();
			tstring EditText = NotifyInfo->PopString();

			Dialog->RuntimeOutput(0,error);
			
			ePipeline Cmd1(GUI_CONTINUE_EDIT);
			Cmd1.PushInt(TRUE);
			Cmd1.PushString(EditText);
			GuiMsg.GetLetter().PushPipe(Cmd1);
			
			Dialog->m_bEditValid = true;
		}
		break;
	default:
		return;
	}

	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);
}

void CBrain::OnNotifyDialogState(CLogicDialog* Dialog, ePipeline* NotifyInfo){

    int64 WorkMode = NotifyInfo->PopInt();
	int64 State = NotifyInfo->PopInt();

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);

	switch(State){
	case TASK_IDLE:
		{

			Dialog->m_bEditValid = true;
            Dialog->m_StatusText = _T("");

			ePipeline Cmd(GUI_SET_WORKMODE);
			Cmd.PushInt(WorkMode);
			GuiMsg.GetLetter().PushPipe(Cmd);

			ePipeline Cmd0(GUI_CONTINUE_EDIT);
            
			if (NotifyInfo->Size()==0)
			{
				Cmd0.PushInt(FALSE); //不设置EDIT TEXT
			}else{
				Cmd0.PushInt(TRUE);
				tstring EditText = NotifyInfo->PopString();
                Cmd0.PushString(EditText);
			}            
			GuiMsg.GetLetter().PushPipe(Cmd0);
			
			ePipeline Cmd1(GUI_STATUS_SET_TEXT);
			Cmd1.PushString(_T(""));
			GuiMsg.GetLetter().PushPipe(Cmd1);

			ePipeline Cmd2(GUI_ONE_LIGHT_FLASH);
			Cmd2.PushInt(FALSE);
			GuiMsg.GetLetter().PushPipe(Cmd2);		

			ePipeline Cmd3(GUI_STATUS_PROGRESS);
			Cmd3.PushInt(0);
			GuiMsg.GetLetter().PushPipe(Cmd3);

			ePipeline Cmd4(GUI_VIEW_LAYOUT);
			Cmd4.PushInt(TASK_IDLE);			
			GuiMsg.GetLetter().PushPipe(Cmd4);	
		}
		break;
	case TASK_THINK:
		{
			ePipeline Cmd1(GUI_ONE_LIGHT_FLASH);
			Cmd1.PushInt(TRUE);
			GuiMsg.GetLetter().PushPipe(Cmd1);
		}
		break;
	case TASK_COMPILE:
		{
			Dialog->m_bEditValid = false;
			Dialog->m_EditText = _T("");
		
			ePipeline Cmd1(GUI_SET_EIDT);
			Cmd1.PushString(Dialog->m_EditText.c_str());
			Cmd1.PushInt(FALSE);  //EDIT readonly
			GuiMsg.GetLetter().PushPipe(Cmd1);

			ePipeline Cmd2(GUI_STATUS_SET_TEXT);
			Cmd2.PushString(_T("Compile..."));
			GuiMsg.GetLetter().PushPipe(Cmd2);

		}
		break;
	case TASK_EXE:
		{
			Dialog->m_EditText = _T("");
			Dialog->m_bEditValid = false;


			ePipeline Cmd(GUI_SET_WORKMODE);
			Cmd.PushInt(WorkMode);
			GuiMsg.GetLetter().PushPipe(Cmd);
/*
			ePipeline Cmd1(GUI_SET_EIDT);
			Cmd1.PushString(Dialog->m_EditText.c_str());
			Cmd1.PushInt(Dialog->m_bEditValid);  //EDIT readonly
			GuiMsg.GetLetter().PushPipe(Cmd1);
*/
			ePipeline Cmd2(GUI_CLEAR_OUTPUT);
			Cmd2.PushInt(CLEAR_RUNTIME);
            GuiMsg.GetLetter().PushPipe(Cmd2);

			ePipeline Cmd3(GUI_VIEW_LAYOUT);
			Cmd3.PushInt(TASK_RUN);			
			GuiMsg.GetLetter().PushPipe(Cmd3);	
		}
	case TASK_RUN:  
		{
			if (WorkMode == WORK_LEARN)
			{
				Dialog->m_bEditValid = true;

			}else{
				Dialog->m_bEditValid = false;
			}


			ePipeline Cmd1(GUI_SET_EIDT);
			Cmd1.PushString(Dialog->m_EditText.c_str());
			Cmd1.PushInt(Dialog->m_bEditValid);  //EDIT readonly
			GuiMsg.GetLetter().PushPipe(Cmd1);

			ePipeline Cmd2(GUI_VIEW_LAYOUT);
			Cmd2.PushInt(TASK_RUN);			
			GuiMsg.GetLetter().PushPipe(Cmd2);	

			/*
			ePipeline Cmd3(GUI_TASK_TOOL_BAR);
			Cmd3.PushInt(FALSE); //RUN BNT disable
			Cmd3.PushInt(TRUE);  //PAUSE BNT Enable
			Cmd3.PushInt(TRUE);  //STOP BNT Enable
			GuiMsg.GetLetter().PushPipe(Cmd3);
			*/
		}
		break;
	case TASK_STOP:
		{

			Dialog->m_bEditValid = true;
			Dialog->m_EditText = _T("");
            Dialog->m_StatusText = _T("");
			
			
			ePipeline Cmd(GUI_SET_WORKMODE);
			Cmd.PushInt(WorkMode);
			GuiMsg.GetLetter().PushPipe(Cmd);

			ePipeline Cmd0(GUI_CONTINUE_EDIT);
			Cmd0.PushInt(TRUE);  //EDIT ENABLE
			Cmd0.PushString(_T(""));
			GuiMsg.GetLetter().PushPipe(Cmd0);
   		
			ePipeline Cmd1(GUI_STATUS_SET_TEXT);
			
			ePipeline Info;
			Dialog->m_Brain->GetBrainData()->GetBrainStateInfo(Info);
			
			int64 EventNum = Info.PopInt();
			int64 DialogNum = Info.PopInt();
			int64 DialogPoolSize = Info.PopInt();
			int64 ThreadNum = Info.PopInt();
			int64 ThreadPoolSize = Info.PopInt();
			int64 TaskNum   = Info.PopInt();
			int64 TaskPoolSize = Info.PopInt();
			int64 GuiNum       = Info.PopInt();

			tstring Tip = Format1024(_T("EN:%I64ld DN:%I64ld DP:%I64ld THN:%I64ld THP:%I64ld TKN:%I64ld TKP:%I64ld"),
				EventNum,DialogNum,DialogPoolSize,ThreadNum,ThreadPoolSize,TaskNum,TaskPoolSize);
			Cmd1.PushString(Tip);
			GuiMsg.GetLetter().PushPipe(Cmd1);

					
			ePipeline Cmd3(GUI_VIEW_LAYOUT);
			Cmd3.PushInt(TASK_STOP);			
			GuiMsg.GetLetter().PushPipe(Cmd3);	

			ePipeline Cmd4(GUI_STATUS_PROGRESS);
			Cmd4.PushInt(0);
			GuiMsg.GetLetter().PushPipe(Cmd4);

		}
		break;
	case TASK_PAUSE:
		{
			
			int64 TaskTimeStamp   = Dialog->m_LogicItemTree.GetID();
			int64 PauseID  = Dialog->GetFocusPauseItemID();
			
			Dialog->m_bEditValid = true;
			Dialog->m_EditText  = _T("");
			
			ePipeline Cmd(GUI_SET_WORKMODE);
			Cmd.PushInt(WorkMode);
			GuiMsg.GetLetter().PushPipe(Cmd);
			
			ePipeline Cmd0(GUI_SET_EIDT);
			Cmd0.PushString(_T(""));
			Cmd0.PushInt(TRUE);  //EDIT ENABLE
			GuiMsg.GetLetter().PushPipe(Cmd0);
			
			Dialog->m_StatusText = _T("Pause...");
			ePipeline Cmd1(GUI_STATUS_SET_TEXT);
			Cmd1.PushString(Dialog->m_StatusText);
			GuiMsg.GetLetter().PushPipe(Cmd1);
			
			
			ePipeline Cmd2(GUI_TASK_TOOL_BAR);
			Cmd2.PushInt(TRUE); //RUN BNT disable
			Cmd2.PushInt(FALSE);  //PAUSE BNT Enable
			Cmd2.PushInt(TRUE);  //STOP BNT Enable
			GuiMsg.GetLetter().PushPipe(Cmd2);
			
			if (PauseID)
			{
				ePipeline Cmd3(GUI_DEBUGVIEW_OUTPUT);
				Cmd3.PushInt(DEBUG_MARK_PAUSE);
				Cmd3.PushInt(TaskTimeStamp);
				Cmd3.PushInt(PauseID);
				GuiMsg.GetLetter().PushPipe(Cmd3);
			}
			
			ePipeline Cmd4(GUI_VIEW_LAYOUT);
			Cmd4.PushInt(TASK_PAUSE);			
			GuiMsg.GetLetter().PushPipe(Cmd4);	
			
		}
		break;
	case TASK_WAIT:
		{

			Dialog->m_StatusText = _T("Waiting for feedback or input..");
			ePipeline Cmd0(GUI_STATUS_SET_TEXT);
			Cmd0.PushString(Dialog->m_StatusText);
			GuiMsg.GetLetter().PushPipe(Cmd0);

			ePipeline Cmd1(GUI_SET_WORKMODE);
			Cmd1.PushInt(WorkMode);
			GuiMsg.GetLetter().PushPipe(Cmd1);
			
			/*
			ePipeline Cmd2(GUI_SET_EIDT);
			Cmd2.PushString(Dialog->m_EditText);
			Cmd2.PushInt(Dialog->m_bEditValid);  //EDIT ENABLE
			GuiMsg.GetLetter().PushPipe(Cmd2);
			

			ePipeline Cmd3(GUI_TASK_TOOL_BAR);
			Cmd3.PushInt(FALSE); //RUN BNT disable
			Cmd3.PushInt(FALSE);  //PAUSE BNT Enable
			Cmd3.PushInt(TRUE);  //STOP BNT Enable
			GuiMsg.GetLetter().PushPipe(Cmd3);
			
			*/
		}
		break;
	case TASK_DELELTE:
		{
			ePipeline Cmd1(GUI_SET_WORKMODE);
			Cmd1.PushInt(WorkMode);
			GuiMsg.GetLetter().PushPipe(Cmd1);
			
			ePipeline Cmd2(GUI_SET_EIDT);
			Cmd2.PushString(_T(""));
			Cmd2.PushInt(TRUE);  //EDIT ENABLE
			GuiMsg.GetLetter().PushPipe(Cmd2);
			
			ePipeline Cmd3(GUI_TASK_TOOL_BAR);
			Cmd3.PushInt(FALSE); //RUN BNT disable
			Cmd3.PushInt(FALSE);  //PAUSE BNT Enable
			Cmd3.PushInt(FALSE);  //STOP BNT Enable
			GuiMsg.GetLetter().PushPipe(Cmd3);
		}
		break;
	default:
		break;
	}

	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);
}

void CBrain::OnNotifyDialogOutput(CLogicDialog* Dialog, ePipeline* NotifyInfo){	

	int64 Type = NotifyInfo->PopInt();
	
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);

	switch(Type){
	case DIALOG_INFO:
		{
			eElectron e;
			NotifyInfo->Pop(&e);
			ePipeline* Item = (ePipeline*)e.Value();

			_CLOCK2(&Dialog->m_DialogMutex,Dialog);

			Dialog->m_DialogHistory.Push_Directly(e.Release());

			ePipeline Cmd0(GUI_DIALOG_OUTPUT);
			Cmd0.Push_Directly(Item->Clone());	
			GuiMsg.GetLetter().PushPipe(Cmd0);
		}
		break;
	case RUNTIME_INFO:
		{
			eElectron e;
			NotifyInfo->Pop(&e);
			ePipeline* Item = (ePipeline*)e.Value();
		
			_CLOCK2(&Dialog->m_DialogMutex,Dialog);
			Dialog->m_RuntimeOutput.Push_Directly(e.Release());

			ePipeline Cmd0(GUI_RUNTIME_OUTPUT);
			Cmd0.Push_Directly(Item->Clone());
			GuiMsg.GetLetter().PushPipe(Cmd0);
		}
		break;
	case STATUS_INFO:
		{
			tstring s = NotifyInfo->PopString();
			ePipeline Cmd1(GUI_STATUS_SET_TEXT);
			Cmd1.PushString(s);
			GuiMsg.GetLetter().PushPipe(Cmd1);
		}
		break;
	case FORECAST_INFO:
		{
			tstring s = NotifyInfo->PopString();
			ePipeline Cmd1(GUI_FORECAST_OUTPUT);
			Cmd1.PushString(s);
			GuiMsg.GetLetter().PushPipe(Cmd1);
		}
		break;
	case INIT_FOCUS:
		{
			ePipeline Cmd1(GUI_SET_CUR_DIALOG);
			Cmd1<<*NotifyInfo;
            GuiMsg.GetLetter().PushPipe(Cmd1);
		}
		break;
	case DIALOG_INFO_MORE:
		{
			eElectron e;
			NotifyInfo->Pop(&e);
			ePipeline* Item = (ePipeline*)e.Value();
			
			ePipeline Cmd0(GUI_SET_DIALOG_MORE_ITEE);
			Cmd0.Push_Directly(e.Release());	
			GuiMsg.GetLetter().PushPipe(Cmd0);
		}
		break;
	case RUNTIME_INFO_MORE:
		{

		}
		break;
	}

	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);

}

void CBrain::OnNotifyLogicView(CLogicDialog* Dialog, ePipeline* NotifyInfo){

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);

	ePipeline Cmd1(GUI_LOGIC_OUTPUT);
	Cmd1<<*NotifyInfo;
	GuiMsg.GetLetter().PushPipe(Cmd1);
	
	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);
};

void CBrain::OnNotifyObjectList(CLogicDialog* Dialog, ePipeline* NotifyInfo){

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd1(GUI_OBJECT_OUTPUT);
	Cmd1<<*NotifyInfo;
	GuiMsg.GetLetter().PushPipe(Cmd1);
	
	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);
}

void CBrain::OnNotifyDialogList(CLogicDialog* Dialog, ePipeline* NotifyInfo){

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	int64 Act = NotifyInfo->PopInt();
	switch(Act){

	case DL_ADD_DIALOG:
		{
			int64 bFocusDialog = NotifyInfo->PopInt();
			
			ePipeline Cmd1(GUI_LINKVIEW_OUTPUT);
			Cmd1.PushInt(ADD_ITEM);
			Cmd1.PushInt(Dialog->m_SourceID);
			Cmd1.PushInt(Dialog->m_DialogID);
			Cmd1.PushInt(Dialog->m_ParentDialogID);
			Cmd1.PushString(Dialog->m_DialogName);
			GuiMsg.GetLetter().PushPipe(Cmd1);
			
			if (bFocusDialog)
            {
				ePipeline Cmd(GUI_SET_CUR_DIALOG);
				Cmd.PushInt(Dialog->m_SourceID);
				Cmd.PushInt(Dialog->m_DialogID);

				Dialog->m_Brain->GetBrainData()->GetFocusDialogData(Dialog->m_SourceID,Dialog->m_DialogID,Cmd,false);
				GuiMsg.GetLetter().PushPipe(Cmd);	
            };
		}
		break;
	case DL_DEL_DIALOG:
		{
			
			ePipeline Cmd1(GUI_LINKVIEW_OUTPUT);
			Cmd1.PushInt(DEL_ITEM);
			Cmd1.PushInt(Dialog->m_SourceID);
			Cmd1.PushInt(Dialog->m_DialogID);
			Cmd1.PushString(Dialog->m_DialogName);

			GuiMsg.GetLetter().PushPipe(Cmd1);

			if (NotifyInfo->Size())
			{
				tstring Reason = NotifyInfo->PopString();
				OutSysInfo(Reason.c_str());
			}
		}
		break;

	case DL_LOGIN_ONE:
		{
			
			int64 SourceID = NotifyInfo->PopInt();
			CLogicDialog* TheDialog = GetBrainData()->GetDialog(SourceID,DEFAULT_DIALOG);
			assert(TheDialog);
			if(!TheDialog)return;

			//初始化当前对话列表，
			ePipeline DialogListInfo;
			GetBrainData()->GetAllDialogListInfo(DialogListInfo);

			ePipeline Cmd1(GUI_LINKVIEW_OUTPUT);
			Cmd1.PushInt(INIT_LIST);
			Cmd1.PushPipe(DialogListInfo);
			GuiMsg.GetLetter().PushPipe(Cmd1);

			//设置此登录对话为当前焦点对话
			ePipeline Pipe;  //历史对话记录
			GetBrainData()->GetFocusDialogData(TheDialog->m_SourceID,TheDialog->m_DialogID,Pipe,false);
			ePipeline Cmd2(GUI_SET_CUR_DIALOG);
			Cmd2.PushInt(TheDialog->m_SourceID);
			Cmd2.PushInt(TheDialog->m_DialogID);
			Cmd2<<Pipe;
			GuiMsg.GetLetter().PushPipe(Cmd2);

			GetBrainData()->SendMsgToGUI(this,SourceID,GuiMsg);

			//通知其它GUI用户，增加一个对话条目。
			ePipeline Receiver;
			Receiver.PushInt(SYSTEM_SOURCE);
			Receiver.PushInt(DEFAULT_DIALOG);
			CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);

			ePipeline Cmd(GUI_LINKVIEW_OUTPUT);
			Cmd.PushInt(ADD_ITEM);
			Cmd.PushInt(TheDialog->m_SourceID);
			Cmd.PushInt(TheDialog->m_DialogID);
			Cmd.PushInt(TheDialog->m_ParentDialogID);
			Cmd.PushString(TheDialog->m_DialogName);
			GuiMsg.GetLetter().PushPipe(Cmd);
			GetBrainData()->SendMsgToGUI(this,GuiMsg,SourceID);
			return;
		}
	default:
		break;
	}

	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);
}

void CBrain::OnNotifyProgressOutput(CLogicDialog* Dialog, ePipeline* NotifyInfo)
{
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	int64 Act = NotifyInfo->PopInt();
	switch(Act){
	case COMMON_PROGRESS:
		{
			tstring s = NotifyInfo->PopString();
			ePipeline Cmd(GUI_STATUS_SET_TEXT);
			Cmd.PushString(s);
			GuiMsg.GetLetter().PushPipe(Cmd);

			int64 Per = NotifyInfo->PopInt();
			ePipeline Cmd1(GUI_STATUS_PROGRESS);
			Cmd1.PushInt(Per);
			GuiMsg.GetLetter().PushPipe(Cmd1);
		}
		break;
	case COMPILE_PROGRESS:
	case THINK_PROGRESS:
		{
			int64 Per = NotifyInfo->PopInt();
			ePipeline Cmd1(GUI_STATUS_PROGRESS);
			Cmd1.PushInt(Per);
			GuiMsg.GetLetter().PushPipe(Cmd1);
		}
		break;
	default:
		break;
	}
	
	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);
	
}
void CBrain::OnNotifyDebugView(CLogicDialog* Dialog, ePipeline* NotifyInfo){


	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd1(GUI_DEBUGVIEW_OUTPUT);
	Cmd1<< *NotifyInfo;
	
	GuiMsg.GetLetter().PushPipe(Cmd1);

	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);
}

void CBrain::OnNotifyBrainInit(CLogicDialog* Dialog, ePipeline* NotifyInfo){
	int64 Type = NotifyInfo->PopInt();

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	

	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);

	if (Type == INIT_PROGRESS)
	{
		tstring tip = NotifyInfo->PopString();
		ePipeline Cmd(GUI_STATUS_SET_TEXT);
		Cmd.PushString(tip);
		GuiMsg.GetLetter().PushPipe(Cmd);

		int64 Per = NotifyInfo->PopInt();
		ePipeline Cmd1(GUI_STATUS_PROGRESS);
		Cmd1.PushInt(Per);
		GuiMsg.GetLetter().PushPipe(Cmd1);

	}else if (Type == BEGIN_INIT)
	{
       
		ePipeline Cmd3(GUI_TASK_TOOL_BAR);
		Cmd3.PushInt(FALSE); //RUN BNT disable
		Cmd3.PushInt(TRUE);  //PAUSE BNT Enable
		Cmd3.PushInt(TRUE);  //STOP BNT Enable
		GuiMsg.GetLetter().PushPipe(Cmd3);
				
	}else if (Type == END_INIT)
	{
		tstring tip;
		ePipeline Cmd(GUI_STATUS_SET_TEXT);
		Cmd.PushString(tip);
		GuiMsg.GetLetter().PushPipe(Cmd);
		
		int64 Per = 0;
		ePipeline Cmd1(GUI_STATUS_PROGRESS);
		Cmd1.PushInt(Per);
		GuiMsg.GetLetter().PushPipe(Cmd1);

		ePipeline Cmd3(GUI_TASK_TOOL_BAR);
		Cmd3.PushInt(FALSE); //RUN BNT disable
		Cmd3.PushInt(FALSE);  //PAUSE BNT Enable
		Cmd3.PushInt(FALSE);  //STOP BNT Enable
		GuiMsg.GetLetter().PushPipe(Cmd3);

	}else{
		return ;
	}

	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);
}

void CBrain::OnNotifyFindInfo(CLogicDialog* Dialog, ePipeline* NotifyInfo){

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
		
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd1(GUI_FIND_OUTPUT);
	Cmd1<< *NotifyInfo;
	GuiMsg.GetLetter().PushPipe(Cmd1);

	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);
}


void CBrain::OnNotifyMemoryList(CLogicDialog* Dialog, ePipeline* NotifyInfo){

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd1(GUI_MEMORY_OUTPUT);
	Cmd1<< *NotifyInfo;
	GuiMsg.GetLetter().PushPipe(Cmd1);
	
	GetBrainData()->SendMsgToGUI(this,Dialog->m_SourceID,GuiMsg);
}