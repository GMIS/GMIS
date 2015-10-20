/*
*author: ZhangHongBing(hongbing75@gmail.com)  
*  
*The LinkerPipe encapsulates a transport protocol that does not rely on any OS. 
*Its main points are :
*1)All data can be expressed and directly use in a ePipeline
*2)It is completely transparent to users when a ePipeline sent or received
*3)Both ends of the same LinkerPipe can distinguish the different local interlocutors.
*4)LinkerPipe has a host,the data sending and receiving state, including the received data,all is as a message transfer to host for handling. it is thread safe.
*5)When used, this class must be inherited and overloading to implement specific physical transmission methods.
*/


#ifndef _LINKER_H__
#define _LINKER_H__


#include <deque>
#include <map>
#include "AbstractSpace.h"
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
#define ERROR_DATA_PART       6
#define ERROR_OTHER_PARNT     7

namespace PHYSIC{

//identification State
#define LINKER_DEL      -2
#define	LINKER_BAN      -1  
#define LINKER_INVALID   0  
#define LINKER_STRANGER  1
#define	LINKER_FRIEND    2

enum STATE_OUTPUT_LEVEL{ NORMAL_LEVEL, LIGHT_LEVEL,WEIGHT_LEVEL};

//output level:
 
//1 NORMAL_LEVEL
#define LINKER_COMPILE_ERROR    101  //If there is any error occured when a received string data being compiled into a ePipeline
#define LINKER_RECEIVE_RESUME   102  //Inform the linker to exit the error state and start receiving information
#define LINKER_INVALID_ADDRESS  103  
#define LINKER_ILLEGAL_MSG      104  

//2 LIGHT_LEVEL
#define LINKER_PUSH_MSG         200  //Pushes a message waiting to be sent and reports the current delivery status
#define LINKER_MSG_SENDED       201  //A message has been sent
#define LINKER_MSG_RECEIVED     202  //A message has been received

//3 WEIGHT_LEVEL
#define LINKER_RECEIVE_STEP     300  //Report receiving progress and data,in each TYPE_PIPELINE data type as a unit 
#define LINKER_SEND_STEP        301  //Report Sending progress,in all bytes of message as a unit
  

enum SendState{ 
	WAIT_MSG,
	SEND_MSG,
	SEND_FEEDBACK,
	SEND_RESET,
	SEND_BREAK
};
 	
enum MsgPhase{ 
	TYPE_PART,
	LENGTH_PART,
	DATA_PART,
	ID_PART,
	NAMELEN_PART,
	NAME_PART
};



class Model;
//class CLinker;
class CLinkerPipe;

bool IsEqualAddress(ePipeline& Address1,ePipeline& Address2);

class CLinkerPipe: protected CLockPipe  
{
public:

	//save the context when receiving based on the stream
	class RevContextInfo{ 
	public:	
		MsgPhase         State;           // HeaderStr        
		int32            DataType;        // |---+---|  
		AnsiString       HeaderStr;       // type@len@data
		uint32           DataLen;         //          |--|  <--DataLen
		AnsiString       Buffer;          //the ePipeline always is converted into string to transfer
		Energy*          Data;
		ePipeline*       ParentPipe;      //stored the temp result when string stream be converted into ePipeline
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


	//only used in ThreadInputProc and ThreadOutputProc
	class  _CInnerIOLock  
	{
	private:
		CABMutex*         m_Mutex;
		CLinkerPipe*      m_User;
	public:
		_CInnerIOLock( CABMutex* mutex,CLinkerPipe* User);
		~_CInnerIOLock ();
	};
	
	friend  class CInnerIOLock;
	friend  class CLinker;
private:
		int32                m_UseCounter;    //For Linker reference counting, when there are other threads to use it to avoid being deleted
		bool                 m_bThreadUse;    //marked whether there was a thread has used the IO func of this class, to avoid a few theads use same IO func at same time

protected:
	
		int64                m_SourceID;         
    
		map<int64,ePipeline> m_LocalAddressList; //<SenderID,LocalAdress>  the local address table that SenderID corresponded

		int32                m_RecoType;         //LINKER_STRANGER or LINKER_FRIEND?
	    
		bool                 m_bClientLink;     // =true if created by Accept();
		Model*               m_Parent;     


        STATE_OUTPUT_LEVEL   m_StateOutputLevel;

protected: //send message

		SendState        m_SendState;		
		CMsg             m_CurSendMsg;      
		int64            m_PendingMsgID;    //The ID of message that waiting for remote end to report receiving state,Only after a report can be sent the new one.
		int64            m_PendMsgSenderID;    
        AnsiString       m_SendBuffer;
		uint32           m_SendPos;         //the char index of m_CurSenMsg.Content.-1 means no message need to sent

protected: //receive message

