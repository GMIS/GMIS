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


namespace TO_BRAIN_MSG{
	const uint32 GUI_IO_INPUTING		= 1001;
	const uint32 GUI_IO_INPUTED			= 1002;
	const uint32 GUI_SET_FOUCUS_DIALOG	= 1003;

	namespace TASK_CONTROL{
		const uint32 ID                 = 1004;
		const uint32 CMD_EXE            = 100;
		const uint32 CMD_RUN            = 101;
		const uint32 CMD_STOP           = 102;  
		const uint32 CMD_PAUSE          = 103; 
		const uint32 CMD_SET_BREAKPOINT    = 104;
		const uint32 CMD_DEBUG_STEP     = 105; 
		//const uint32 CMD_WAITED_FEEDBACK= 106;
		//const uint32 CMD_DEBUG          = 107;
	}

	const uint32 GUI_GET_DEBUG_ITEM     = 1005;
	const uint32 GUI_GET_THINK_RESULT   = 1006;
	const uint32 GUI_GET_ANALYSE_RESULT = 1007;
	const uint32 GUI_CLEAR_DIALOG_OUTPUT= 1008; 
	const uint32 GUI_GET_FIND_REUSLT    = 1009;
	const uint32 GUI_CLEAR_THINK        = 1010;
	const uint32 GUI_CONNECT_TO         = 1011;
	const uint32 GUI_DISCONNECT         = 1012;

	namespace GUI_LOGIC_OPERATE{
		const uint32 ID					= 1013;
		const uint32  DEL_LOGIC			= 20;
		const uint32  CLEAR_LOGIC		= 30;  
	}
	namespace GUI_OBJECT_OPERATE{
		const uint32   ID				= 1014;
		const uint32   SELECT_OBJECT    = 10;
		const uint32   DELETE_OBJECT    = 20;
		const uint32   CLR_OBJECT       = 30;
	}
	const uint32  GUI_MEMORY_OPERATE     =1015;
	const uint32  GUI_SET_LOG_FLAG       =1016;
	const uint32  GUI_GET_DIALOG_MORE_ITME=1017;
}



//大脑输出至GUI界面的命令

namespace TO_STATUS_VIEW{
	const uint32 ID					= 100;
	const uint32 SET_TEXT			= 101;
	const uint32 IO_LIGHT_FLASH		= 102;
	const uint32 SET_PROGRESS		= 103;
	const uint32 PFM_MSG_UPDATE		= 104;
	const uint32 PFM_THREAD_UPDATE	= 105;
};

namespace TO_RUNTIME_VIEW{
	const uint32 ID					= 10000;
	const uint32 OUT_PIPE_INFO		= 10001;
	const uint32 CLEAR_PIPE_INFO	= 10002;
	const uint32 OUT_FORECAST_INFO	= 10003;
	const uint32 CLEAR_FORECAST		= 10004;
	const uint32 TASK_TOOLBAR	    = 10005;
	const uint32 TASK_STATE			= 10006;
	const uint32 SET_EDIT			= 10007;
	const uint32 CONTINUE_EDIT		= 10008;
	const uint32 SET_WORKMODE		= 10009;
	const uint32 OUT_THINK_RESULT	= 10010;
	const uint32 OUT_ANALYSE_RESULT	= 10011;
};

namespace TO_SYSTEM_VIEW{
	const uint32 ID					= 20000;
	const uint32 SET_CUR_DIALOG		= 20001;
	const uint32 CONNECT_START		= 20002;
	const uint32 CONNECT_STATE		= 20003;

};

namespace TO_DATA_VIEW{
	const uint32 ID					= 30000;
	const uint32 CLEAR				= 30001;
};
namespace TO_OBJECT_VIEW{
	const uint32 ID					= 40000;
};
namespace TO_DEBUG_VIEW{
	const uint32 ID					= 50000;
	const uint32 CLEAR				= 50001;
};

namespace TO_LOGIC_VIEW{
	const uint32 ID					= 60000;
};

namespace TO_DIALOG_VIEW{
	const uint32 ID					= 70000;
	const uint32 ADD_ITEM			= 70001;
	const uint32 DEL_ITEM			= 70002;	
	const uint32 CLEAR				= 70003;
	const uint32 INIT				= 70004;
	const uint32 ENABLE_ITEM        = 70005;

};

namespace TO_SPACE_VIEW{
	const uint32 ID					= 80000;
	const uint32 INIT				= 80001;
	const uint32 CATALOG			= 80002;
	const uint32 ADD_ITEM			= 80003;
	const uint32 DEL_ITEM			= 80004;
	const uint32 SEND_OBJECT		= 80005;
	const uint32 ACESS_DENIED		= 80006;
	const uint32 ADDRESSBAR_STATE	= 80007;
};

namespace TO_HISTORY_VIEW{
	const uint32 ID					= 90000;
	const uint32 CLEAR				= 90001;
	const uint32 ADD_ITEM			= 90002;
	const uint32 ADD_MORE_ITEM		= 90003;
};

namespace TO_FIND_VIEW{
	const uint32 ID					= 100000;
	const uint32 CLEAR				= 100001;
};





#endif