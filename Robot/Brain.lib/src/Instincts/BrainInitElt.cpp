#pragma warning (disable: 4786)

#include "BrainInitElt.h"
#include "..\InstinctDefine.h"
#include "..\LogicDialog.h"
struct NewWord{ 
	int64         PartOfSpeech;
	const TCHAR*  Word1;
	const TCHAR*  Word2;
};
 

//初始化基本单词本能，其中 
NewWord _WordList[]={    //note: 增减项目必须修改初始化单词数目
	
	//{"d",      MEMORY_VERB},      //测试用
	//{"i",        MEMORY_NOUN},
	//{"de",      MEMORY_VERB},      //测试用
	//{"in",        MEMORY_NOUN},
	
	{MEMORY_VERB,_T("define"), _T("定义")},
	{MEMORY_NOUN,_T("int"),_T("整数")},
	{MEMORY_NOUN,_T("float"),_T("小数")},
	{MEMORY_NOUN,_T("string"),_T("字符串")},
	
	{MEMORY_LOGIC_THEN,_T("then"),_T("然后")},
	{MEMORY_LOGIC_AND,_T("and"),_T("同时")},
	
	{MEMORY_VERB,_T("use"),_T("使用")},
	{MEMORY_NOUN,_T("operator"),_T("操作符")},
	
	{MEMORY_NOUN,_T("resistor"),_T("电阻")},
	{MEMORY_NOUN,_T("rs"), _T("电阻")},
	
	{MEMORY_NOUN,_T("inductor"),_T("电感")},
	{MEMORY_NOUN,_T("id"),_T("电感")},
	
	{MEMORY_NOUN,_T("capacitor"),_T("电容")},
	{MEMORY_NOUN,_T("cp"),_T("电容")},
	
	{MEMORY_NOUN,_T("diode"),_T("二极管")},
	{MEMORY_NOUN,_T("dd"),_T("二极管")},
	
	{MEMORY_VERB,_T("reference"),_T("引用")},

	{MEMORY_VERB,_T("set"),_T("设置")},
	{MEMORY_NOUN,_T("label"),_T("标签")},
	{MEMORY_VERB,_T("goto"),_T("转向")},
	{MEMORY_VERB,_T("view"),_T("查看")}, 
	{MEMORY_NOUN,_T("pipe"),_T("管道")}, 
	
	{MEMORY_VERB,_T("input"),_T("输入")},
	{MEMORY_NOUN,_T("text"),_T("文本")},
	{MEMORY_NOUN,_T("num"),_T("数字")},
	{MEMORY_VERB,_T("wait"),_T("等待")},
	{MEMORY_NOUN,_T("second"),_T("秒")},
	
	{MEMORY_VERB,_T("create"),_T("制造")},
	{MEMORY_NOUN,_T("table"),_T("数据表")},
	{MEMORY_VERB,_T("import"),_T("导入")},
	{MEMORY_VERB,_T("export"),_T("导出")},

	{MEMORY_VERB,_T("focus"),_T("关注")},
	{MEMORY_VERB,_T("insert"),_T("插入")},
	{MEMORY_VERB,_T("modify"),_T("修改")},
	{MEMORY_NOUN,_T("data"),_T("数据")},
	{MEMORY_VERB,_T("get"),_T("得到")},
	{MEMORY_VERB,_T("remove"),_T("移除")},
	{MEMORY_NOUN,_T("size"),_T("大小")},
	{MEMORY_VERB,_T("close"),_T("关闭")},
	
	{MEMORY_NOUN,_T("logic"),_T("逻辑")},
	{MEMORY_VERB,_T("name"),_T("名字")},
	
	{MEMORY_NOUN,_T("date"),_T("日期")},

	{MEMORY_VERB,_T("start"),_T("启动")},
	{MEMORY_NOUN,_T("object"),_T("物体")},
	
	//{MEMORY_VERB,_T("ask"),_T("请求")},
	//{MEMORY_NOUN,_T("help"),_T("帮助")},
	
	{MEMORY_VERB,_T("think"),_T("思考")},
	
	{MEMORY_VERB,_T("run"),_T("运行")},
	{MEMORY_VERB,_T("debug"),_T("调试")},
	{MEMORY_VERB,_T("stop"),_T("停止")},
	{MEMORY_VERB,_T("pause"),_T("暂停")},
	{MEMORY_VERB,_T("step"),_T("单步")},
	{MEMORY_VERB,_T("test"),_T("测试")},

	{MEMORY_NOUN,_T("dialog"),_T("对话")},
	
	{MEMORY_VERB,_T("learn"),_T("学习")}, 
	{MEMORY_NOUN,_T("token"),_T("符号")},
	{MEMORY_NOUN,_T("pronoun"),_T("代词")},
	{MEMORY_NOUN,_T("adjective"),_T("形容词")},
	{MEMORY_NOUN,_T("numeral"),_T("数字")},
	{MEMORY_NOUN,_T("verb"),_T("动词")},
	{MEMORY_NOUN,_T("adverb"),_T("副词")},
	{MEMORY_NOUN,_T("article"),_T("冠词")},
	{MEMORY_NOUN,_T("preposition"),_T("介词")},
	{MEMORY_NOUN,_T("conjunction"),_T("连词")},
	{MEMORY_NOUN,_T("interjection"),_T("感叹词")},
	{MEMORY_NOUN,_T("noun"),_T("名词")},
	{MEMORY_NOUN,_T("text"),_T("文本")},
	
	{MEMORY_NOUN,_T("action"),_T("行为")},
	{MEMORY_VERB,_T("find"),_T("搜索")},
	{MEMORY_NOUN,_T("time"),_T("时间")},
	{MEMORY_NOUN,_T("end"),_T("终点")},
	{MEMORY_NOUN,_T("pricision"),_T("精度")},	
	{MEMORY_VERB,_T("output"),_T("输出")}, 
	{MEMORY_NOUN,_T("info"),_T("信息")}, 
	{MEMORY_NOUN,_T("document"),_T("文档")}, 
	{MEMORY_NOUN,_T("expectation"),_T("预期")},

	{MEMORY_NOUN,_T("account"),_T("帐号")}, 
	{MEMORY_NOUN,_T("memory"),_T("记忆")}, 
	{MEMORY_VERB,_T("delete"),_T("删除")}, 
	{MEMORY_NOUN,_T("address"),_T("地址")}, 
	{MEMORY_NOUN,_T("node"),_T("节点")}, 
	{MEMORY_NOUN,_T("focus"),_T("焦点")}, 
	{MEMORY_NOUN,_T("breakpoint"),_T("断点")}, 	
};


