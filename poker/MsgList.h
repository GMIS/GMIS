#ifndef  _MSGLIST_H_
#define  _MSGLIST_H_

#include "Msg.h"
#include "LinkerPipe.h"

using namespace ABSTRACT;

#define  LOCAL_DIALOG    1
#define  LOCAL_ADDRESS   2 
#define  LOCAL_INPUTWIN  3
#define  LOCAL_LINKERWIN 4
#define  LOCAL_LOGWIN    5

//#define  MSG_WHO_ARE_YOU           MSG_BASE+1  已经定义
#define MSG_LINKER_NOTIFY            MSG_BASE+2
#define MSG_LINKER_ERROR			 MSG_BASE+3
#define MSG_BROADCAST_MSG            MSG_BASE+4 
#define MSG_MODEL_MSG                MSG_BASE+5  //自身组织发来的信息
#define MSG_INIT_BRAIN               MSG_BASE+6

#define MSG_GUI_TO_BRAIN             MSG_BASE+7


#define MSG_TIME_OUT                 MSG_BASE+8 
#define MSG_SET_TIMER                MSG_BASE+9
#define MSG_CONNECT_TO               MSG_BASE+11
#define MSG_I_AM                     MSG_BASE+12 
#define MSG_CLOSE_DIALOG             MSG_BASE+13
#define MSG_CONNECT_OK               MSG_BASE+14
#define MSG_SET_FOCUS_DIALOG         MSG_BASE+15
#define MSG_GET_MORE_LOG             MSG_BASE+16
#define MSG_STOP_TASK                MSG_BASE+17
//#define MSG_LINKER_RECEIVE_RESUME    MSG_BASE+18
#define MSG_GET_LINKERLIST           MSG_BASE+19
#define MSG_SEND_LINKERLIST          MSG_BASE+20
#define MSG_SEND_LOG                 MSG_BASE+21
#define MSG_SEND_MORE_LOG            MSG_BASE+22

#define MSG_POKER_GET_INIT           MSG_BASE+23
#define MSG_POKER_SET_INIT           MSG_BASE+24
#define MSG_POKER_GET_EVALUATE       MSG_BASE+25
#define MSG_POKER_EVA_RESULT         MSG_BASE+26
#define MSG_POKER_OUTPUT             MSG_BASE+27


#define MSG_TASK_COMPILE_RESULT      MSG_BASE+100001

#define MSG_TEXT_INPUTING            MSG_BASE+500001
#define MSG_TEXT_INPUTEND            MSG_BASE+500002
#define MSG_CHAT                     MSG_BASE+500003


#define MSG_TASK_RUN                 MSG_BASE+600001
#define MSG_TASK_PAUSE               MSG_BASE+600002
#define MSG_TASK_OK                  MSG_BASE+600003
#define MSG_TASK_EXE                 MSG_BASE+600004
#define MSG_TASK_STOP                MSG_BASE+600005

struct _MSG_TEXT{ 
	int64  msg_id;
	const TCHAR*  msg_text;
}; 

#ifdef _UNICODE
#define _MSG2TEXT(ID) {ID,L#ID}
#else
#define _MSG2TEXT(ID) {ID,#ID}
#endif

static _MSG_TEXT _MsgTextList[]={                
	    _MSG2TEXT(MSG_WHO_ARE_YOU),
	    _MSG2TEXT(MSG_LINKER_NOTIFY),
		_MSG2TEXT(MSG_LINKER_ERROR),
        _MSG2TEXT(MSG_BROADCAST_MSG),
        _MSG2TEXT(MSG_MODEL_MSG),
		_MSG2TEXT(MSG_INIT_BRAIN),

		_MSG2TEXT(MSG_TIME_OUT),
		_MSG2TEXT(MSG_SET_TIMER),
		_MSG2TEXT(MSG_CONNECT_TO),
		_MSG2TEXT(MSG_I_AM),
		_MSG2TEXT(MSG_CLOSE_DIALOG),
		_MSG2TEXT(MSG_CONNECT_OK),
		_MSG2TEXT(MSG_SET_FOCUS_DIALOG),
        _MSG2TEXT(MSG_GET_MORE_LOG),
        _MSG2TEXT(MSG_STOP_TASK),

        _MSG2TEXT(MSG_GET_LINKERLIST),
        _MSG2TEXT(MSG_SEND_LINKERLIST),
        _MSG2TEXT(MSG_SEND_LOG),
        _MSG2TEXT(MSG_SEND_MORE_LOG),

		_MSG2TEXT(MSG_POKER_GET_INIT),
		_MSG2TEXT(MSG_POKER_SET_INIT),
        _MSG2TEXT(MSG_POKER_GET_EVALUATE),
		_MSG2TEXT(MSG_POKER_EVA_RESULT),
		_MSG2TEXT(MSG_POKER_OUTPUT),

		_MSG2TEXT(MSG_TASK_COMPILE_RESULT),

		_MSG2TEXT(MSG_TEXT_INPUTING),
		_MSG2TEXT(MSG_TEXT_INPUTEND),
		_MSG2TEXT(MSG_CHAT),
		_MSG2TEXT(MSG_TASK_RUN),
		_MSG2TEXT(MSG_TASK_PAUSE),
		_MSG2TEXT(MSG_TASK_OK),
		_MSG2TEXT(MSG_TASK_EXE)
};


static _MSG_TEXT _LinkerCodeStrList[]={   
	_MSG2TEXT(LINKER_COMPILE_ERROR),
	_MSG2TEXT(LINKER_RECEIVE_RESUME),
	_MSG2TEXT(LINKER_INVALID_ADDRESS),
	_MSG2TEXT(LINKER_ILLEGAL_MSG),
	_MSG2TEXT(LINKER_PUSH_MSG),
	_MSG2TEXT(LINKER_MSG_SENDED),
	_MSG2TEXT(LINKER_MSG_RECEIVED),
	_MSG2TEXT(LINKER_RECEIVE_STEP),
	_MSG2TEXT(LINKER_SEND_STEP),
};


tstring MsgID2Str(int64 MsgID);
tstring LinkerCode2Str(int32 CodeID);


class CPipeView{
    ePipeline  m_Result;
public:
	CPipeView(ePipeline& Pipe){
		PrintPipe(Pipe);
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
	//把Pipe的所有数据翻译成string存储在m_Result里
	void PrintPipe(ePipeline& Pipe,int32 TabNum=0);
};


tstring TriToken(tstring& Token);

#endif