	    ePipeline               m_CurRevMsg;  //the message that is compiling 

		/*
		 The receiving context list, when a string stream is not sufficient to compile into a ePipeline completely, 
		 it pushs the context in the list for waiting next stream.
		*/
		deque<RevContextInfo*>  m_ContextStack;  
		
		bool                    m_bRevError;    // >0 = error ID
	
		//The specify length that system continue to receive the data when it has gone into error state
		uint32                  m_ErrorSaveLen;
	
protected: //below functions no need to lock (for thead safe) 
	bool ReceiverID2LocalAddress(ePipeline& Receiver,ePipeline& LocalAddress);
	int64 LocalAddress2SenderID(ePipeline& LocalAddress);

	void CompileMsg(const char* s, int32 size);
	void RevOneMsg(eElectron& E);
	
	
	//call this func to go into error state,if there was a error occured when receiving data
	void BeginErrorState(RevContextInfo* Info,int32 ErrorType);
	void EndErrorState(RevContextInfo* Info);
	
	void    ClearSendMsgList();
	Energy* CreateEmptyData(int32 Type);

	//Mainly used for internal send a direct reply that will be preferentially sended
	virtual void FeedbackDirectly(ePipeline* Msg);

    //Reserved for encryption and decryption interface, use by the inherited class, here ignored
	virtual ePipeline* Encrypt(ePipeline* Msg){
		return Msg;
	};
	virtual ePipeline* Decrypt(ePipeline* Msg){
		return Msg;
	};
protected: 
	//These two functions has put a lock
	void   IncreUserNum();
	void   DecreUserNum();

public:
	ePipeline*       m_Owner;

    //int64 GetPendingMsgID_unlock(){ return m_PendingMsgID;};
	//int32 GetGetWaitToSendMsgNum_unlock(){ return Size()+m_UrgenceMsg.Size();};
	//int32 GetSendState_unlock(){return m_SendState;};
	//int32 GetSendBufferSize_unlock(){ return m_SendBuffer.size();};
	//int32 GetSendPos_unlock(){return m_SendPos;};
public: 
	//Below public functions must be locked to ensure thread safe, but also avoiding recursive lock
	CLinkerPipe(CABMutex* m,Model* Parent,bool m_bClientLink);
	CLinkerPipe(CABMutex* m,Model* Parent,bool m_bClientLink,int64 SourceID,tstring Name);
	
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
    
	void   CloseDialog(int64 LocalID); //delete the dialog that begin with LocalID
	int32  GetWaitToSendMsgNum(){
		return DataNum();
	};
	int32  GetDialogCount();

	int32  GetRecoType();
	void   SetRecoType(int32 Type);

	int32  GetUserNum();

    bool   IOBusy(){
		return m_bThreadUse;
	};

	SendState GetSendState();

	virtual int64 PushMsgToSend(CMsg& Msg,bool bUrgence=FALSE);

	//Stop send current message, must be locked  before calling it
	void BreakSend();   

	void SwitchDebugMode(bool open);
	void SetDebugStep(int32 Step);

    //To achieve specific IO processing here, called by host thread
	virtual bool  ThreadIOWorkProc(char* Buffer,uint32 BufSize);

protected:
	//Returns the number of bytes actually processed
	uint32 InputProc(char* Buffer,uint32 BufSize);
	uint32 OutputProc(char* Buffer,uint32 BufSize);


	//should be implement later by user according to actual conditions
	virtual bool  PhysicalRev(char* Buf,uint32 BufSize, uint32& RevLen, uint32 flag=0){
		return false;
	}
	virtual bool  PhysicalSend(char* Buf,uint32 BufSize, uint32& SendLen, uint32 flag=0){
		return false;
	};
};


//The goal of designing this class is to manage the lifetime of LinkerPipe,
//a LinkerPipe will be avoided to delete if another thread is using it.
class CLinker{
//	friend  class CLinker;
private:
    CLinkerPipe* m_Ptr;


	CLinker(const CLinker& Linker)
	:m_Ptr(NULL){
		assert(0);
	};
	CLinker& operator=(const CLinker& Linker){
		assert(0);
		return *this;
	};
public:
	CLinker()
    :m_Ptr(NULL){
	};
	CLinker(CLinkerPipe* LinkerPipe);
	~CLinker();

	void Reset(CLinkerPipe* LinkerPipe);
	CLinkerPipe* Release();

	CLinkerPipe& operator()();
	bool IsValid();
	const ePipeline& GetCompileData()const;//used to view currently assembling data 
};

} // namespace ABSTRACT

#endif // !define _LINKER_H__


