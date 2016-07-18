/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _LOGICDIALOG_H__
#define _LOGICDIALOG_H__

//#include "SpaceMsgList.h"
#include "Element.h"
#include "SpaceMutex.h"
#include "LogicThread.h"
#include "LogicTask.h"
#include <vector>
#include "BrainMemory.h"
#include "Brain.h"

class CElementCell;
class CLocalLogicCell;
class CLogicTask;
class CObjectData;

#define   FDeque deque<int64>

enum THINK_STATE{
		THINK_IDLE, 	//准备新的理解
		THINK_ON,	    //对话理解阶段
		THINK_WAIT,     //等待继续输入
		THINK_ERROR     //思考错误
};
enum TASK_STATE{
		TASK_DELELTE = -1,  //表明所属Dialog将被删除
		TASK_STOP,			//任务未执行
		TASK_COMPILE,		//逻辑编译阶段
		TASK_COMPILE_ERROR, //编译错误
		TASK_RUN,			//逻辑正在执行
		TASK_PAUSE,			//逻辑执行暂停 
		TASK_WAIT       	//等待外部任务返回
};

enum WORK_MODE{
		WORK_TASK,
		WORK_THINK,
		WORK_DEBUG,
		WORK_CHAT,
		WORK_LEARN,
		WORK_TEST
};

enum FindTypeExpected {
	    FIND_ALL,
		FIND_OBJECT,
		FIND_PEOPLE,
		FIND_TEXT,
		FIND_LOGIC, 
		FIND_COMMAND
}; 

struct CNameUser{
	tstring  m_LogicName;
	int64	 m_TaskID;
	int64 	 m_InstanceID;
};

class CNameList{
public:
	map<tstring, CNameUser>   m_NameList;
public:
	bool	HasName(CLogicDialog* Dialog,tstring& Name);
	bool	HasMemoryInstance(CLogicDialog* Dialog,tstring& Name);
	void	RegisterNameByLogic(tstring& Name,tstring& LogicName);

	void	RegisterNameByTask(tstring& Name,int64 TaskID,int64 InstanceID);
	void	UnregisterNameByTask(tstring& Name);

	int64   GetInstanceID(tstring& Name);
	tstring GetInstanceName(int64 InstanceID);

	Energy*	ToEnergy(); 
	bool    FromEnergy(Energy* E);

};

class CLogicDialog  
{
	friend				CBrain;
	friend				CLocalInfoAuto;

public:
	int64				m_SourceID;       //对话源ID 
	int64				m_DialogID;       //对话ID

	CBrain*				m_Brain;
	int64               m_ParentDialogID; //逻辑父对话ID

	tstring             m_SourceName;
	tstring				m_DialogName;     

    DIALOG_TYPE			m_DialogType;     //对话类型
	TASK_OUT_TYPE       m_TaskOutType;    //任务类型
 
	tstring             m_CompileError;    //编译如果不能进行，存储错误提示
	tstring             m_ThinkError;

	int64               m_TaskDialogCount; //记录生成子对话数目,用来做识别ID

protected:
    
	static CSpaceMutex  m_DialogMutex;

	WORK_MODE           m_WorkMode;

    THINK_STATE			m_ThinkState;
	TASK_STATE			m_TaskState;


	map<int64,int64>    m_PauseEventList;    //map<PauseID,EventID> ,如果PauseID == EventID 则是其它子事件

	int64               m_FocusPauseItemID;  //=0表示无效
	int64               m_ControlDialogID;

	ePipeline           m_ExePipe;            //执行管道

	CLockPipe           m_ElementMsgList;     //待处理信息列表
	int32				m_SysProcCounter;     //统计有多少个控制线程，避免控制执行时删除任务

public:	
	int64         		m_ThinkID;	
	int64    			m_TaskID;

	tstring             m_CurLogicName;
	int64               m_CurTaskMsgID;       //正在处理的任务信息ID

public:
    /*
	ePipeline{ 历史对话数据
		ePipeline{  m_ID=时间戳， m_Label=发言者名字 
			int32   信息状态
			string  信息文本
		} 
		...
		ePipeline{
		}			
	}
	*/
    ePipeline			m_DialogHistory;
	ePipeline			m_RuntimeOutput;
	ePipeline           m_SearchOutput;   //暂存当前一页的搜索结果，m_ID=0表示是否为最后一页，=1表示还有后续

