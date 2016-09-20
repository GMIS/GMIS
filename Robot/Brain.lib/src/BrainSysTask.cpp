// BrainObject.cpp: implementation of the CBrainObject class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "Brain.h"
#include "BrainSysTask.h"
#include "GUIMsgDefine.h"
#include "InstinctDefine.h"
#include "LogicDialog.h"
struct NewWord{ 
	int64         PartOfSpeech;
	const TCHAR*  Word1;
	const TCHAR*  Word2;
};


//初始化基本单词本能，其中 
NewWord _WordList[]={    //note: 增减项目必须修改初始化单词数目


	{MEMORY_VERB,		_T("define"),	_T("定义")},
	{MEMORY_NOUN,		_T("int"),		_T("整数")},
	{MEMORY_NOUN,		_T("float"),	_T("小数")},
	{MEMORY_NOUN,		_T("string"),	_T("字符串")},

	{MEMORY_LOGIC_THEN,	_T("then"),		_T("然后")},
	{MEMORY_LOGIC_AND,	_T("and"),		_T("同时")},

	{MEMORY_VERB,		_T("use"),		_T("使用")},
	{MEMORY_NOUN,		_T("operator"),	_T("操作符")},

	{MEMORY_NOUN,		_T("resistor"),	_T("电阻")},
	{MEMORY_NOUN,		_T("rs"),		_T("电阻")},

	{MEMORY_NOUN,		_T("inductor"),	_T("电感")},
	{MEMORY_NOUN,		_T("id"),		_T("电感")},

	{MEMORY_NOUN,		_T("capacitor"),_T("电容")},
	{MEMORY_NOUN,		_T("cp"),		_T("电容")},

	{MEMORY_NOUN,		_T("diode"),	_T("二极管")},
	{MEMORY_NOUN,		_T("dd"),		_T("二极管")},

	{MEMORY_VERB,		_T("reference"),_T("引用")},

	{MEMORY_VERB,		_T("set"),		_T("设置")},
	{MEMORY_NOUN,		_T("label"),	_T("标签")},
	{MEMORY_VERB,		_T("goto"),		_T("转向")},
	{MEMORY_VERB,		_T("view"),		_T("查看")}, 
	{MEMORY_NOUN,		_T("pipe"),		_T("管道")}, 

	{MEMORY_VERB,		_T("input"),	_T("输入")},
	{MEMORY_NOUN,		_T("text"),		_T("文本")},
	{MEMORY_NOUN,		_T("num"),		_T("数字")},
	{MEMORY_VERB,		_T("wait"),		_T("等待")},
	{MEMORY_NOUN,		_T("second"),	_T("秒")},

	{MEMORY_VERB,		_T("create"),	_T("制造")},
	{MEMORY_VERB,		_T("import"),	_T("导入")},
	{MEMORY_VERB,		_T("export"),	_T("导出")},

	{MEMORY_VERB,		_T("focus"),	_T("关注")},
	{MEMORY_VERB,		_T("insert"),	_T("插入")},
	{MEMORY_VERB,		_T("modify"),	_T("修改")},
	{MEMORY_NOUN,		_T("data"),		_T("数据")},
	{MEMORY_VERB,		_T("get"),		_T("得到")},
	{MEMORY_VERB,		_T("remove"),	_T("移除")},
	{MEMORY_NOUN,		_T("size"),		_T("大小")},
	{MEMORY_VERB,		_T("close"),	_T("关闭")},

	{MEMORY_NOUN,		_T("logic"),	_T("逻辑")},
	{MEMORY_VERB,		_T("name"),		_T("名字")},

	{MEMORY_NOUN,		_T("date"),		_T("日期")},

	{MEMORY_VERB,		_T("start"),	_T("启动")},
	{MEMORY_NOUN,		_T("object"),	_T("物体")},

	{MEMORY_VERB,		_T("execute"),	_T("执行")},
	{MEMORY_NOUN,		_T("request"),	_T("请求")},

	{MEMORY_NOUN,		_T("robot"),	_T("机器人")},

	{MEMORY_VERB,		_T("call"),	    _T("呼叫")},

	{MEMORY_VERB,		_T("think"),	_T("思考")},

	{MEMORY_VERB,		_T("run"),		_T("运行")},
	{MEMORY_VERB,		_T("debug"),	_T("调试")},
	{MEMORY_VERB,		_T("stop"),		_T("停止")},
	{MEMORY_VERB,		_T("pause"),	_T("暂停")},
	{MEMORY_VERB,		_T("step"),		_T("单步")},
	{MEMORY_VERB,		_T("test"),		_T("测试")},

	{MEMORY_NOUN,		_T("dialog"),	_T("对话")},

	{MEMORY_VERB,		_T("learn"),	_T("学习")}, 
	{MEMORY_NOUN,		_T("token"),	_T("符号")},
	{MEMORY_NOUN,		_T("pronoun"),	_T("代词")},
	{MEMORY_NOUN,		_T("adjective"),_T("形容词")},
	{MEMORY_NOUN,		_T("numeral"),	_T("数字")},
	{MEMORY_NOUN,		_T("verb"),		_T("动词")},
	{MEMORY_NOUN,		_T("adverb"),	_T("副词")},
	{MEMORY_NOUN,		_T("article"),	_T("冠词")},
	{MEMORY_NOUN,		_T("preposition"),_T("介词")},
	{MEMORY_NOUN,		_T("conjunction"),_T("连词")},
	{MEMORY_NOUN,		_T("interjection"),_T("感叹词")},
	{MEMORY_NOUN,		_T("noun"),		_T("名词")},
	{MEMORY_NOUN,		_T("text"),		_T("文本")},

	{MEMORY_NOUN,		_T("action"),	_T("行为")},
	{MEMORY_VERB,		_T("find"),		_T("搜索")},
	{MEMORY_NOUN,		_T("time"),		_T("时间")},
	{MEMORY_NOUN,		_T("end"),		_T("终点")},
	{MEMORY_NOUN,		_T("pricision"),_T("精度")},	
	{MEMORY_VERB,		_T("output"),	_T("输出")}, 
	{MEMORY_NOUN,		_T("info"),		_T("信息")}, 
	{MEMORY_NOUN,		_T("document"), _T("文档")}, 
	{MEMORY_NOUN,		_T("expectation"),_T("预期")},

	{MEMORY_NOUN,		_T("account"),	_T("帐号")}, 
	{MEMORY_NOUN,		_T("memory"),	_T("记忆")}, 
	{MEMORY_VERB,		_T("delete"),	_T("删除")}, 
	{MEMORY_NOUN,		_T("address"),	_T("地址")}, 
	{MEMORY_NOUN,		_T("node"),		_T("节点")}, 
	{MEMORY_NOUN,		_T("focus"),	_T("焦点")}, 
	{MEMORY_NOUN,		_T("breakpoint"),_T("断点")}, 	
	{MEMORY_NOUN,		_T("temp"),		_T("临时")}, 
	{MEMORY_VERB,		_T("clear"),	_T("清除")}, 
	{MEMORY_VERB,		_T("print"),	_T("打印")},
	{MEMORY_NOUN,		_T("word"),		_T("单词")},
	{MEMORY_NOUN,		_T("language"), _T("语言")},
	{MEMORY_VERB,		_T("chat"),     _T("聊天")},
};


