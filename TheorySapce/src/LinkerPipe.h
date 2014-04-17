/*
*author: ZhangHongBing(hongbing75@gmail.com)  
*  
* LinkerPipe封装了一个不依赖任何OS系统的传输协议。它的要点是：
*
*	1)所有信息可以用ePipeline格式表达,以ePipeline格式直接使用
*	2)一个ePipeline以字符串形式发出去，接收端再实时组装成ePipeline,对用户完全透明
*	3)同一个LinkerPipe两端支持区分本地不同的对话者。
*	4)LinkerPipe有宿主，收发状态包括结果会向宿主管道发信息，宿主只需负责处理此信息。
*	5)线程安全。
*	6)使用时，必须承继此类，然后重载实现具体的物理传输方式。
*/

#ifndef _LINKER_H__
#define _LINKER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <deque>
#include <map>
#include "TheorySpace.h"
#include "LockPipe.h"
#include "Msg.h"

#define LINKER_RESET     1
#define LINKER_FEEDBACK  2

#define RECEIVE_ERROR   0
#define RECEIVE_OK      1

#define NUMBER_LENGTH_LIMIT   20
#define DATA_LENGTH_LIMIT     10
#define ERROR_RESUME_LENGTH   5


#define ERROR_TYPE_PART       1
#define ERROR_LENGTH_PART     2
#define ERROR_ID_PART         3
#define ERROR_NAMELEN_PART    4
#define ERROR_NAME_PART       5
#define ERROR_OTHER_PARNT     6

namespace ABSTRACT{

//识别状态
#define LINKER_DEL      -2
#define	LINKER_BAN      -1  //禁止
#define LINKER_INVALID   0  //无效
#define LINKER_STRANGER  1
#define	LINKER_FRIEND    2

//输出LinkerPipe的工作状态
enum STATE_OUTPUT_LEVEL{ NORMAL_LEVEL, LIGHT_LEVEL,WEIGHT_LEVEL};

//输出分级:
 
//1 常规输出 NORMAL_LEVEL
#define LINKER_COMPILE_ERROR    101  //装配信息时发生的任何错误，进入错误接收状态
#define LINKER_RECEIVE_RESUME   102  //Linker接收状态恢复，退出错误接收状态
#define LINKER_INVALID_ADDRESS  103  //无法根据来信地址找到本地对应接收者
#define LINKER_ILLEGAL_MSG      104  //接收到不是一个合法格式信息

//2 轻度输出 LIGHT_LEVEL
#define LINKER_PUSH_MSG         200  //压入一个信息等待发送，并报告当前发送状态
#define LINKER_MSG_SENDED       201  //某个信息已经发送完毕
#define LINKER_MSG_RECEIVED     202  //接收到某个信息

//3 重度输出 WEIGHT_LEVEL
#define LINKER_RECEIVE_STEP     300  //报告接受进度并复制收到的数据,以每一个TYPE_PIPELINE数据类型为单位，不关心数据之间的关系，但数据之间的关系很容易被推导出
#define LINKER_SEND_STEP        301  //报告发送进度，以整个信息为单位
  

enum SendState{ 
	WAIT_MSG,
	SEND_MSG,
	SEND_FEEDBACK,
	SEND_RESET,
	SEND_BREAK
};
 	
enum MsgPhase{ //一个字符串流被还原成ePipeline
	TYPE_PART,
	LENGTH_PART,
	DATA_PART,
	ID_PART,
	NAMELEN_PART,
	NAME_PART
};



class Model;
class CLinker;
class CLinkerPipe;

bool IsEqualAddress(ePipeline& Address1,ePipeline& Address2);

class CLinkerPipe: protected CLockPipe  
{
public:

	//用于流式处理接收到的信息时保存上下文处理状态。
	class RevContextInfo{ 
	public:	
		MsgPhase         State;           // HeaderStr        
		int32            DataType;        // |---+---|  
		AnsiString       HeaderStr;       // type@len@data
		uint32           DataLen;         //          |--|  <--DataLen
		AnsiString       Buffer;          // 因为数据总是被编译成字符串形式
		Energy*          Data;
		ePipeline*       ParentPipe;      //当前正在接收的信息
		int32            InfoLen;
	public:	
		RevContextInfo(ePipeline* Parent)
			:State(TYPE_PART),DataType(-1),DataLen(0),Data(NULL),ParentPipe(Parent),InfoLen(0){
		};
		~RevContextInfo(){ 
		};
		
		void Reset(){
			State = TYPE_PART;
			DataType = -1;
			Buffer = ""; 
			HeaderStr = ""; 
			DataLen  = 0;
			Data = NULL;
			InfoLen = 0;
		};
	};	


	//仅限于ThreadInputProc和ThreadOutputProc使用
	//与CABMutex相比多一个标记m_bThreadUse的行为
	class  _CInnerIOLock  
	{
	private:
		CABMutex*         m_Mutex;
		CLinkerPipe*      m_User;
	public:
		_CInnerIOLock( CABMutex* mutex,CLinkerPipe* User);
		~_CInnerIOLock ();
	};
	
	friend _CInnerIOLock;
	friend CLinker;
private:
		int32                m_UseCounter;    //用于Linker引用计数，当有其他线程用Linker引用此类时，避免被删除
		bool                 m_bThreadUse;    //标记线程IO函数是否在使用此Pipe，避免多线程执行同一个IO

protected:
	
