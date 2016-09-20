#include "..\ObjectList.h"

#if defined _COMPILE_TEST_OBJECT


#include <assert.h>

void CTestObject::OnObjectRun(CMsg& Msg){

	int64 SenderID  = Msg.GetSenderID();
	ePipeline& Letter = Msg.GetLetter(true);
	eElectron E;
	Letter.Pop(&E);

	if(E.EnergyType() != TYPE_PIPELINE){
		ePipeline ExePipe;
		SendFeedbackError(SenderID,ExePipe,_T("the inputed data is illegal"));
		return;
	}

	ePipeline& ExePipe     = *(ePipeline*)E.Value();

	//测试运行时输出
	ePipeline RuntimeInfo;
	RuntimeInfo.PushString(_T("this is a  runtime info"));
	SendRuntimeInfo(SenderID,RuntimeInfo);
	
	//测试运行时逻辑
	tstring LogicText =_T("please input a num ");
	SendRuntimeLogic(SenderID,LogicText);

	ePipeline LogicExePipe;
	if(!WaitingForLogicResult(SenderID,LogicExePipe)){
		return ;
	}

	if(LogicExePipe.GetID()==RETURN_ERROR){
		tstring& error = LogicExePipe.GetLabel();
		SendFeedbackError(SenderID,LogicExePipe,error);
		return;
	}
	if(!LogicExePipe.HasTypeAB(PARAM_TYPE1(TYPE_INT))){
		SendFeedbackError(SenderID,LogicExePipe,_T("the inputed data is not a num"));
		return;
	}

	int64 n = LogicExePipe.PopInt();
	
	n=n*2;

	LogicExePipe.PushInt(n);
	SendFeedback(SenderID,LogicExePipe);
}


void CTestObject::OnObjectGetDoc(CMsg& Msg)
{
	int64 SenderID  = Msg.GetSenderID();

	ePipeline& Letter = Msg.GetLetter(true);

	eElectron E;
	Letter.Pop(&E);

	if(E.EnergyType() != TYPE_PIPELINE){
		ePipeline ExePipe;
		tstring Error =_T("the inputed data is illegal");
		SendFeedbackError(SenderID,ExePipe,Error);
		return;
	}

	ePipeline& ExePipe     = *(ePipeline*)E.Value();
	tstring s = __TEXT(\
		"\
		this is a test object for testing MSG_TASK_RUNTIME_INFO and MSG_TASK_RUNTIME_LOGIC\n\
		no input param required\n\
		"\
		);		
	ExePipe.PushString(s);
	SendFeedback(SenderID,ExePipe);
}


#endif //_COMPILE_TEST_OBJECT