//初始化本能行为
struct InitInstinct{ 
	int64         InstinctID;
	const TCHAR*  Cmd;
}; 

InitInstinct _InstinctList[]={

	//通用命令
	{INSTINCT_DEFINE_INT64,				_T("define int")},
	{INSTINCT_DEFINE_FLOAT64,			_T("define float")},
	{INSTINCT_DEFINE_STRING,			_T("define string") },

	{INSTINCT_USE_OPERATOR,				_T("use operator")},


	{INSTINCT_USE_RESISTOR,				_T("use Resistor")},
	{INSTINCT_USE_INDUCTOR,				_T("use inductor")},
	{INSTINCT_USE_CAPACITOR,			_T("use capacitor")},
	{INSTINCT_USE_DIODE,				_T("use Diode")},

	{INSTINCT_REFERENCE_CAPACITOR,		_T("reference capacitor")},
	{INSTINCT_REFERENCE_INDUCTOR,		_T("reference inductor")},


	{INSTINCT_SET_LABEL,				_T("set  label ")},
	{INSTINCT_GOTO_LABEL,				_T("goto label ")},
	{INSTINCT_PRINT_PIPE,				_T("print pipe")},

	{INSTINCT_INPUT_TEXT,				_T("input text")},
	{INSTINCT_INPUT_NUM,				_T("input num")},
	{INSTINCT_WAIT_TIME,				_T("wait time")},


	{INSTINCT_CREATE_MEMORY,			_T("create memory")},
	{INSTINCT_FOCUS_MEMORY,				_T("focus  memory")},
	{INSTINCT_SET_MEMORY_ADDRESS,		_T("set memory address")},
	{INSTINCT_GET_MEMORY_ADDRESS,		_T("get memory address")},
	{INSTINCT_CREATE_MEMORY_NODE,		_T("create memory node")},
	{INSTINCT_IMPORT_MEMORY,			_T("import memory")},
	{INSTINCT_EXPORT_MEMORY,			_T("export memory")},
	{INSTINCT_GET_MEMORY,				_T("get memory")},
	{INSTINCT_INSERT_MEMORY,			_T("insert memory")},
	{INSTINCT_MODIFY_MEMORY,			_T("modify memory")},
	{INSTINCT_REMOVE_MEMORY,			_T("remove memory")},
	{INSTINCT_GET_MEMORY_SIZE,			_T("get memory size")},
	{INSTINCT_CLOSE_MEMORY,				_T("close memory")},
	{INSTINCT_GET_MEMORY_FOCUS,			_T("get memory focus")},

	{INSTINCT_USE_LOGIC,				_T("use logic")},
	{INSTINCT_FOCUS_LOGIC,				_T("focus logic")},
	{INSTINCT_NAME_LOGIC,				_T("name logic")},
	{INSTINCT_INSERT_LOGIC,				_T("insert logic")},
	{INSTINCT_REMOVE_LOGIC,				_T("remove logic")},
	{INSTINCT_SET_LOGIC_ADDRESS,		_T("set logic address")},
	{INSTINCT_GET_LOGIC_ADDRESS,		_T("get logic address")},
	{INSTINCT_SET_LOGIC_BREAKPOINT,		_T("set logic breakpoint")},
	{INSTINCT_REMOVE_TEMP_LOGIC,		_T("remove temp logic")},
	{INSTINCT_CLEAR_TEMP_LOGIC,			_T("clear temp logic")},

	{INSTINCT_GET_TIME,					_T("get time")},

	{INSTINCT_OUTPUT_INFO,				_T("output  info")},
	{INSTINCT_TEST_EXPECTATION,			_T("test  expectation")},

	{INSTINCT_START_OBJECT,				_T("start object")},
	{INSTINCT_FOCUS_OBJECT,				_T("focus object")},
	{INSTINCT_NAME_OBJECT,				_T("name object")},
	{INSTINCT_USE_OBJECT,				_T("use object")},
	{INSTINCT_CLOSE_OBJECT,				_T("close object")},
	{INSTINCT_GET_OBJECT_DOC,			_T("get object document")},
	{INSTINCT_CALL_ROBOT,				_T("call robot")},
	{INSTINCT_FOCUS_REQUEST,			_T("focus request")},
	{INSTINCT_NAME_REQUEST,			    _T("name request")},
	{INSTINCT_EXECUTE_REQUEST,			_T("execute request")},
	{INSTINCT_CLOSE_REQUEST,			_T("close request")},
	{INSTINCT_CHAT_ROBOT,				_T("chat")},

	//内部命令
	{INSTINCT_THINK_LOGIC,				_T("think logic")},

	{INSTINCT_RUN_TASK,					_T("run")},
	{INSTINCT_DEBUG_TASK,				_T("debug")},
	{INSTINCT_STOP_TASK,				_T("stop")},
	{INSTINCT_PAUSE_TASK,				_T("pause")},
	{INSTINCT_STEP_TASK,				_T("step")},
	{INSTINCT_GOTO_TASK,				_T("goto")},	
	{INSTINCT_TEST_TASK,				_T("test")},	

	{INSTINCT_CLOSE_DIALOG,				_T("close dialog")},

	{INSTINCT_CREATE_ACCOUNT,			_T("create account")},
	{INSTINCT_DELETE_ACCOUNT,			_T("delete account")},

	//内部非独立命令
	{INSTINCT_LEARN_WORD,				_T("learn word")},
	{INSTINCT_LEARN_TEXT,				_T("learn text")},

	{INSTINCT_LEARN_LOGIC,				_T("learn logic")},
	{INSTINCT_LEARN_OBJECT,				_T("learn object")},
	{INSTINCT_LEARN_ACTION,				_T("learn action")},
	{INSTINCT_LEARN_LANGUAGE,			_T("learn language")},

	{INSTINCT_FIND,						_T("find") },
	{INSTINCT_FIND_LOGIC,				_T("find logic")},
	{INSTINCT_FIND_OBJECT,				_T("find object")}

};

