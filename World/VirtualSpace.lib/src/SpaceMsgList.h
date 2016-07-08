/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _SPACEMSGLIST_H_
#define _SPACEMSGLIST_H_

#include "LinkerPipe.h"


enum  DIALOG_TYPE{
		DIALOG_NULL,
		DIALOG_SYSTEM, //系统主对话
		DIALOG_MODEL,  //系统器官对话
		DIALOG_TASK,   //任务对话，也是主对话的子对话
		DIALOG_EVENT   //事件对话，通常也是任务对话的子对话	
};

enum {
		RETURN_NORMAL,             //0 正常执行返回
		RETURN_PIPE_BREAK,         //1 软中断，当前分支执行管道失效，通常在并联中被处理
		RETURN_DEBUG_PIPE_BREAK,   //2 RETURN_PIPE_BREAK的调试版
		RETURN_GOTO_LABEL,         //3 正常跳转
		RETURN_DEBUG_GOTO_LABEL,   //4 RETURN_GOTO_LABEL的调试版
		RETURN_WAIT,               //5 当逻辑委托第三方执行部分任务，自身返回等待,对于并联来说一个分支返回等待不影响其他分支执行
		RETURN_DEBUG_WAIT,         //6 RETURN_WAIT的调试版
		RETURN_BREAK,              //7 引发一个调试中断,中断后转换为RETURN_PAUSE
		RETURN_PAUSE,              //8 逻辑任务暂停返回，执行自动处于调试状态，用户可以选择单步或重新正常执行
		RETURN_ERROR,              //9 执行错误返回或者用户要求任务停止
		
};


enum REQUEST_ITEM{
//		REQUEST_END,
//		REQUEST_PAUSE,
//		REQUEST_DIALOG,
//		REQUEST_TRANS_TASK,
		REQUEST_EXE_LOGIC,
		REQUEST_START_OBJECT,
		REQUEST_USE_OBJECT,
		REQUEST_GET_OBJECT_DOC,
		REQUEST_CLOSE_OBJECT,
		REQUEST_OTHER_ROBOT,
		REQUEST_INSERT_LOGIC,
		REQUEST_REMOVE_LOGIC
};


#define MSG_CONNECT_TO               MSG_BASE+101
#define MSG_CONNECT_OK               MSG_BASE+102

#define MSG_I_AM                     MSG_BASE+201
#define MSG_EVENT_TICK               MSG_BASE+202 


#define MSG_TASK_FEEDBACK            MSG_BASE+301
#define MSG_TASK_REQUEST             MSG_BASE+302
#define MSG_OPERATOR_FEEDBACK        MSG_BASE+303        



#define MSG_ROBOT_GOTO_SPACE         MSG_BASE+1001
#define MSG_ROBOT_CREATE_SPACE       MSG_BASE+1002 
#define MSG_ROBOT_DEL_SPACE          MSG_BASE+1003	
#define MSG_ROBOT_GET_OBJECT         MSG_BASE+1004
#define MSG_ROBOT_EXPORT_OBJECT      MSG_BASE+1005

#define MSG_SPACE_CATALOG            MSG_BASE+2001 
#define MSG_SPACE_ADDED              MSG_BASE+2002
#define MSG_SPACE_DELETED            MSG_BASE+2003
#define MSG_SPACE_SEND_OBJECT        MSG_BASE+2004
#define MSG_SPACE_ACCESS_DENIED      MSG_BASE+2005

#define MSG_OBJECT_START             MSG_BASE+3001
#define MSG_OBJECT_RUN               MSG_BASE+3002
#define MSG_OBJECT_CLOSE             MSG_BASE+3003
#define MSG_OBJECT_FEEDBACK          MSG_BASE+3004
#define MSG_OBJECT_GET_DOC           MSG_BASE+3005
#define MSG_OBJECT_RUNTIME_OUT       MSG_BASE+3006

struct _MSG_TEXT{ 
	int64  msg_id;
	const TCHAR*  msg_text;
}; 

#ifdef _UNICODE
#define _MSG2TEXT(ID) {ID,L#ID}
#else
#define _MSG2TEXT(ID) {ID,#ID}
#endif

static _MSG_TEXT _SpaceMsgTextList[]={ 
		_MSG2TEXT(MSG_WHO_ARE_YOU),
        _MSG2TEXT(MSG_BROADCAST_MSG),

		_MSG2TEXT(MSG_CONNECT_TO),
		_MSG2TEXT(MSG_CONNECT_OK),

		_MSG2TEXT(MSG_I_AM),
		_MSG2TEXT(MSG_EVENT_TICK),

		_MSG2TEXT(MSG_TASK_FEEDBACK),
		_MSG2TEXT(MSG_TASK_REQUEST),
		_MSG2TEXT(MSG_OPERATOR_FEEDBACK),

		_MSG2TEXT(MSG_ROBOT_GOTO_SPACE),
		_MSG2TEXT(MSG_ROBOT_CREATE_SPACE),
		_MSG2TEXT(MSG_ROBOT_DEL_SPACE),
        _MSG2TEXT(MSG_ROBOT_GET_OBJECT),
		_MSG2TEXT(MSG_ROBOT_EXPORT_OBJECT),

		_MSG2TEXT(MSG_SPACE_CATALOG),
		_MSG2TEXT(MSG_SPACE_ADDED),
		_MSG2TEXT(MSG_SPACE_DELETED),
		_MSG2TEXT(MSG_SPACE_SEND_OBJECT),
		_MSG2TEXT(MSG_SPACE_ACCESS_DENIED),

		_MSG2TEXT(MSG_OBJECT_START),
		_MSG2TEXT(MSG_OBJECT_RUN),
		_MSG2TEXT(MSG_OBJECT_CLOSE),
		_MSG2TEXT(MSG_OBJECT_FEEDBACK),
		_MSG2TEXT(MSG_OBJECT_GET_DOC),
		_MSG2TEXT(MSG_OBJECT_RUNTIME_OUT)

};


static _MSG_TEXT _LinkerCodeStrList[]={   
        _MSG2TEXT(LINKER_BEGIN_ERROR_STATE),
		_MSG2TEXT(LINKER_END_ERROR_STATE),
		_MSG2TEXT(LINKER_INVALID_ADDRESS),
		_MSG2TEXT(LINKER_ILLEGAL_MSG),
		_MSG2TEXT(LINKER_PUSH_MSG),
		_MSG2TEXT(LINKER_MSG_SENDED),
		_MSG2TEXT(LINKER_MSG_RECEIVED),
		_MSG2TEXT(LINKER_RECEIVE_STEP),
		_MSG2TEXT(LINKER_SEND_STEP),
};


tstring LinkerCode2Str(int32 CodeID);


class CPipeView{
    ePipeline  m_Result;
public:
	CPipeView(ePipeline* Pipe){
		PrintPipe(*Pipe);
	};
	~CPipeView(){};
	
    ePipeline& GetPipe(){ return m_Result;};
	tstring    GetString(){
		tstring text;
		for (int i=0; i<m_Result.Size(); i++)
		{
			tstring* s = (tstring*)m_Result.GetData(i);
			text +=*s;
		}
		return text;	
	};
protected:
	void PrintPipe(ePipeline& Pipe,int32 TabNum=0);
};


tstring TriToken(tstring& Token);


#endif //_SPACEMSGLIST_H_