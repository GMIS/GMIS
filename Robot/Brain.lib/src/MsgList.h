/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef  _MSGLIST_H_
#define  _MSGLIST_H_

#include "SpaceMsgList.h"
#include "LinkerPipe.h"

using namespace ABSTRACT;


#define  LOCAL_DIALOG    1
#define  LOCAL_ADDRESS   2 
#define  LOCAL_INPUTWIN  3
#define  LOCAL_LINKERWIN 4
#define  LOCAL_LOGWIN    5


#define MSG_INIT_BRAIN               MSG_BASE+1000000000000

#define MSG_GUI_TO_BRAIN             MSG_INIT_BRAIN+100
#define MSG_BRAIN_TO_GUI             MSG_INIT_BRAIN+101
#define MSG_FROM_BRAIN               MSG_INIT_BRAIN+102
#define MSG_TASK_RESULT              MSG_INIT_BRAIN+103
#define MSG_GET_LINKERLIST           MSG_INIT_BRAIN+104
#define MSG_SEND_LINKERLIST          MSG_INIT_BRAIN+105

//暂时不用，以后考虑作为统一信息接口
#define MSG_APP_RUN                  MSG_INIT_BRAIN+2001
#define MSG_APP_ADD                  MSG_INIT_BRAIN+2002
#define MSG_APP_DELETE               MSG_INIT_BRAIN+2003
#define MSG_APP_ONLINE               MSG_INIT_BRAIN+2004 
#define MSG_APP_RESET                MSG_INIT_BRAIN+2005
#define MSG_APP_PAUSE                MSG_INIT_BRAIN+2006

#define MSG_CLOSE_DIALOG             MSG_INIT_BRAIN+3001
#define MSG_SET_FOCUS_DIALOG         MSG_INIT_BRAIN+3002
#define MSG_GET_MORE_LOG             MSG_INIT_BRAIN+3003
#define MSG_STOP_TASK                MSG_INIT_BRAIN+3004

//Element信息
#define MSG_ELT_TASK_CTRL            MSG_INIT_BRAIN+4001
#define MSG_ELT_INSERT_LOGIC         MSG_INIT_BRAIN+4002
#define MSG_ELT_REMOVE_LOGIC         MSG_INIT_BRAIN+4003



static _MSG_TEXT _MsgTextList[]={                
	_MSG2TEXT(MSG_INIT_BRAIN),
		
        _MSG2TEXT(MSG_GUI_TO_BRAIN),
        _MSG2TEXT(MSG_BRAIN_TO_GUI),
		_MSG2TEXT(MSG_FROM_BRAIN),
		_MSG2TEXT(MSG_TASK_RESULT),
        _MSG2TEXT(MSG_GET_LINKERLIST),
        _MSG2TEXT(MSG_SEND_LINKERLIST),
		
		_MSG2TEXT(MSG_APP_RUN),
        _MSG2TEXT(MSG_APP_ADD),
		_MSG2TEXT(MSG_APP_DELETE),
		_MSG2TEXT(MSG_APP_ONLINE),
        _MSG2TEXT(MSG_APP_RESET),
		_MSG2TEXT(MSG_APP_PAUSE),
		
		_MSG2TEXT(MSG_CLOSE_DIALOG),
		_MSG2TEXT(MSG_SET_FOCUS_DIALOG),
        _MSG2TEXT(MSG_GET_MORE_LOG),
        _MSG2TEXT(MSG_STOP_TASK),
		
		_MSG2TEXT(MSG_ELT_TASK_CTRL),
		_MSG2TEXT(MSG_ELT_INSERT_LOGIC),
        _MSG2TEXT(MSG_ELT_REMOVE_LOGIC)
};



#endif