//////////////////////////////////////////////////////////////////////////

CBrainInitElt::CBrainInitElt(int64 ID,tstring Name,ePipeline& Param)
	:CElement(0,_T("Brain Init")){
	m_Param = Param;
};

CBrainInitElt::~CBrainInitElt(){

}

bool CBrainInitElt::CheckWord(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress){

	CLogicDialog* SysDialog = GetBrain()->GetBrainData()->GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
	assert(SysDialog);

	CLogicThread& Think = *Dialog->GetThink();	

	Think.ClearAnalyse();

	if (m_Param.Size()==0)
	{
		m_Param.PushInt(0);

		Dialog->RuntimeOutput(_T("Checking  word..."));
	}

	int64& Index = *(int64*)m_Param.GetData(0);


	int n = (sizeof(_WordList)/sizeof(_WordList[0]));

	bool bLearnChinese = false;
	try{	

		for(Index;Index<n;Index++)
		{
			int64 State = ExePipe.GetID();

			if( !ExePipe.IsAlive() ){
				Dialog->SetTaskState(TASK_STOP);
				return false;
			}else if(State == RETURN_BREAK){
				Dialog->NotifyPause(ExePipe,LocalAddress);
				return true;
			}

			NewWord& r = _WordList[Index];
			tstring s = Format1024(_T("Checking  word =\"%s\""),r.Word1);

			CNotifyDialogState nf(NOTIFY_BRAIN_INIT);
			nf.PushInt(INIT_PROGRESS);
			nf.PushString(s);
			nf.PushInt(Index*100/n);
			nf.Notify(Dialog);

			tstring word = r.Word1;

			int64 ID = Think.CheckWord(word,r.PartOfSpeech,MEANING_SENSE_OK);
			if(ID==0){
				SysDialog->RuntimeOutput(0,_T("Learning word \"%s\" "),r.Word1);										

				ID = Think.LearnWord(word,r.PartOfSpeech,MEANING_SENSE_OK);

				if(ID==0){
					SysDialog->RuntimeOutput(0,_T("WARNING: Learn word [%s]  fail"),r.Word1);
				}else if(bLearnChinese){
					//学习中文
					SysDialog->RuntimeOutput(0,_T("Learning word \"%s\" "),r.Word2);	
					word = r.Word2;
					ID = Think.LearnWordFromWord(word,r.PartOfSpeech,tstring(r.Word1));
					if(ID==0){
						SysDialog->RuntimeOutput(0,_T("Learn word [%s]  fail"),r.Word2); 
					}
				} 
			}
		}
		if (Index == n)
		{
			m_Param.m_ID = 1; //准备执行下一阶段任务
			m_Param.PopInt();
		}

	}catch(CppSQLite3Exception &e ){
		AnsiString s = e.errorMessage();
		tstring error = UTF8toWS(s);
		Dialog->RuntimeOutput(0,_T("BrainDB  fail: %s"),error); 
		return false;
	}catch(...){
		return false;
	}
	return true;
}

