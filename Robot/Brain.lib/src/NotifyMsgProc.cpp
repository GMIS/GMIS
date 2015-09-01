#pragma warning (disable:4786)

#include "Brain.h"
#include "TaskDialog.h"
#include "GUIMsgDefine.h"



void CBrain::NotifySysState(int64 NotifyID,ePipeline* Data /*=NULL*/){
	ePipeline Receiver;
	Receiver.PushInt(SYSTEM_SOURCE);
	Receiver.PushInt(DEFAULT_DIALOG);
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	switch(NotifyID){
	case MNOTIFY_CENTRAL_NERVE_MSG_NUM:
		{
			assert(Data);
			ePipeline Cmd(GUI_PFM_MSG_UPDATE);
			Cmd.PushInt(CENTRALNERVE_MSG_NUM);
			Cmd.PushPipe(*Data);
			GuiMsg.GetLetter().PushPipe(Cmd);
			
			int64 GuiID = LOCAL_GUI;
			SendMsgToGUI(GuiID,GuiMsg);
			
		}
		break;
		case SNOTIFY_NERVE_MSG_NUM:
		{
			assert(Data);
			ePipeline Cmd(GUI_PFM_MSG_UPDATE);
			Cmd.PushInt(NERVE_MSG_NUM);
			Cmd.PushPipe(*Data);
			GuiMsg.GetLetter().PushPipe(Cmd);
			
			int64 GuiID = LOCAL_GUI;
			SendMsgToGUI(GuiID,GuiMsg);
		};
		break;
	case MNOTIFY_CENTRAL_NERVE_THREAD_JOIN:
		{
			int64 n = Data->PopInt();
			
			ePipeline Cmd(GUI_PFM_THREAD_UPDATE);
			Cmd.PushInt(THREAD_CENTRAL_NERVE);
			Cmd.PushInt(n);
			GuiMsg.GetLetter().PushPipe(Cmd);
			
			int64 GuiID = LOCAL_GUI;
			SendMsgToGUI(GuiID,GuiMsg);
			
		};
		break;
	case MNOTIFY_CENTRAL_NERVE_THREAD_FAIL:
		{
			//OutSysInfo(_T("Create New CentralNerve Thread Fail"));
		};
		break;
	case MNOTIFY_CENTRAL_NERVE_THREAD_CLOSE:
		{
			int64 n = Data->PopInt();
			ePipeline Cmd(GUI_PFM_THREAD_UPDATE);
			Cmd.PushInt(THREAD_CENTRAL_NERVE);
			Cmd.PushInt(n);
			GuiMsg.GetLetter().PushPipe(Cmd);
			
			int64 GuiID = LOCAL_GUI;
			SendMsgToGUI(GuiID,GuiMsg);
		};
		break;
	case SNOTIFY_NERVE_THREAD_JOIN:
		{
			int64 n = Data->PopInt();
			ePipeline Cmd(GUI_PFM_THREAD_UPDATE);
			Cmd.PushInt(THREAD_NERVE);
			Cmd.PushInt(n);
			GuiMsg.GetLetter().PushPipe(Cmd);
			
			int64 GuiID = LOCAL_GUI;
			SendMsgToGUI(GuiID,GuiMsg);
			
		};
		break;
	case SNOTIFY_NERVE_THREAD_FAIL:
		{
			//OutSysInfo(_T("Create New Nerve Thread Fail"));
		};
		break;
	case SNOTIFY_NERVE_THREAD_CLOSE:
		{
			int64 n = Data->PopInt();
			ePipeline Cmd(GUI_PFM_THREAD_UPDATE);
			Cmd.PushInt(THREAD_NERVE);
			Cmd.PushInt(n);
			GuiMsg.GetLetter().PushPipe(Cmd);
			
			int64 GuiID = LOCAL_GUI;
			SendMsgToGUI(GuiID,GuiMsg);
		};
		break;
	case SNOTIFY_LISTEN_FAIL:
		{
			tstring s=Data->PopString();
			OutSysInfo(s);
		};
		break;
	case MNOTIFY_ILLEGAL_MSG:
		{
			CMsg Msg(Data);
			int64 MsgID = Msg.GetMsgID();
			tstring MsgStr = MsgID2Str(MsgID);
			tstring s = Format1024(_T("Receive illegal msg: %I64ld(%s)"),MsgID,MsgStr.c_str());
			Msg.Release(); //避免delete Data;
			OutSysInfo(s.c_str());	
		};
		break;
	default:
		{
			System::NotifySysState(NotifyID,Data);
		}
	}
	
};

void CBrain::NotifyLinkerState(CLinkerPipe* LinkerPipe,int64 NotifyID,ePipeline& Info){
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
						OutSysInfo(s);
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
				OutSysInfo(s);
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
				OutSysInfo(s);
			}
			return;
		case LINKER_ILLEGAL_MSG:
			{
				ePipeline* Msg = (ePipeline*)Info.GetData(0);
				
				CPipeView PipeView(Msg);
				
				tstring text = PipeView.GetString();
				
				tstring s = Format1024(_T("Linker Notify: receive illegal msg \n<<\n%s\n>>\n"),text.c_str());
				
				OutputLog(LOG_WARNING,s.c_str());
				OutSysInfo(s);	
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
	
}




void CBrain::NoitfyDialogState(CTaskDialog* Dialog, ePipeline* NotifyInfo){
	CLock lk(&Dialog->m_Mutex);

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
		OutSysInfo(s);
		assert(0);
	};
}