	bool				m_bEditValid;
    tstring				m_EditText;       //暂存输出窗口未完成的编辑文本
	tstring				m_StatusText;

	int64               m_LastDebugTimeStamp;	
	ePipeline			m_LogicItemTree;  //用于DEBUG显示，ID=最近更改时间戳

public:
	int64                         m_ObjectFocus;
	int64                         m_MemoryFocus;
	tstring                       m_LogicFocus;
	
	vector<CLocalLogicCell*>      m_LogicList;
	vector<CElementCell*>         m_CapaList;
	vector<CElementCell*>         m_InduList;
    vector<CObjectData*>          m_ObjectList;

	map<int64, ePipeline>         m_ObjectInstanceList;
	map<int64, ePipeline>		  m_MemoryInstanceList;
    map<int64, CElement*>         m_LogicInstanceList;

	CNameList                     m_NamedObjectList;   //被命名引用的物体
    CNameList                     m_NamedMemoryList;

	ePipeline                     m_MemoryAddress;     //当前焦点记忆的操作地址
	ePipeline                     m_LogicAddress;      //当前任务逻辑的操作地址

	deque<CSentence*>			  m_TaskList;

public:
	CLogicDialog(CBrain* Frame,int64 SourceID,int64 DialogID,int64 ParentDialogID,tstring SourceName,tstring DialogName,
		       DIALOG_TYPE Type,TASK_OUT_TYPE TaskType);

	virtual ~CLogicDialog();

	void Reset(CBrain* Frame,int64 SourceID,int64 DialogID,int64 ParentDialogID,tstring SourceName,tstring DialogName,
		       DIALOG_TYPE Type,TASK_OUT_TYPE TaskType);

	virtual void Do(CMsg& Msg); //只能Brain调用,并且可能多线程调用

	void NotifyPause(ePipeline& ExePipe,ePipeline& Address);
    
	void StopPause(int64 PauseID,int64 CmdID);

	void  SetFocusPauseItemID(int64 ID)
	{
		m_FocusPauseItemID = ID;
	}
		
	int64 GetFocusPauseItemID();
 
	void GetPauseIDList(ePipeline& List);

	bool IsPaused();

	void ClosePauseDialog(int64 PauseEventID);

	void  PushEltMsg(CMsg& Msg,bool bUrgence){
		if(bUrgence){
			m_ElementMsgList.PushUrgence(Msg.Release());
		}else{
			m_ElementMsgList.Push(Msg.Release());
		}
		
	};

	int64 GetControlEventID(){
		return m_ControlDialogID;
	}
	bool CompileTask();

	Energy*  ToEnergy(); 
	bool     FromEnergy(Energy* E);
public:
	CLogicDialog& operator << (CLogicDialog& Dialog); //把逻辑环境转交给子对话，让其去执行具体任务

	void  ResetThink();
	void  ResetTask();

	void  ClearEltMsgList();

	void  Think2TaskList();

	bool  HasTask(){
		return m_TaskList.size()>0;
	}

	CSentence* PopTask();

	void SetWorkMode(WORK_MODE Mode);
    WORK_MODE GetWorkMode();

	CLogicThread* GetThink();
	CLogicTask* GetTask();

	//一个任务在执行逻辑任务过程中，另一个或多个线程可能也在执行系统任务，这里给出计数从而保证删除任务不会出错
	int32 GetSysProcNum();

	THINK_STATE SetThinkState(THINK_STATE State);
	THINK_STATE GetThinkState();

	TASK_STATE  SetTaskState(TASK_STATE State); //返回旧状态
	TASK_STATE  GetTaskState();



	//等候10秒让任务暂停
	void			NotifySuspendTask();
	TASK_OUT_TYPE	GetTaskType();

	
	void FeedbackToBrain();

	void RuntimeOutput(tstring s);
	void RuntimeOutput(int64 MassID,tstring s);
	void RuntimeOutput(INT64 MassID,TCHAR* Format, ...);

	
	CLogicDialog* StartEventDialog(int64 EventID,tstring DialogName,tstring DialogText,TASK_OUT_TYPE OutType,ePipeline& ClientExePipe,ePipeline& ClientAddress,int64 EventInterval,bool bFocus,bool bEditValid,bool bOnce);
    void		  CloseEventDialog(int64 EventID,ePipeline& OldExePipe,ePipeline& ExePipe);

public:

	//State用于今后可能出现的需求，比如信息正在接收
    ePipeline&  SaveReceiveItem(tstring Info,int32 State);
	ePipeline&  SaveSendItem(tstring Info, int32 State);
	

	//当前Sentence,编译完一个子句通知一声，用于显示进度之类
	//ClauseNum为本Sentence含有的句子子句总数，n为已经编译完成的数
	void SentenceCompileProgress(int32 ClauseNum, int32 Completed);

public:
	/*除了登记逻辑，还处理逻辑里的元件和其它逻辑的引用或生成
	*/
	CLocalLogicCell* FindLogic(const tstring& Name);
	bool RegisterLogic(CLogicTask* Task);
    void DeleteLogic(const tstring& Name);

	void ReferenceLogic(const tstring& scrName,const tstring& refName,CLocalLogicCell* WhoRef,bool Add = true); //add==false dereference;

	CElementCell* FindCapacitor(const tstring& Name);
	void RegisterCapacitor(const tstring& Name, CLocalLogicCell* CreatedBy);
	void ReferenceCapacitor(const tstring& scrName,const tstring& refName,  CLocalLogicCell* WhoRef,bool Add = true); //add==false dereference;
	void DeleteCapacitor(const tstring& Name);

	CElementCell* FindInductor(const tstring& Name);
	void RegisterInductor(const tstring& Name, CLocalLogicCell* CreatedBy);
	void ReferenceInductor(const tstring& Name,const tstring& refName, CLocalLogicCell* WhoRef ,bool Add = true); //add==false dereference;
	void DeleteInductor(const tstring& Name);

	CObjectData* FindObject(int64 ObjectID);
    int32 FindObject(tstring Name,vector<CObjectData>& ObjectList);
	void RegisterObject(ePipeline& ObjectData);
	void DeleteObject(ePipeline& ObjectData);

    ePipeline*  FindTempMemory(int64 InstanceID);
	CElement*   FindLogicInstance(int64 InstanceID);
    ePipeline*  FindObjectInstance(int64 InstanceID);


	void AddObjectInstance(int64 InstanceID,ePipeline& Pipe);
	void AddMemoryInstance(int64 InstanceID, ePipeline& Pipe);

	void CloseObjectInstance(int64 InstanceID);
	void CloseMemoryInstance(int64 InstanceID);

/*
	CVirtualObject* FindRefObject(tstring Name);
	void DeleteObjectRef(tstring ObjectName);
	void DeleteObjectRef(int64 ObjectInstanceID);
*/
	void ClearLogicSence(); //清除临时逻辑

	void ClearObject();

	/*打包输出本地逻辑数据及引用，用于更新焦点对话
	  List输出格式：
         ePipeline0=Item
		 ePipeline1=Item ref
		 ...
	*/
	void GetLocalLogicData(ePipeline& List);
	void GetLocalCapaData(ePipeline& List);
	void GetLocalInduData(ePipeline& List);
	void GetLocalObjectData(ePipeline& List);

	void GetMemoryInstanceData(ePipeline& List);

	void SetBreakPoint(ePipeline& Path,BOOL bEnable);

	//处理信息搜索
	/////////////////////////////////////////////////////////
public:
	
#define TEXT_RESULT   0
#define LOGIC_RESULT  1
#define OBJECT_RESULT 2
	
	struct _FindResult{
		uint8  m_Type;   //0=text 1=Logic 2=Object
		int64  m_ID;  //空间值
		int64  m_Value;
		_FindResult(int8 type, int64 ID,int64 Value):m_Type(type),m_ID(ID),m_Value(Value){};
	};
	
	uint32                m_ItemNumPerPage;    //每次显示多少个结果 default = 10
	
	FindTypeExpected      m_FindType;
	uint32                m_Interval; //通过判断两个意义间隔时间来判断是否有关系，缺省= 5秒
	deque<int64>          m_FindSeedList;
	vector<_FindResult>   m_FindResultList;
	