bool CBrainInitElt::CheckInstinct(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress){
	CLogicDialog* SysDialog = GetBrain()->GetBrainData()->GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
	assert(SysDialog);

	CLogicThread& Think = *Dialog->GetThink();	
	Think.ClearAnalyse();

	int n = (sizeof(_InstinctList)/sizeof(_InstinctList[0]));

	if (m_Param.Size()==0)
	{
		m_Param.PushInt(0);

		Dialog->RuntimeOutput(_T("Checking  instinct..."));
	}

	int64& Index = *(int64*)m_Param.GetData(0);

	try{	
		for(Index; Index<n; Index++)
		{
			int64 State = ExePipe.GetID();

			if( !ExePipe.IsAlive() ){
				Dialog->SetTaskState(TASK_STOP);
				return false;
			}else if(State == RETURN_BREAK){

				Dialog->NotifyPause(ExePipe,LocalAddress);
				return true;
			}

			InitInstinct& r = _InstinctList[Index];


			tstring s = Format1024(_T("Checking  instinct =\"%s\""),r.Cmd);

			CNotifyDialogState nf(NOTIFY_BRAIN_INIT);
			nf.PushInt(INIT_PROGRESS);
			nf.PushString(s);
			nf.PushInt(Index*100/n);
			nf.Notify(Dialog);

			int64 ID = Think.CheckAction(Dialog,tstring(r.Cmd),r.InstinctID,MEANING_SENSE_OK);
			if(ID==0){
				SysDialog->RuntimeOutput(0,_T("Learning instinct \"%s\" "),r.Cmd);

				ID = Think.LearnAction(Dialog,tstring(r.Cmd),r.InstinctID,MEANING_SENSE_OK);
				if(ID==0){
					SysDialog->RuntimeOutput(0,_T("WARNING! Learn instinct \"%s\" fail: %s>"),r.Cmd,Think.m_LastError.c_str());
				} 
			} 
		};

		if(Index == n){
			Index = 0;
		}
	}catch(CppSQLite3Exception &e ){
		AnsiString s = e.errorMessage();
		tstring error = UTF8toWS(s);
		Dialog->RuntimeOutput(0,_T("BrainDB  fail: %s"),error); 

		return false;
	}catch(...){
		return false;
	}
	return true;
}

bool  CBrainInitElt::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg)
{
	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);

	if(!Msg.IsReaded()){
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	try{

		if (m_Param.m_ID == 0)
		{
			bool ret = CheckWord(Dialog,ExePipe,LocalAddress);
			if(!ret){
				return false;
			}
		}

		if (m_Param.m_ID == 1)
		{
			bool ret = CheckInstinct(Dialog,ExePipe,LocalAddress);
			if (!ret)
			{
				return false;
			}
		}

		if(ExePipe.IsAlive()){
			GetBrain()->OutSysInfo(_T("Checking memory....OK"));
		}
	}catch(CppSQLite3Exception &e ){ 

		AnsiString s = e.errorMessage();
		tstring ws = UTF8toWS(s);

		tstring str = Format1024(_T("WARNING: Brain Database error>>%s"),ws.c_str());
		Dialog->RuntimeOutput(str);
		return false;
	}
	catch(...){
		Dialog->RuntimeOutput(0,_T("%s fail: Unkown error"),m_Name.c_str());
		return false;
	} 

	return true;
}


//////////////////////////////////////////////////////////////////////////
CBrainGotoSpaceElt::CBrainGotoSpaceElt(int64 ID,tstring Name,ePipeline& Param)
	:CElement(0,_T("Goto Space")){
	m_Param = Param;

};

CBrainGotoSpaceElt::~CBrainGotoSpaceElt(){

}

bool  CBrainGotoSpaceElt::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg)
{
	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);

	if (!Msg.IsReaded())
	{
		int64 MsgID = Msg.GetMsgID();
		if(MsgID == MSG_EVENT_TICK)
		{
			int64 EventID = Msg.GetEventID();
			if (EventID != GetEventID()) //调试暂停后也会持续收到此信息
			{
				int32 ChildIndex = IT_SELF;
				MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
				if (ret == RETURN_DIRECTLY)
				{
					return true;
				}
			}

			ePipeline& Letter = Msg.GetLetter(true); 

			int64 TimeStamp   = Letter.PopInt();
			float64 t = (float64)(TimeStamp-GetEventID());
			t /=(float64)(TIME_SEC);

			if(t>5){ //超过5秒则取消此任务

				CBrainEvent EventInfo;
				bool ret = GetBrain()->GetBrainData()->GetEvent(EventID,EventInfo,true);
				if (!ret) 
				{
					return ExeError(ExePipe,_T("'goto space'  lost event"));
				}

				return ExeError(ExePipe,_T("goto space event time out"));
			}
			ExePipe.SetID(RETURN_WAIT); 
			return true;

		}
		else if(MsgID == MSG_SPACE_CATALOG)
		{	
			ePipeline& Letter = Msg.GetLetter(true);

			CBrainEvent EventInfo;
			bool ret = GetBrain()->GetBrainData()->GetEvent(GetEventID(),EventInfo,true);
			if (!ret) 
			{
				return ExeError(ExePipe,Format1024(_T("Error: (%I64ld)%s event lost"),m_ID,GetName().c_str()));
			}

			ePipeline Receiver;
			Receiver.PushInt(SYSTEM_SOURCE);
			Receiver.PushInt(DEFAULT_DIALOG);

			CMsg GuiMsg(SYSTEM_SOURCE,Receiver,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);

			ePipeline Cmd(TO_SPACE_VIEW::ID);
			Cmd.PushInt(TO_SPACE_VIEW::CATALOG);
			Cmd<<Letter;

			GuiMsg.GetLetter(false).PushPipe(Cmd);

			int64 SourceID = m_Param.GetID();
			if (SourceID==0) //向所有用户广播此信息
			{
				GetBrain()->GetBrainData()->SendMsgToGUI(GuiMsg,-1);
			}else{ //向指定对话发送此信息
				GetBrain()->GetBrainData()->SendMsgToGUI(SourceID,GuiMsg);
			}

			ExePipe.SetID(RETURN_NORMAL); 
			Dialog->SetTaskState(TASK_RUN);
			return true;
		}else if (MsgID == MSG_SPACE_ACCESS_DENIED)
		{
			ePipeline& Letter = Msg.GetLetter(true);
			int64     SpaceID = Letter.PopInt();
			tstring   Tip     = Letter.PopString();

			CBrainEvent EventInfo;
			bool ret = GetBrain()->GetBrainData()->GetEvent(GetEventID(),EventInfo,true);
			if (!ret) 
			{
				return ExeError(ExePipe,Format1024(_T("Error: (%I64ld)%s event lost"),m_ID,GetName().c_str()));
			}

			int64 SourceID = m_Param.GetID();
			ePipeline Receiver;
			Receiver.PushInt(SourceID);
			Receiver.PushInt(DEFAULT_DIALOG);

			CMsg GuiMsg(SYSTEM_SOURCE,Receiver,MSG_BRAIN_TO_GUI,DEFAULT_DIALOG,0);
			ePipeline Cmd(TO_SPACE_VIEW::ID);
			Cmd.PushInt(TO_SPACE_VIEW::ACESS_DENIED);
			Cmd<<Letter;

			GuiMsg.GetLetter(false).PushPipe(Cmd);


			if (SourceID==0) //向所有用户广播此信息
			{
				GetBrain()->GetBrainData()->SendMsgToGUI(GuiMsg,-1);
			}else{ //向指定对话发送此信息
				GetBrain()->GetBrainData()->SendMsgToGUI(SourceID,GuiMsg);
			}

			ePipeline& OldExePipe = EventInfo.m_ClientExePipe;

			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);

			ExePipe.SetID(RETURN_NORMAL); 
			Dialog->SetTaskState(TASK_RUN);
			return true;
		}

		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if (ret == RETURN_DIRECTLY)
		{
			return true;
		}
	}

	if(!m_Param.HasTypeAB(PARAM_TYPE1(TYPE_PIPELINE))){
		return ExeError(ExePipe,_T("param error"));
	}


	CLinker Linker;
	GetBrain()->GetLinker(SPACE_SOURCE,Linker);

	if(!Linker.IsValid()){
		return ExeError(ExePipe,_T("spaceport disconnect"));
	}

	UpdateEventID();

	int64 EventID = GetEventID();
	GetBrain()->GetBrainData()->RegisterEvent(EventID,EventID,ExePipe,LocalAddress,TIME_SEC,true,MSG_ROBOT_GOTO_SPACE,false); 

	ePipeline* Address = (ePipeline*)m_Param.GetData(0);

	CMsg rMsg(Dialog->m_SourceID,DEFAULT_DIALOG,MSG_ROBOT_GOTO_SPACE,DEFAULT_DIALOG,EventID);
	ePipeline& rLetter = rMsg.GetLetter(false);

	rLetter.PushPipe(*Address);  //空地址意味着要求获得根空间目录
	Linker().PushMsgToSend(rMsg);

	ExePipe.SetID(RETURN_WAIT);
	return true;
}