		int64                m_SourceID;         //是信息源ID		
    
		map<int64,ePipeline> m_LocalAddressList; //<SenderID,LocalAdress> SenderID对应的本地地址表

		int32                m_RecoType;         //识别类型
	    
		bool                 m_bClientLink;      //Listen到的连接
		Model*               m_Parent;     


        STATE_OUTPUT_LEVEL   m_StateOutputLevel;

protected: //发送信息

		SendState        m_SendState;		//当前发送状态
		CMsg             m_CurSendMsg;      //当前正在发送的信息
		int64            m_PendingMsgID;    //等待对方报告发送状态的信息，只有报告以后才能发送新信息。
		int64            m_PendMsgSenderID;    
        AnsiString       m_SendBuffer;
		int32            m_SendPos;              //m_CurSenMsg.Content的字符Index。-1表示没有需要发送的信息

protected: //接收信息

	    ePipeline               m_CurRevMsg;  //正在组装完成的信息

		/*
		 接受数据的上下文,当一次读取的数据不足于完成某个数据的组装时就压入此堆栈，等待
		 下次接收到新数据后继续组装，
		*/
		deque<RevContextInfo*>  m_ContextStack;  
		
		bool                    m_bRevError;    //接收数据时产生错误为真 >0 =错误号
	
		//当进入错误状态后，指定后续数据的保存长度
		int32                   m_ErrorSaveLen; 
	
protected: //无需加锁的函数
	bool ReceiverID2LocalAddress(ePipeline& Receiver,ePipeline& LocalAddress);
	int64 LocalAddress2SenderID(ePipeline& LocalAddress);

	void CompileMsg(const char* s, int32 size);
	void RevOneMsg(eElectron& E);
	
	
	//当接受数据时发生错误，调用此函数为Linker进入错误状态做准备
	void BeginErrorState(RevContextInfo* Info,int32 ErrorType);
	void EndErrorState(RevContextInfo* Info);
	
	void    ClearSendMsgList();
	Energy* CreateEmptyData(int32 Type);

	//主要用于内部接收状态的直接回复,并且优先发送
	virtual void FeedbackDirectly(ePipeline* Msg);

    //预留的加密解密接口，供承继类使用,这里无视
	virtual ePipeline* Encrypt(ePipeline* Msg){
		return Msg;
	};
	virtual ePipeline* Decrypt(ePipeline* Msg){
		return Msg;
	};
protected: 
	//这两个非公开函数加了锁
	void   IncreUserNum();
	void   DecreUserNum();

public:
	ePipeline*       m_Owner;

    //int64 GetPendingMsgID_unlock(){ return m_PendingMsgID;};
	//int32 GetGetWaitToSendMsgNum_unlock(){ return Size()+m_UrgenceMsg.Size();};
	//int32 GetSendState_unlock(){return m_SendState;};
	//int32 GetSendBufferSize_unlock(){ return m_SendBuffer.size();};
	//int32 GetSendPos_unlock(){return m_SendPos;};
public: //公开函数必须加锁保证线程安全，同时又要避免递归加锁

	CLinkerPipe(CABMutex* m,Model* Parent,bool bClientLink);
	CLinkerPipe(CABMutex* m,Model* Parent,bool bClientLink,int64 SourceID,tstring Name);
	virtual ~CLinkerPipe();

	void  SetLabel(tstring Text){ m_Label=Text;}; 
    tstring& GetLabel(){return m_Label;};

	void   SetSourceID(int64 SourceID);
	int64  GetSourceID();

	bool   IsClientLinker(){ return m_bClientLink;};

	void   SetStateOutputLevel(STATE_OUTPUT_LEVEL  Level);
    STATE_OUTPUT_LEVEL   GetOutputLevel();

	virtual bool   IsValid();
	virtual void   Close();
	
	void   Reset();
    
	void   CloseDialog(int64 LocalID); //删除此ID打头的对话地址
 
	int32  GetWaitToSendMsgNum(){
		return DataNum();
	}
	int32  GetDialogCount();

	int32  GetRecoType();
	void   SetRecoType(int32 Type);

	int32  GetUserNum();

    bool   IOBusy(){
		return m_bThreadUse;
	}

	SendState GetSendState();

	virtual int64 PushMsgToSend(CMsg& Msg,bool bUrgence=FALSE);

	//停止当前信息的传送，在思绪中调用此函数必须先Lock Linker
	void BreakSend();   

	void SwitchDebugMode(bool open);
	void SetDebugStep(int32 Step);


    //用户在这里实现具体的IO处理，由宿主线程调用
	virtual bool  ThreadIOWorkProc(char* Buffer,uint32 BufSize) =0;
};

//设计此类的目的主要是管理LinkerPipe的生存期，当LinkerPipe要被删除时，如果其他线程正在使用LinkerPipe，
//则可以通过此设计避免提前delete
class CLinker{
	friend CLinker;
private:
    CLinkerPipe* m_Ptr;
	CLinker():m_Ptr(NULL){};
public:
	CLinker(CLinkerPipe* LinkerPipe);
	CLinker(CLinker& Linker);
	CLinker& operator=(CLinker& Linker);
	~CLinker();
	CLinkerPipe& operator()();
	bool IsValid();
	const ePipeline& GetCompileData()const;//用于观察LinkerPipe当前正在组装中的数据
};

} // namespace ABSTRACT

#endif // !define _LINKER_H__