	//MeaingList保存意义空间的存储ID，同时在deque里保存此空间下所有结尾空间的空间ID（记忆时间戳）
	void _FindTokenAnd(deque<int64>& DestMeaningList, map<int64,FDeque>& SrcMeaningList,map<int64,FDeque>& ResultMeaningList,bool first);
	void _FindTokenOr(deque<int64>& DestMeaningList, map<int64,FDeque>& SrcMeaningList,map<int64,FDeque>& ResultMeaningList);
	void _FindTokenNot(deque<int64>& DestMeaningList, map<int64,FDeque>& SrcMeaningList,map<int64,FDeque>& ResultMeaningList);
    
	//处理搜索运算
	void _FindMemoryRoom(CLogicThread* Think,map<int64,FDeque>* DestTokenList, deque<int64>* ResultRoomList);
	
	
	//MeaningList[空间存储ID]=意义值, 把结果发送给FindView
    void ProcessMeaning(CLogicThread* Think,int64 RoomID,int64 RoomValue,int64 RoomType,FindTypeExpected FindType = FIND_ALL); 
	
    void  OutputFindResult(int8 type,int64 RoomID,int64 RoomValue); 
	
	
	//把ID的意义翻译成文本然后按一定格式保存在Item里
	void  PrintText(CLogicThread* Think,ePipeline& SearchResult, int32 n,int64 RoomID);
	void  PrintLogic(CLogicThread* Think,ePipeline& SearchResult,int32 n,int64 RoomID,int64 RoomValue);
	void  PrintObject(CLogicThread* Think,ePipeline& SearchResult,int32 n,int64 RoomID,int64 RoomValue);	
	

	//没找到返回0，否则返回找到的种子
	void  FindFirst(tstring& text,FindTypeExpected FindType = FIND_ALL);
	void  FindContinue(CLogicThread* Think,uint32 Index, ePipeline& SearchResult); 
	void  SetFindCellSize(int32 size){  m_Interval = size;};


//任务对话的信息处理
//////////////////////////////////////////////////////////////////////////

	void DialogMsgProc(ePipeline* ExePipe,CMsg& Msg);		
		void OnBroadcaseMsg(ePipeline* ExePipe,CMsg& Msg);  //对方BroadcastMsg()发来的信息
		void OnMsgFromBrain(ePipeline* ExePipe,CMsg& Msg);    //处理用户界面发来的信息
		void OnBrainTextInputing(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainTextInputEnd(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainGetMoreLog(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainSetFocusDialog(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainTaskControl(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainGetDebugItem(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainLogicOperate(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainObjectOperate(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainMemoryOperate(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainGetThinkResult( ePipeline* ExePipe,CMsg& Msg);
		void OnBrainGetAnalyseResult(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainClearDialogOutput(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainGetFindResult(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainClearThink(ePipeline* ExePipe,CMsg& Msg);		
		void OnBrainConnectTo(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainDisconnect(ePipeline* ExePipe,CMsg& Msg);
		void OnBrainSetLogFlag(ePipeline* ExePipe,CMsg& Msg);
		void OnTaskResult(ePipeline* ExePipe,CMsg& Msg);
		void OnEventTick(ePipeline* ExePipe,CMsg& Msg);

public:
	class AutoSysProcCounter
	{
	private:
		int  m_Counter;
		CLogicDialog* m_Dialog;
	public:
		AutoSysProcCounter(CLogicDialog* Dialog):m_Dialog(Dialog)
		{
			bool ret = m_Dialog->m_DialogMutex.AcquireThis(Dialog);
			assert(ret);
			m_Dialog->m_SysProcCounter++;
			m_Counter = m_Dialog->m_SysProcCounter;
			m_Dialog->m_DialogMutex.ReleaseThis(Dialog);
		};
		~AutoSysProcCounter(){
			bool ret = m_Dialog->m_DialogMutex.AcquireThis(m_Dialog);
			assert(ret);
			m_Dialog->m_SysProcCounter--;
			m_Dialog->m_DialogMutex.ReleaseThis(m_Dialog);
		};

		int GetProcNum(){
			return m_Counter;
		}
	};

	friend class AutoSysProcCounter;

};

#endif // !defined(_LOGICDIALOG_H__)