//////////////////////////////////////////////////////////////////////////

CBrainRobotRequestElt::CBrainRobotRequestElt(int64 ID,tstring Name,ePipeline& Param)
:CElement(ID,Name.c_str()),m_State(INIT)
{
	m_SpaceEventID = Param.PopInt();
	m_Caller       = Param.PopString();
	m_Right        = Param.PopString();
	if (m_Right==_T("task"))
	{
		if(Param.GetDataType(0) == TYPE_STRING){
			m_LogicText = Param.PopString();
		}	
	}
}

CBrainRobotRequestElt::~CBrainRobotRequestElt(){


}

//关闭对话分两个阶段完成：
//1)关闭子对话 2）收到子对话关闭信息后，修改事件地址，退出任务后由系统对话关闭自身
void CBrainRobotRequestElt::StopRequest(CLogicDialog* Dialog,ePipeline& ExePipe,tstring tip,bool bClose){

	CLogicDialog* SpaceDlg = GetBrain()->GetBrainData()->GetDialog(SPACE_SOURCE,DEFAULT_DIALOG);

	SpaceDlg->SaveDialogItem(tip,_T("System"));

	CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(SPACE_SOURCE,GetEventID());

	if(bClose)
		m_State = CLOSE;
	else
		m_State = USER_STOP;

	//如果子对话在执行任务，则向其发出停止执行信息，对方把执行结果反馈回来时再关闭本对话
	if(EventDlg){
		if(EventDlg->GetTaskState()!=TASK_STOP){
			CMsg rMsg(SYSTEM_SOURCE,GetEventID(),MSG_FROM_BRAIN,DEFAULT_DIALOG,0);
			ePipeline& rLetter = rMsg.GetLetter(false);
			rLetter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
			rLetter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_STOP);
			GetBrain()->PushNerveMsg(rMsg,false,false);
			return;
		}
	}

	//没有执行则直接停止本对话
	ExePipe.Break();
	Dialog->SetTaskState(TASK_STOP);

}
bool  CBrainRobotRequestElt::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){
	
	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);
	
	if(!Msg.IsReaded()){
		MsgProcState ret= EltMsgProc(Dialog,IT_SELF,Msg,ExePipe,LocalAddress);
		if (ret == RETURN_DIRECTLY)
		{
			return true;
		}
	}

	assert(m_State==INIT);
	{
		m_State = QUERY;

		tstring tip = Format1024(_T("%s request a conversation. right = '%s'"),m_Caller.c_str(),m_Right.c_str());
		Dialog->SaveDialogItem(tip,_T("System"),0);

		tstring DialogInfo;
		if (m_Right == _T("free"))
		{
			DialogInfo = Format1024(_T("%s request a conversation. right = '%s'\nif you agree that mean the robot can require you to do any thing that he want.\n\nplease input 'yes' to permit, others to refuse."),m_Caller.c_str(),m_Right.c_str());
		}
		else if (m_Right == _T("manual"))
		{
			DialogInfo = Format1024(_T("%s request a conversation. right = '%s'\nif you agree that mean you will input the command manually when he requiring .\n\nplease input 'yes' to permit, others to refuse."),m_Caller.c_str(),m_Right.c_str());
		}else if (m_Right == _T("chat"))
		{
			DialogInfo = Format1024(_T("%s request a chat.\nplease input 'yes' to permit, others to refuse."),m_Caller.c_str());
		}
		else if(m_Right == _T("task")){
			CLogicThread* Think = Dialog->GetThink();
			Think->ThinkProc(Dialog,0,m_LogicText,false,0);	
			bool ret = Think->CanBeExecute(Dialog);
			if(!ret){
				tstring error  = Format1024(_T("can't understand the logic: %s"),m_LogicText.c_str());	
				CLinker Linker;
				GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);
				GetBrain()->FeedbackError(Linker,m_SpaceEventID,error,NULL);
				return true;
			}

			Dialog->ResetThink();		
			DialogInfo = Format1024(_T("%s request a conversation. right = '%s'\n\ntask description: %s\n\nplease input 'yes' to permit, others to refuse."),m_Caller.c_str(),m_LogicText.c_str(),m_Right.c_str());
		}else {
			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);
			tstring error = Format1024(_T("the %s of request is invalid"),m_Right.c_str());
			GetBrain()->FeedbackError(Linker,m_SpaceEventID,error,NULL);
			return true;
		}

		UpdateEventID();
		CLogicDialog* Dlg = Dialog->StartEventDialog(GetEventID(),_T("request"),TASK_OUT_THINK,ExePipe,LocalAddress,TIME_SEC,true,true,true,0);
		if(!Dlg){		
			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);
			tstring error = Format1024(_T("the %s request starting failed"),m_Right.c_str());
			GetBrain()->FeedbackError(Linker,m_SpaceEventID,error,NULL);
			return true;
		}

		Dlg->SaveDialogItem(DialogInfo,_T("System"),0);

		//更改事件地址，这样能本地终止任务时会收到通知
		CBrainEvent  EventInfo;
		bool ret = GetBrain()->GetBrainData()->GetEvent(Dialog->m_DialogID,EventInfo,false);
		assert(ret);

		EventInfo.m_ClientAddress = LocalAddress;
		EventInfo.m_ClientEventID = GetEventID();

		GetBrain()->GetBrainData()->ModifyEvent(Dialog->m_DialogID,EventInfo);
	} 
	
	return true;
}