//初始化本能行为
struct InitInstinct{ 
	int64         InstinctID;
	const TCHAR*  Cmd;
}; 

InitInstinct _InstinctList[]={
	
	//通用命令
	//{"d i",   INSTINCT_DEFINE_INT32},
	//{"de in",   INSTINCT_DEFINE_INT32},
	//{INSTINCT_DEFINE_BYTE,_T("define byte")},
	//{INSTINCT_DEFINE_INT32,_T("define int32") },
	//{INSTINCT_DEFINE_UINT32,_T("define uint32") },
	{INSTINCT_DEFINE_INT64,_T("define int")},
	//{INSTINCT_DEFINE_UINT64,_T("define uint64")},
	//{INSTINCT_DEFINE_FLOAT32,_T("define float32")},
	{INSTINCT_DEFINE_FLOAT64,_T("define float")},
	{INSTINCT_DEFINE_STRING,_T("define string") },
	
	{INSTINCT_USE_OPERATOR,_T("use operator")},
	
	{INSTINCT_USE_RESISTOR,_T("use rs")},
	{INSTINCT_USE_INDUCTOR,_T("use id")},
	{INSTINCT_USE_CAPACITOR,_T("use cp")},
	{INSTINCT_USE_DIODE,_T("use dd")},
	
	{INSTINCT_USE_RESISTOR,_T("use Resistor")},
	{INSTINCT_USE_INDUCTOR,_T("use inductor")},
	{INSTINCT_USE_CAPACITOR,_T("use capacitor")},
	{INSTINCT_USE_DIODE,_T("use Diode")},
	
	{INSTINCT_REFERENCE_CAPACITOR,_T("reference capacitor")},
	{INSTINCT_REFERENCE_INDUCTOR,_T("reference inductor")},
	{INSTINCT_REFERENCE_CAPACITOR,_T("reference cp")},
	{INSTINCT_REFERENCE_INDUCTOR,_T("reference id") },
	
	
	{INSTINCT_SET_LABEL,_T("set  label ")},
	{INSTINCT_GOTO_LABEL,_T("goto label ")},
	{INSTINCT_VIEW_PIPE,_T("view pipe")},
	
	
	{INSTINCT_INPUT_TEXT,_T("input text")},
	{INSTINCT_INPUT_NUM,_T("input num")},
	{INSTINCT_WAIT_SECOND,_T("wait second")},
	
	
	{INSTINCT_CREATE_MEMORY,_T("create memory")},
	{INSTINCT_FOCUS_MEMORY,_T("focus  memory")},
	{INSTINCT_SET_MEMORY_ADDRESS,_T("set memory address")},
	{INSTINCT_GET_MEMORY_ADDRESS,_T("get memory address")},
	{INSTINCT_CREATE_MEMORY_NODE,_T("create memory node")},
	{INSTINCT_IMPORT_MEMORY,_T("import memory")},
	{INSTINCT_EXPORT_MEMORY,_T("export memory")},
	{INSTINCT_GET_MEMORY,_T("get memory")},
	{INSTINCT_INSERT_MEMORY,_T("insert memory")},
	{INSTINCT_MODIFY_MEMORY,_T("modify memory")},
	{INSTINCT_REMOVE_MEMORY,_T("remove memory")},
	{INSTINCT_GET_MEMORY_SIZE,_T("get memory size")},
	{INSTINCT_CLOSE_MEMORY,_T("close memory")},
	{INSTINCT_GET_MEMORY_FOCUS,_T("get memory focus")},
	
	{INSTINCT_USE_LOGIC,_T("use logic")},
	{INSTINCT_FOCUS_LOGIC,_T("focus logic")},
	{INSTINCT_NAME_LOGIC,_T("name logic")},
	{INSTINCT_INSERT_LOGIC,_T("insert logic")},
	{INSTINCT_REMOVE_LOGIC,_T("remove logic")},
	{INSTINCT_SET_LOGIC_ADDRESS,_T("set logic address")},
	{INSTINCT_SET_LOGIC_BREAKPOINT,_T("set logic breakpoint")},

	{INSTINCT_GET_DATE,_T("get date")},
	{INSTINCT_GET_TIME,_T("get time")},

	{INSTINCT_OUTPUT_INFO,_T("output  info")},
	{INSTINCT_TEST_EXPECTATION,_T("test  expectation")},

	{INSTINCT_START_OBJECT,_T("start object")},
	{INSTINCT_FOCUS_OBJECT,_T("focus object")},
	{INSTINCT_NAME_OBJECT,_T("name object")},
	{INSTINCT_USE_OBJECT,_T("use object")},
	{INSTINCT_CLOSE_OBJECT,_T("close object")},
	{INSTINCT_GET_OBJECT_DOC,_T("get object document")},
	
	
	//内部命令
	{INSTINCT_THINK_LOGIC,_T("think logic")},
	
	{INSTINCT_RUN_TASK,_T("run")},
	{INSTINCT_DEBUG_TASK,_T("debug")},
	{INSTINCT_STOP_TASK,_T("stop")},
	{INSTINCT_PAUSE_TASK,_T("pause")},
	{INSTINCT_STEP_TASK,_T("step")},
	{INSTINCT_GOTO_TASK,_T("goto")},	
	{INSTINCT_TEST_TASK,_T("test")},	

	{INSTINCT_CLOSE_DIALOG,_T("close dialog")},

	{INSTINCT_CREATE_ACCOUNT,_T("create account")},
	{INSTINCT_DELETE_ACCOUNT,_T("delete account")},

	//内部非独立命令
	{INSTINCT_LEARN_TOKEN,_T("learn token")},
	{INSTINCT_LEARN_PRONOUN,_T("learn pronoun")},
	{INSTINCT_LEARN_ADJECTIVE,_T("learn adjective")},
	{INSTINCT_LEARN_NUMERAL,_T("learn numeral")},
	{INSTINCT_LEARN_VERB,_T("learn verb")},
	{INSTINCT_LEARN_ADVERB,_T("learn adverb")},
	{INSTINCT_LEARN_ARTICLE,_T("learn article")},
	{INSTINCT_LEARN_PREPOSITION,_T("learn preposition")},
	{INSTINCT_LEARN_CONJUNCTION,_T("learn conjunction")},
	{INSTINCT_LEARN_INTERJECTION,_T("learn interjection")},
	{INSTINCT_LEARN_NOUN,_T("learn noun")},
	{INSTINCT_LEARN_TEXT,_T("learn text")},
	
	{INSTINCT_LEARN_LOGIC,_T("learn logic")},
	{INSTINCT_LEARN_OBJECT,_T("learn object")},
	{INSTINCT_LEARN_ACTION,_T("learn action")},
	
	//{INSTINCT_FIND_SET_STARTTIME,_T("set find start time")},
	//{INSTINCT_FIND_SET_ENDTIME,_T("set find end time")},
	//{INSTINCT_FIND_SET_PRICISION,_T("set find pricision")},
	
	{INSTINCT_FIND,_T("find") },
	{INSTINCT_FIND_LOGIC,_T("find logic")},
	{INSTINCT_FIND_OBJECT,_T("find object") },
	//{INSTINCT_USE_ARM,_T("use arm")}
	
};
		

