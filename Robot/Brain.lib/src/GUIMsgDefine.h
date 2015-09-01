/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _GUIMSGDEFINE_H_
#define _GUIMSGDEFINE_H_


#define THREAD_CENTRAL_NERVE  0
#define THREAD_NERVE          1

#define CENTRALNERVE_MSG_NUM  0
#define NERVE_MSG_NUM        1     



//以窗口分类，以行为为中心来设计消息，一个完整的信息可以组合这些行为

//GUI界面输出至大脑的信息
#define GUI_IO_INPUTING       1001
#define GUI_IO_INPUTED        1002
#define GUI_SET_FOUCUS_DIALOG 1003

#define GUI_TASK_CONTROL      1004
	#define CMD_EXE                100
	#define CMD_RUN                 101
	#define CMD_STOP                102  
	#define CMD_PAUSE               103 
	#define CMD_DEBUG_BREAK         104
	#define CMD_DEBUG_STEP          105 
	#define CMD_WAITED_FEEDBACK     106
	#define CMD_DEBUG               107

#define GUI_GET_DEBUG_ITEM        1005
#define GUI_GET_THINK_RESULT      1006
#define GUI_GET_ANALYSE_RESULT    1007
#define GUI_CLEAR_DIALOG_OUTPUT   1008 
#define GUI_GET_FIND_REUSLT       1009
#define GUI_CLEAR_THINK           1010

#define GUI_CONNECT_TO            1011
#define GUI_DISCONNECT            1012

#define GUI_LOGIC_OPERATE         1013
	#define DEL_LOGIC    20
	#define CLEAR_LOGIC  30  

#define GUI_OBJECT_OPERATE        1014
	#define  SELECT_OBJECT      10
	#define  DELETE_OBJECT      20
	#define  CLR_OBJECT         30

#define GUI_MEMORY_OPERATE        1015
#define GUI_SET_LOG_FLAG          1016

//大脑输出至GUI界面的命令
#define GUI_STATUS_SET_TEXT   3001
#define GUI_DIALOG_OUTPUT     3002
#define GUI_RUNTIME_OUTPUT    3003
#define GUI_FORECAST_OUTPUT   3004
#define GUI_ONE_LIGHT_FLASH   3005
#define GUI_TWO_LIGHT_FLASH   3006
#define GUI_CLEAR_OUTPUT      3007
	#define CLEAR_DIALOG      1
	#define CLEAR_RUNTIME     2
	#define CLEAR_FORECAST    3

#define GUI_LOGIC_OUTPUT      3008
	#define ADD_ITEM    101
	#define DEL_ITEM    102
	#define CLEAR_ITEM  103
#define GUI_SET_CUR_DIALOG    3009

#define GUI_STATUS_PROGRESS   3010
#define GUI_TASK_TOOL_BAR     3011

#define GUI_SPACE_OUTPUT      3013
	#define SPACE_CATALOG      101
	#define SPACE_ADDED        102
	#define SPACE_DELETED      103
	#define SPACE_SEND_OBJECT  104
	#define SPACE_ACESS_DENIED 105

#define GUI_OBJECT_OUTPUT     3014
	#define ADD_ITEM    101
	#define DEL_ITEM    102
	#define CLEAR_ITEM  103
    
#define GUI_CONNECT_STATE     3015
#define CON_START  100
#define CON_END    101 

#define GUI_VIEW_LAYOUT      3016

#define GUI_GET_DIALOG_MORE_ITME  3100
#define GUI_SET_DIALOG_MORE_ITEE  3101
#define GUI_SET_EIDT          3102
#define GUI_CONTINUE_EDIT     3103
#define GUI_SET_WORKMODE      3104

#define GUI_OUT_THINK_RESULT  3105
#define GUI_OUT_ANALYSE_RESULT 3106



	
#define GUI_LINKVIEW_OUTPUT   4001
	#define INIT_LIST         106   

#define GUI_DEBUGVIEW_OUTPUT   4002

#define GUI_FIND_OUTPUT       4004

#define GUI_MEMORY_OUTPUT     4005

#define GUI_TASK_STATE_UPDATE 5000
#define GUI_ADDRESSBAR_STATE  6000
#define GUI_PFM_MSG_UPDATE    7001
#define GUI_PFM_THREAD_UPDATE 7002


#endif