MsgProcState CBrainRobotRequestElt::EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	MsgProcState ret; 
	int64 MsgID = Msg.GetMsgID();

	switch(MsgID){
	case MSG_TASK_RESULT:
		{
			ret = OnEltTaskResult(Dialog,IT_SELF,Msg,ExePipe,LocalAddress);
		}
		break;
	case MSG_EXECUTE_REQUEST:
		{
			ret = OnEltExecuteRequest(Dialog,IT_SELF,Msg,ExePipe,LocalAddress);
		}
		break;
	case MSG_REQUEST_RUNTIME:
		{
			ret = OnEltRequestRuntime(Dialog,IT_SELF,Msg,ExePipe,LocalAddress);
		}
		break;
	case MSG_CLOSE_REQUEST:
		{
			ret = OnEltCloseRequest(Dialog,IT_SELF,Msg,ExePipe,LocalAddress);
		}
		break;
	case MSG_EVENT_TICK:
		{
			//检查空间链接是否有效
			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);
			if(!Linker.IsValid()){
				StopRequest(Dialog,ExePipe,_T("the request stoped because of the space link breaked"),true);
			}
		}
	default:
		ret = CElement::EltMsgProc(Dialog,IT_SELF,Msg,ExePipe,LocalAddress);

	}
	return ret;
}