CBrainInitElt::CBrainInitElt(int64 ID)
:CElement(ID,_T("Brain Init"))
{
	m_EventType = 0;
	m_Index = 0;
	        
};

CBrainInitElt::~CBrainInitElt(){
	
}


bool  CBrainInitElt::Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg){ 
	
	CLocalInfoAuto LocalInfoAuto(Dialog,this,LocalAddress);
	
	if(!Msg.IsReaded()){
		int32 ChildIndex = IT_SELF;
		MsgProcState ret = CElement::EltMsgProc(Dialog,ChildIndex,Msg,ExePipe,LocalAddress);
		if(ret == RETURN_DIRECTLY){
			return true;
		}
	}
	try{
		
		if (m_EventType == 0)
		{
			bool ret = CheckWord(Dialog,ExePipe,LocalAddress);
			if(!ret){
				return false;
			}
		}
		
		if (m_EventType == 1)
		{
			bool ret = CheckInstinct(Dialog,ExePipe,LocalAddress);
			if (!ret)
			{
				return false;
			}
		}

		if(ExePipe.IsAlive()){
			Dialog->m_Brain->OutSysInfo(_T("Checking memory....OK"));
		}
	}catch(CppSQLite3Exception &e ){ 
		
		AnsiString s = e.errorMessage();
		tstring ws = UTF8toWS(s);

		tstring str = Format1024(_T("WARNING: Brain Database error>>%s"),ws.c_str());
		Dialog->RuntimeOutput(str);
		return false;
	}
	catch(...){
		Dialog->RuntimeOutput(_T("WARNING: Unkown error lead to fail when check instinct "));
		return false;
	} 
	
	return true;
};


