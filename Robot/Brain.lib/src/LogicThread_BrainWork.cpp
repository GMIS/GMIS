// LogicThread.cpp: implementation of the CLogicThread class.
// 大脑内部任务部分
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)


#include "TaskDialog.h"
#include "LogicThread.h"
#include "InstinctDefine.h"


/*	
void CLogicThread::BegingBrainTask(tstring TaskDescription){
    SendToConvS(MSG_CONV_CTRL,CM_BRAINTASKBEGIN,TaskDescription);
};
	
void CLogicThread::EndBrainTask(tstring FeekMsg){
   RuntimeOutput(FeekMsg);
   SendToConv3(MSG_CONV_CTRL,CM_TASKEND,SUCCESS);
   SetWorkState(THREAD_IDLE);
};

bool CLogicThread::LocalConv(ePipeline* Msg){
	ePipeline* Local  = GET_LOCALINFO(Msg);
	ePipeline* Letter = GET_LETTER(Msg);
	int64 CmdID = Letter->GetID();
			int n = Local->Size();
			n = Letter->Size();	
	switch(CmdID){
	case MSG_SAYTEXT:
		{
			if(m_InErrorState){
				m_InErrorState = false;
			}
			
			eElectron E3,E4;	 			
			Letter->Pop(&E3);
			assert(E3.EnergyType() == TYPE_STRING);
			tstring& Text = *(tstring*)E3.Value();
			
			Local->Pop(&E4);
			int32 Pos = *(int32*)E4.Value();
			assert(Pos>=0);
			
			ThinkProc(Pos,Text,true);
			if(m_Text.m_TokenList.size()==0)SetWorkState(THREAD_IDLE);
			else { 
				SetWorkState(THREAD_THINK);
			}
			
		}
		break;
	case MSG_SAY_END:
		{
			if(CanExecute()){
				SetForecast(NULL_TEXT,NULL);				
				ExecuteTask();	
			}
			else{ //否则要求继续edit	
				SendToConvS(MSG_CONV_CTRL,CM_CONTINUEINPUT,"");
			}
		}
		break;
	default:
		DefaultMsgProc(Msg->GetID(),Local,Letter);
	}
	return true;
}

void CLogicThread::OnI_AM(int64 SourceID,ePipeline* Local,ePipeline* Letter){
    //询问用户是否接受对方，不过不接受将关闭对话和关闭此连接。
	assert(0); //以后实现
	return;
}
void CLogicThread::OnNewConv(int64 SourceID,ePipeline* Local,ePipeline* Letter){
    //检查是否有附带信息，如果有并且是MSG_WHO_ARE_YOU则直接回答对方MSG_I_AM
	//否则询问用户是否接受，接受处理附带信息，否则关闭对哈（但是关闭连接）
			
	if(Letter->Size()){  //如果附带具体则放入信息管道重新处理
       eElectron E;
	   Letter->Pop(&E);
	   m_CentralNerve->Push((ePipeline*)E.Release());
	   return;	
	}
	assert(0);
	return;
} 

void CLogicThread::DefaultMsgProc(int64 SourceID,ePipeline* Local,ePipeline* Letter){
	eElectron E;
	int64 CmdID = Letter->GetID();
	switch(CmdID){
	case MSG_RECEIVE_OK:
		return ;
	case MSG_RECEIVE_ERROR:
		{
			Letter->Pop(&E);
			ePipeline* SrcMsg = (ePipeline*)E.Release();
			
			CLinker* Who = GetBrain()->GetLinker(SourceID);
			GetBrain()->ReSendMsg(Who,SrcMsg);//重发
		}
		return ;
	case MSG_I_AM:
		OnI_AM(SourceID,Local,Letter);
		break;
	case MSG_NEW_CONV:
		OnNewConv(SourceID,Local,Letter);
		break;
	case MSG_ACCEPT_CONV:
		return ;
        break;
	case MSG_CONV_BUSY:
		return ;
		break;
	case MSG_CLOSE_CONV:
		{
			CLinker* Who = GetBrain()->GetLinker(SourceID);
            if(Who == NULL)return;
			GetBrain()->LockLinker(Who);
			Local->Pop(&E);
			int64 FromLocalID  = E.Int64();
			Who->DeleteLocalID(FromLocalID);
			GetBrain()->LockLinker(Who);
		}
		break;
	case MSG_SEND_PROGRESS:
		{
			Letter->Pop(&E);
			int32 Completed = E.Int32();
			Letter->Pop(&E);
			int32 Count = E.Int32();
			int32 Per = Completed*100/Count;
	        CConversation* Conv = GetConv();
			if(Conv){
			    SendToConv33(MSG_CONV_OUTPUT,CM_SET_PROGRESS,Per,OUT_PROGRESS);
			}
			if(Per==100){
				int64 CmdID = Letter->PopInt64();
				GetBrain()->PrintMsg(CmdID,true);
			}
		}
		break;
	case MSG_SAYCONTINUE:
		{	//通常此消息从相应的CLinker传来			
			SendToConvS(MSG_CONV_CTRL,CM_CONTINUEINPUT,"");
		}
		break;
	case MSG_SAYISEE:
		{//通常此消息从相应的CLinker传来
		//	GetConv()->ExeBegin();
		}
		break;
    case MSG_SHOW_THINKRESULT:{
		    tstring s = m_Text.PrintThinkResult();
		    SendToConv3S(MSG_CONV_OUTPUT,CM_OUTPUT_RESULT,ANALYSE_RESULT,s);
		}
		break;
	case MSG_SHOW_ANALYSERESULT:{
			tstring s = m_Text.PrintAnlyseResult();
			SendToConv3S(MSG_CONV_OUTPUT,CM_OUTPUT_RESULT,ANALYSE_RESULT,s);
		}
		break;
	case MSG_SET_GLOBLELOGIC:
		{
			int n = Local->Size();
			n = Letter->Size();
			DoSetGlobleLogic(Letter); 
		}
		break;
	case MSG_TASK_CONTROL:
		assert(0);
		break;
	default:
		//委托他人的任务已经执行完毕，现在处理对方返回的结果
		//Thread应该正阻断在某个地方等待这个消息，所以这里应该没有机会处理
	    ASSERT(CmdID != MSG_TASK_END);
	    ASSERT(CmdID != MSG_EXECUTE_RESULT);
		break;
	}
}
	
bool  CLogicThread::AskInputText(tstring Tip,tstring& Result){
	ClearAnalyse();

 	//把Memo置入当前输入区,并转为文本输入模式
    SendToConvSS(MSG_CONV_CTRL,CM_TASKFEEDBACK,Tip,Result); 
	
	int64 ID = 0;
	bool LearnEnd = false;
	while(m_Alive){
		if(m_CentralNerve->Size() == 0){
			ExecuteForecast();
		}else{
			eElectron E1;
			m_CentralNerve->Pop(&E1);
			assert(E1.EnergyType() == TYPE_PIPELINE);
			
			ePipeline* Msg = (ePipeline*)E1.Value();
			ePipeline* Local  = GET_LOCALINFO(Msg);
			ePipeline* Letter = GET_LETTER(Msg);
			int64 SourceID = Msg->GetID();
			int64 LocalID  = Local->GetID();
			int64 CmdID = Letter->GetID();
			
			switch(CmdID){
			case MSG_SAYTEXT:
				{
					eElectron E1,E2;	 			
					Letter->Pop(&E1);
					assert(E1.EnergyType() == TYPE_STRING);
					tstring& Text = *(tstring*)E1.Value();
					
					Local->Pop(&E2);
					int32 Pos = *(int32*)E2.Value();
					assert(Pos>=0);
					
					ChatProc(Pos,Text);
				}
				break;
			case MSG_TASK_CONTROL:
				{
					eElectron E;
					Letter->Pop(&E);
					int32 ControlID = *(int32*)E.Value();
					if(ControlID == CMD_STOP){
						return false;
					}
				}
				break;
			case MSG_SAY_END:
				SendToConvS(MSG_CONV_CTRL,CM_WAITEND,"Wait...");
				Result = m_Text.GetText(); 
				return true;
			default:
				DefaultMsgProc(Msg->GetID(),Local,Letter);
			}
			
		}
	}
	return false;
}

void  CLogicThread::DoSetGlobleLogic(ePipeline* Letter){
	int64 ThreadID = Letter->PopInt64();
	tstring LogicName;
	Letter->PopString(LogicName);
    int32 ActionState;
	tstring LogicText;
	tstring LogicMeaning;
    ePipeline LogicData;

	tstring LogicInputDescription;
	tstring LogicOutputDescription;

	tstring s = format("Set Globle Logic <%s>",LogicName.c_str());
	BegingBrainTask(s);
	
    if(ThreadID == GetLocalID()){
        CLocalLogicCell* LogicCell = FindLogic(LogicName);
        if(LogicCell == NULL){
			s = format("Not find temp logic <%s>",LogicName.c_str());
            EndBrainTask(s);
			return;
		}else{
			ActionState = LogicCell->m_ActionState;
			LogicText  = LogicCell->m_LogicText;
			LogicMeaning  = LogicCell->m_LogicMemo;
			LogicData = LogicCell->m_LogicData;
		}
	}else{
        ActionState = Letter->PopInt32();
      	Letter->PopString(LogicText);
		Letter->PopString(LogicMeaning);
		eElectron e;
		Letter->Pop(&e);
		ePipeline* Pipe = (ePipeline*)e.Value();
		LogicData << *Pipe;
	}
	//0首先检查是否能编译通过
	CTask Task;
		
	SendToConvS(MSG_CONV_OUTPUT,CM_CONV_TITLE,"Set Globle Logic");
			
	SetWorkState(THREAD_COMPILE);

    m_CurrentTask = &Task;
    Task.m_OwnerThread = this;

	//还没通知刷新不用锁定
	Task.SetTaskState(TASK_COMPILE);
	Task.m_BeginTime = CHighTime::GetPresentTime();
	Task.m_Executer  = m_Name;

	
	RuntimeOutput("--------------------------\nCompile Logic <%s>...",LogicName.c_str());
	
	SendToConv(MSG_CONV_CTRL,CM_NTASKRUN);
    bool Ret = Task.Compile(&LogicData);
	if(Ret==false){
		RuntimeOutput("Compile Error: %s\n",Task.m_CompileError.c_str());
		EndBrainTask("fail"); 
		return ;
	}
    
	m_CurrentTask = NULL;
    Task.m_OwnerThread = NULL;
	char buf[100];
	CHighTimeSpan Span = CHighTime::GetPresentTime() - Task.m_BeginTime;
	Task.m_CompileTime = (const char*)Span.Format(buf,100,"Compile Spend : %H:%M:%S.%s");
	RuntimeOutput("%s\n",Task.m_CompileTime.c_str());
	
	tstring feedback = "User cancle";
	//1询问要求输入逻辑描述
	tstring tip = "Please input logic meaning:";
	if(AskInputText(tip,LogicMeaning)==false){
		EndBrainTask(feedback); 
		return;
	};
				
	//2询问要求输入逻辑输入数据的格式描述
	tip = "Logic input data format description:";
	if(AskInputText(tip,LogicInputDescription)==false){
		EndBrainTask(feedback); 
	    return;
	};
				
	//3询问要求输入逻辑输出数据的格式描述			
	tip = "Logic output data format description:";
	if(AskInputText(tip,LogicOutputDescription)==false){
		EndBrainTask(feedback); 
		return;
	};

	RegisterGlobalLogic(LogicName,LogicText,LogicData,ActionState,LogicMeaning,LogicInputDescription,LogicOutputDescription);
    GetBrain()->m_LogicView.Refresh();

	feedback = "ok";
	EndBrainTask(feedback); 
				
}
*/