MsgProcState CBrainRobotRequestElt::OnEltTaskResult(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	ePipeline& Letter = Msg.GetLetter(true);
	ePipeline OldExePipe;

	ePipeline* NewExePipe = (ePipeline*)Letter.GetData(0);


	//请求是否同意对话
	if(m_State == QUERY){

		if(!ExePipe.IsAlive()|| !NewExePipe->IsAlive()){
			CMsg NewMsg(SYSTEM_SOURCE,m_SpaceEventID,MSG_CLOSE_REQUEST,DEFAULT_DIALOG,0);
			CLinker Linker;
			GetBrain()->GetLinker(SPACE_SOURCE,Linker);
			if (Linker.IsValid())
			{
				Linker().PushMsgToSend(NewMsg);
			}
			StopRequest(Dialog,ExePipe,Format1024(_T("you stoped %s's %s request"),m_Caller.c_str(),m_Right.c_str()),true);
			return RETURN_DIRECTLY;
		}
		
		int64 ThinkID = NewExePipe->PopInt();
		CLogicThread* Think = GetBrain()->GetBrainData()->GetLogicThread(ThinkID);
		assert(Think);
		tstring Text = Think->GetUserInput();
		Text = TriToken(Text);

		if(Text==_T("yes")|| Text==_T("yes;")){
			Dialog->SaveDialogItem(_T("yes"),Dialog->m_SourceName,0);

			Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);	

			UpdateEventID();

			CLogicDialog* Dlg = NULL;
	
			tstring DialogInfo;
			if(m_Right == _T("chat")){
				DialogInfo = _T("now you can chat.");
				Dlg = Dialog->StartEventDialog(GetEventID(),m_Right,TASK_OUT_THINK,ExePipe,LocalAddress,TIME_SEC,true,true,false,0);
			}
			else{
				DialogInfo = _T("now waiting for request");
				Dlg = Dialog->StartEventDialog(GetEventID(),m_Right,TASK_OUT_DEFAULT,ExePipe,LocalAddress,TIME_SEC,false,false,false,0);
			}
			if(!Dlg){
				ExeError(ExePipe,_T("create event dialog fail"));
				return RETURN_DIRECTLY;	
			}
			Dlg->SaveDialogItem(DialogInfo,_T("System"),0);

			CMsg rMsg(SPACE_SOURCE,m_SpaceEventID,MSG_REQUEST_RUNTIME,DEFAULT_DIALOG,GetEventID());
			ePipeline& rLetter = rMsg.GetLetter(false);
			rLetter.PushInt(REQUEST_ACCEPTED);
			rLetter.PushInt(m_SpaceEventID);  //as Instance ID

			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);

			if(Linker.IsValid()){
				Linker().PushMsgToSend(rMsg);
			}

			assert(ExePipe.GetID()== RETURN_WAIT);

			m_State = EXECUTION;


		}else{//不同意

			CMsg NewMsg(SYSTEM_SOURCE,m_SpaceEventID,MSG_CLOSE_REQUEST,DEFAULT_DIALOG,0);
			CLinker Linker;
			GetBrain()->GetLinker(SPACE_SOURCE,Linker);
			if (Linker.IsValid())
			{
				Linker().PushMsgToSend(NewMsg);
			}

			StopRequest(Dialog,ExePipe,Format1024(_T("you stoped %s's %s request"),m_Caller.c_str(),m_Right.c_str()),true);
		}
	}
	else if (m_State == EXECUTION)  //for execute result				
	{
		if(!ExePipe.IsAlive()|| !NewExePipe->IsAlive()){
			CMsg NewMsg(SYSTEM_SOURCE,m_SpaceEventID,MSG_CLOSE_REQUEST,DEFAULT_DIALOG,0);
			CLinker Linker;
			GetBrain()->GetLinker(SPACE_SOURCE,Linker);
			if (Linker.IsValid())
			{
				Linker().PushMsgToSend(NewMsg);
			}
			StopRequest(Dialog,ExePipe,Format1024(_T("you stoped %s's %s request"),m_Caller.c_str(),m_Right.c_str()),true);
			return RETURN_DIRECTLY;
		}

		if(m_Right == _T("chat")){  //chat
			
			CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());
			assert(EventDlg);

			int64 ThinkID = NewExePipe->PopInt();
			assert(ThinkID!=0);

			CLogicThread* Think = GetBrain()->GetBrainData()->GetLogicThread(ThinkID);
			assert(Think);
			tstring Text = Think->GetUserInput();
			Text = TriToken(Text);

			EventDlg->SaveDialogItem(Text,GetBrain()->GetName(),0);
			EventDlg->ResetThink();

			CMsg rMsg(SYSTEM_SOURCE,m_SpaceEventID,MSG_REQUEST_RUNTIME,DEFAULT_DIALOG,0);
			ePipeline& rLetter = rMsg.GetLetter(false);
			rLetter.PushInt(REQUEST_OUTPUT);
		
			ePipeline Info;
			Info.PushString(Text);

			rLetter.PushPipe(Info);
			ExePipe.Clear();

			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);

			if(Linker.IsValid()){
				Linker().PushMsgToSend(rMsg);
			}
		}else{
			CMsg rMsg(SPACE_SOURCE,DEFAULT_DIALOG,MSG_TASK_FEEDBACK,DEFAULT_DIALOG,m_SpaceEventID);
			ePipeline& rLetter = rMsg.GetLetter(false);
			rLetter.PushPipe(*NewExePipe);

			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);

			if(Linker.IsValid()){
				Linker().PushMsgToSend(rMsg);
			}

			ExePipe.Clear();

			CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());
			assert(EventDlg);
			
			EventDlg->ResetThink();
			EventDlg->ResetTask();
			EventDlg->EnableInput(false);
		}

		ExePipe.SetID(RETURN_WAIT);

	}else if (m_State == USER_STOP)  //应对方要求只停止当前任务，但不关闭对话（可以接受新的任务）
	{
		
		ExePipe.Clear();

		CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());
		assert(EventDlg);
		EventDlg->EnableInput(false);
		EventDlg->ResetThink();
		EventDlg->ResetTask();
		ExePipe.SetID(RETURN_WAIT);

	}else if (m_State == CLOSE)
	{
		ePipeline OldExePipe;
		Dialog->CloseEventDialog(GetEventID(),OldExePipe,ExePipe);	

		//修改事件客户地址，返回后系统将自动删除对话
		CBrainEvent  EventInfo;
		GetBrain()->GetBrainData()->GetEvent(Dialog->m_DialogID,EventInfo,false);

		EventInfo.m_ClientAddress.Clear();
		EventInfo.m_ClientAddress.PushInt(DEFAULT_DIALOG);
		EventInfo.m_ClientEventID = 0;

		GetBrain()->GetBrainData()->ModifyEvent(Dialog->m_DialogID,EventInfo);

		ExePipe.Break();
		Dialog->SetTaskState(TASK_STOP);
	}
	return RETURN_DIRECTLY;
}