bool CBrainInitElt::CheckWord(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress){

	CLogicDialog* SysDialog = Dialog->m_Brain->GetBrainData()->GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
	assert(SysDialog);

	CLogicThread& Think = *Dialog->GetThink();	
	
	Think.ClearAnalyse();

    if (m_Index == 0)
    {
		Dialog->RuntimeOutput(_T("Checking base word..."));
    }

 		
	int n = (sizeof(_WordList)/sizeof(_WordList[0]));
        
	try{	

		for(m_Index;m_Index<n;m_Index++)
		{
			int64 State = ExePipe.GetID();

			if( !ExePipe.IsAlive() ){
				Dialog->SetTaskState(TASK_STOP);
				return false;
			}else if(State == RETURN_BREAK){
				Dialog->NotifyPause(ExePipe,LocalAddress);
				return true;
			}

			NewWord& r = _WordList[m_Index];
			tstring s = Format1024(_T("Checking  word =\"%s\""),r.Word1);

			CNotifyDialogState nf(NOTIFY_BRAIN_INIT);
			nf.PushInt(INIT_PROGRESS);
			nf.PushString(s);
			nf.PushInt(m_Index*100/n);
			nf.Notify(Dialog);

			tstring word = r.Word1;

			int64 ID = Think.CheckWord(word,r.PartOfSpeech,MEANING_SENSE_OK);
			if(ID==0){
				SysDialog->RuntimeOutput(0,_T("Learning word \"%s\" "),r.Word1);										

				ID = Think.LearnWord(word,r.PartOfSpeech,MEANING_SENSE_OK);

				if(ID==0){
					SysDialog->RuntimeOutput(0,_T("WARNING: Learn word [%s]  fail"),r.Word1);
				}else{
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
		if (m_Index == n)
		{
			m_EventType = 1; //准备执行下一阶段任务
			m_Index  = 0;
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
	CLogicDialog* SysDialog = Dialog->m_Brain->GetBrainData()->GetDialog(SYSTEM_SOURCE,DEFAULT_DIALOG);
	assert(SysDialog);

	CLogicThread& Think = *Dialog->GetThink();	
	Think.ClearAnalyse();

	int n = (sizeof(_InstinctList)/sizeof(_InstinctList[0]));
	
	if (m_Index==0)
	{
		Dialog->RuntimeOutput(_T("Checking  instinct..."));
	}

	try{	

		for(m_Index; m_Index<n; m_Index++)
		{

			int64 State = ExePipe.GetID();

			if( !ExePipe.IsAlive() ){
				Dialog->SetTaskState(TASK_STOP);
				return false;
			}else if(State == RETURN_BREAK){

				Dialog->NotifyPause(ExePipe,LocalAddress);
				return true;
			}

			InitInstinct& r = _InstinctList[m_Index];


			tstring s = Format1024(_T("Checking  instinct =\"%s\""),r.Cmd);

			CNotifyDialogState nf(NOTIFY_BRAIN_INIT);
			nf.PushInt(INIT_PROGRESS);
			nf.PushString(s);
			nf.PushInt(m_Index*100/n);
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

		if(m_Index == n){
			m_Index = 0;
			m_EventType = 0;

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