void CBrain::OnNotifyDialogError(CTaskDialog* Dialog, ePipeline* NotifyInfo){


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

			//构建一个临时输出，但是并不正式做记录
			ePipeline Cmd0(GUI_RUNTIME_OUTPUT);
			ePipeline Item;
			Item.PushInt(0);
			Item.PushString(error);
			Cmd0.PushPipe(Item);

            GuiMsg.GetLetter().PushPipe(Cmd0);
			
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

			//构建一个临时输出，但是并不正式做记录
			ePipeline Cmd0(GUI_RUNTIME_OUTPUT);
			ePipeline Item;
			Item.PushInt(0);
			Item.PushString(error);
			Cmd0.PushPipe(Item);
			
            GuiMsg.GetLetter().PushPipe(Cmd0);
			
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

	int64 GuiID = Dialog->m_OutputSourceID;
	SendMsgToGUI(GuiID,GuiMsg);
}

void CBrain::OnNotifyDialogState(CTaskDialog* Dialog, ePipeline* NotifyInfo){

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
			Dialog->m_Brain->GetBrainData()->GetBrainInfo(Info);
			
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

	int64 GuiID = Dialog->m_OutputSourceID;
	SendMsgToGUI(GuiID,GuiMsg);
}

void CBrain::OnNotifyDialogOutput(CTaskDialog* Dialog, ePipeline* NotifyInfo){	

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

	int64 GuiID = Dialog->m_OutputSourceID;
	SendMsgToGUI(GuiID,GuiMsg);

}

void CBrain::OnNotifyLogicView(CTaskDialog* Dialog, ePipeline* NotifyInfo){

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);

	ePipeline Cmd1(GUI_LOGIC_OUTPUT);
	Cmd1<<*NotifyInfo;
	GuiMsg.GetLetter().PushPipe(Cmd1);
	
	int64 GuiID = Dialog->m_OutputSourceID;;
	SendMsgToGUI(GuiID,GuiMsg);
};

void CBrain::OnNotifyObjectList(CTaskDialog* Dialog, ePipeline* NotifyInfo){

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd1(GUI_OBJECT_OUTPUT);
	Cmd1<<*NotifyInfo;
	GuiMsg.GetLetter().PushPipe(Cmd1);
	
	int64 GuiID = Dialog->m_OutputSourceID;;
	SendMsgToGUI(GuiID,GuiMsg);
}

void CBrain::OnNotifyDialogList(CTaskDialog* Dialog, ePipeline* NotifyInfo){

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
				OutSysInfo(Reason);
			}
		}
		break;
	default:
		break;
	}

	int64 GuiID = Dialog->m_OutputSourceID;;
	SendMsgToGUI(GuiID,GuiMsg);
}

void CBrain::OnNotifyProgressOutput(CTaskDialog* Dialog, ePipeline* NotifyInfo)
{
	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	int64 Act = NotifyInfo->PopInt();
	switch(Act){
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
	
	int64 GuiID = Dialog->m_OutputSourceID;;
	SendMsgToGUI(GuiID,GuiMsg);
	
}
void CBrain::OnNotifyDebugView(CTaskDialog* Dialog, ePipeline* NotifyInfo){


	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd1(GUI_DEBUGVIEW_OUTPUT);
	Cmd1<< *NotifyInfo;
	
	GuiMsg.GetLetter().PushPipe(Cmd1);

	int64 GuiID = Dialog->m_OutputSourceID;;
	SendMsgToGUI(GuiID,GuiMsg);
}

void CBrain::OnNotifyBrainInit(CTaskDialog* Dialog, ePipeline* NotifyInfo){
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
	}else if (Type == INIT_DIALOG_LIST)
	{
		ePipeline DialogList;
		GetBrainData()->GetAllDialogListInfo(DialogList);

		ePipeline Cmd1(GUI_LINKVIEW_OUTPUT);
		Cmd1.PushInt(INIT_LIST);
		Cmd1.PushPipe(DialogList);
		GuiMsg.GetLetter().PushPipe(Cmd1);

	}else{
		return ;
	}

	
	int64 GuiID = Dialog->m_OutputSourceID;;
	SendMsgToGUI(GuiID,GuiMsg);	
}

void CBrain::OnNotifyFindInfo(CTaskDialog* Dialog, ePipeline* NotifyInfo){

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
		
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd1(GUI_FIND_OUTPUT);
	Cmd1<< *NotifyInfo;
	GuiMsg.GetLetter().PushPipe(Cmd1);

	int64 GuiID = Dialog->m_OutputSourceID;;
	SendMsgToGUI(GuiID,GuiMsg);	
}


void CBrain::OnNotifyMemoryList(CTaskDialog* Dialog, ePipeline* NotifyInfo){

	ePipeline Receiver;
	Receiver.PushInt(Dialog->m_SourceID);
	Receiver.PushInt(Dialog->m_DialogID);
	
	CMsg GuiMsg(Receiver,MSG_BRAIN_TO_GUI,0);
	
	ePipeline Cmd1(GUI_MEMORY_OUTPUT);
	Cmd1<< *NotifyInfo;
	GuiMsg.GetLetter().PushPipe(Cmd1);
	
	int64 GuiID = Dialog->m_OutputSourceID;;
	SendMsgToGUI(GuiID,GuiMsg);	
}