MsgProcState CBrainRobotRequestElt::OnEltExecuteRequest(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){
	int64 SourceID = Msg.GetSourceID();
	int64 SenderID = Msg.GetSenderID();

	assert(m_SpaceEventID == SenderID);

	Dialog->RuntimeOutput(m_ID,_T("the '%s' execute request"),m_Caller.c_str());
	
	m_State = EXECUTION;

	if (m_Right == _T("free"))
	{
		ePipeline& Letter = Msg.GetLetter(true);
		ePipeline* RemoteExePipe = (ePipeline*)Letter.GetData(0);

		if(!RemoteExePipe->HasTypeAB(PARAM_TYPE1(TYPE_STRING))){
			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SourceID,Linker);

			GetBrain()->FeedbackError(Linker,SenderID,_T("no finding the logic while executing the free request"),RemoteExePipe);
			return RETURN_DIRECTLY;

		}
		tstring LogicText = RemoteExePipe->PopString();

		CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());

		CLogicThread*  Think = EventDlg->GetThink();	
		Think->ThinkProc(Dialog,0,LogicText,false,0);	
		bool ret = Think->CanBeExecute(EventDlg);
		if(!ret){
			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SourceID,Linker);
			GetBrain()->FeedbackError(Linker,SenderID,Format1024(_T("can't understand the logic: %s"),LogicText.c_str()),&ExePipe);
			return RETURN_DIRECTLY;
		}else{

			EventDlg->Think2TaskList();

			CMsg rMsg(SYSTEM_SOURCE,EventDlg->m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,0);
			ePipeline& Letter = rMsg.GetLetter(false);
			Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
			Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE);

			Letter.PushPipe(*RemoteExePipe);

			GetBrain()->PushNerveMsg(rMsg,false,false);

			assert(ExePipe.GetID() == RETURN_WAIT);
		}

	}else if (m_Right == _T("manual"))
	{
		ePipeline& Letter = Msg.GetLetter(true);
		ePipeline* RemoteExePipe = (ePipeline*)Letter.GetData(0);

		if(!RemoteExePipe->HasTypeAB(PARAM_TYPE1(TYPE_STRING))){
			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SourceID,Linker);

			GetBrain()->FeedbackError(Linker,SenderID,_T("no finding the task description while executing the task request"),RemoteExePipe);
			return RETURN_DIRECTLY;

		}
		tstring Text = RemoteExePipe->PopString();

		CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());
		assert(EventDlg);
//		EventDlg->m_ExePipe << *RemoteExePipe;
		EventDlg->SaveDialogItem(Text,m_Caller,0);
		EventDlg->SetThinkState(THINK_IDLE);

		assert(ExePipe.GetID() == RETURN_WAIT);

	}else if (m_Right == _T("task"))
	{
		ePipeline& Letter = Msg.GetLetter(true);
		ePipeline* RemoteExePipe = (ePipeline*)Letter.GetData(0);
	
		CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());
		assert(EventDlg);
		EventDlg->SaveDialogItem(m_LogicText,m_Caller,0);

		CLogicThread*  Think = EventDlg->GetThink();	
		Think->ThinkProc(EventDlg,0,m_LogicText,false,0);

		bool ret = Think->CanBeExecute(EventDlg);
		if(!ret){

			CLinker Linker;
			GetBrain()->GetSuperiorLinkerList()->GetLinker(SPACE_SOURCE,Linker);

			GetBrain()->FeedbackError(Linker,m_SpaceEventID,Format1024(_T("can't understand the task: %s"),m_LogicText.c_str()),RemoteExePipe);

			return RETURN_DIRECTLY;
		}else{
			EventDlg->Think2TaskList();
			CMsg Msg(SYSTEM_SOURCE,EventDlg->m_DialogID,MSG_FROM_BRAIN,DEFAULT_DIALOG,0);
			ePipeline& Letter = Msg.GetLetter(false);
			Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::ID);
			Letter.PushInt(TO_BRAIN_MSG::TASK_CONTROL::CMD_EXE);
			Letter.PushPipe(*RemoteExePipe);

			GetBrain()->PushNerveMsg(Msg,false,false);
		}

	}else if (m_Right == _T("chat"))
	{
		tstring Greeting = _T("now you are chatting mode");

		CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());
		assert(EventDlg);
		EventDlg->SaveDialogItem(Greeting,_T("System"),0);
		EventDlg->SetThinkState(THINK_IDLE);
	}
	return RETURN_DIRECTLY;

}
MsgProcState CBrainRobotRequestElt::OnEltRequestRuntime(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	ePipeline& Letter = Msg.GetLetter(true);
	
	int64 MsgID = Msg.GetMsgID();

	if(!Letter.HasTypeAB(PARAM_TYPE1(TYPE_INT))){
		Dialog->RuntimeOutput(m_ID,_T("Receive a msg(%s) that include invalid data"),GetBrain()->MsgID2Str(MsgID).c_str());
		return RETURN_DIRECTLY;
	}
	CLogicDialog* EventDlg = GetBrain()->GetBrainData()->GetDialog(Dialog->m_SourceID,GetEventID());
	REQUEST_RUNTIME_CODE code = (REQUEST_RUNTIME_CODE)Letter.PopInt();
	if (code == REQUEST_OUTPUT)
	{
		ePipeline* InfoPipe = (ePipeline*)Letter.GetData(0);

		if(m_Right == _T("chat")){
			tstring Text = InfoPipe->PopString();
			EventDlg->SaveDialogItem(Text,m_Caller,0);
		}else{
			CPipeView PipeView(InfoPipe);
			tstring RuntimeInfo = PipeView.GetString();
			EventDlg->RuntimeOutput(m_ID,RuntimeInfo);
		}
	}else if(code == REQUEST_STOP){

		if(m_Right == _T("chat")){
			tstring Text = Format1024(_T("%s stop chatting"),m_Caller.c_str());
			EventDlg->SaveDialogItem(Text,_T("System"),0);
			EventDlg->EnableInput(false);
		}else{

			tstring tip = Format1024(_T("%s  stoped the request"),m_Caller.c_str());
			StopRequest(Dialog,ExePipe,tip,false);
			EventDlg->SaveDialogItem(_T("user stoped the request"),m_Caller);	
		}
	} 
	return RETURN_DIRECTLY;
}



MsgProcState CBrainRobotRequestElt::OnEltCloseRequest(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress){

	tstring tip = Format1024(_T(" %s has been stoped the resquest "),m_Caller.c_str());
	StopRequest(Dialog,ExePipe,tip,true);
	return RETURN_DIRECTLY;
}
