/*
*author: ZhangHongBing(hongbing75@gmail.com)  
*  
* LinkerPipe��װ��һ���������κ�OSϵͳ�Ĵ���Э�顣���Ҫ���ǣ�
*
*	1)������Ϣ������ePipeline��ʽ���,��ePipeline��ʽֱ��ʹ��
*	2)һ��ePipeline���ַ���ʽ����ȥ�����ն���ʵʱ��װ��ePipeline,���û���ȫ͸��
*	3)ͬһ��LinkerPipe����֧����ֱ��ز�ͬ�ĶԻ��ߡ�
*	4)LinkerPipe���������շ�״̬���������������ܵ�����Ϣ������ֻ�踺�������Ϣ��
*	5)�̰߳�ȫ��
*	6)ʹ��ʱ������м̴��࣬Ȼ������ʵ�־�������?�䷽ʽ��
*/

#ifndef _LINKER_H__
#define _LINKER_H__


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

//ʶ��״̬
#define LINKER_DEL      -2
#define	LINKER_BAN      -1  //��ֹ
#define LINKER_INVALID   0  //��Ч
#define LINKER_STRANGER  1
#define	LINKER_FRIEND    2

//���LinkerPipe�Ĺ���״̬
enum STATE_OUTPUT_LEVEL{ NORMAL_LEVEL, LIGHT_LEVEL,WEIGHT_LEVEL};

//����ּ�:
 
//1 ������� NORMAL_LEVEL
#define LINKER_COMPILE_ERROR    101  //װ����Ϣʱ������κδ��󣬽���������״̬
#define LINKER_RECEIVE_RESUME   102  //Linker����״̬�ָ����˳��������״̬
#define LINKER_INVALID_ADDRESS  103  //�޷�������ŵ�ַ�ҵ����ض�Ӧ������
#define LINKER_ILLEGAL_MSG      104  //���յ�����һ���Ϸ���ʽ��Ϣ

//2 ������ LIGHT_LEVEL
#define LINKER_PUSH_MSG         200  //ѹ��һ����Ϣ�ȴ��ͣ������浱ǰ����״̬
#define LINKER_MSG_SENDED       201  //ĳ����Ϣ�Ѿ��������
#define LINKER_MSG_RECEIVED     202  //���յ�ĳ����Ϣ

//3 �ض���� WEIGHT_LEVEL
#define LINKER_RECEIVE_STEP     300  //������ܽ�Ȳ������յ������,��ÿһ��TYPE_PIPELINE�������Ϊ��λ�����������֮��Ĺ�ϵ�������֮��Ĺ�ϵ�����ױ��Ƶ���
#define LINKER_SEND_STEP        301  //���淢�ͽ�ȣ��������ϢΪ��λ
  

enum SendState{ 
	WAIT_MSG,
	SEND_MSG,
	SEND_FEEDBACK,
	SEND_RESET,
	SEND_BREAK
};
 	
enum MsgPhase{ //һ���ַ�������ԭ��ePipeline
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

	//������ʽ������յ�����Ϣʱ���������Ĵ���״̬��
	class RevContextInfo{ 
	public:	
		MsgPhase         State;           // HeaderStr        
		int32            DataType;        // |---+---|  
		AnsiString       HeaderStr;       // type@len@data
		uint32           DataLen;         //          |--|  <--DataLen
		AnsiString       Buffer;          // ��Ϊ������Ǳ�������ַ���ʽ
		Energy*          Data;
		ePipeline*       ParentPipe;      //��ǰ���ڽ��յ���Ϣ
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


	//������ThreadInputProc��ThreadOutputProcʹ��
	//��CABMutex��ȶ�һ�����m_bThreadUse����Ϊ
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
		int32                m_UseCounter;    //����Linker���ü����������߳���Linker���ô���ʱ�����ⱻɾ��
		bool                 m_bThreadUse;    //����߳�IO�����Ƿ���ʹ�ô�Pipe��������߳�ִ��ͬһ��IO

protected:
	
		int64                m_SourceID;         //����ϢԴID		
    
		map<int64,ePipeline> m_LocalAddressList; //<SenderID,LocalAdress> SenderID��Ӧ�ı��ص�ַ��

		int32                m_RecoType;         //ʶ������
	    
		bool                 m_bClientLink;      //Listen��������
		Model*               m_Parent;     


        STATE_OUTPUT_LEVEL   m_StateOutputLevel;

protected: //������Ϣ

		SendState        m_SendState;		//��ǰ����״̬
		CMsg             m_CurSendMsg;      //��ǰ���ڷ��͵���Ϣ
		int64            m_PendingMsgID;    //�ȴ�Է����淢��״̬����Ϣ��ֻ�б����Ժ���ܷ�������Ϣ��
		int64            m_PendMsgSenderID;    
        AnsiString       m_SendBuffer;
		uint32            m_SendPos;              //m_CurSenMsg.Content���ַ�Index��-1��ʾû����Ҫ���͵���Ϣ

protected: //������Ϣ

	    ePipeline               m_CurRevMsg;  //������װ��ɵ���Ϣ

		/*
		 ������ݵ�������,��һ�ζ�ȡ����ݲ��������ĳ����ݵ���װʱ��ѹ��˶�ջ���ȴ�
		 �´ν��յ�����ݺ������װ��
		*/
		deque<RevContextInfo*>  m_ContextStack;  
		
		bool                    m_bRevError;    //�������ʱ�������Ϊ�� >0 =�����
	
		//���������״̬��ָ��������ݵı��泤��
		uint32                   m_ErrorSaveLen;
	
protected: //�������ĺ���
	bool ReceiverID2LocalAddress(ePipeline& Receiver,ePipeline& LocalAddress);
	int64 LocalAddress2SenderID(ePipeline& LocalAddress);

	void CompileMsg(const char* s, int32 size);
	void RevOneMsg(eElectron& E);
	
	
	//���������ʱ������󣬵��ô˺���ΪLinker�������״̬��׼��
	void BeginErrorState(RevContextInfo* Info,int32 ErrorType);
	void EndErrorState(RevContextInfo* Info);
	
	void    ClearSendMsgList();
	Energy* CreateEmptyData(int32 Type);

	//��Ҫ�����ڲ�����״̬��ֱ�ӻظ�,�������ȷ���
	virtual void FeedbackDirectly(ePipeline* Msg);

    //Ԥ���ļ��ܽ��ܽӿڣ����м���ʹ��,��������
	virtual ePipeline* Encrypt(ePipeline* Msg){
		return Msg;
	};
	virtual ePipeline* Decrypt(ePipeline* Msg){
		return Msg;
	};
protected: 
	//�������ǹ������������
	void   IncreUserNum();
	void   DecreUserNum();

public:
	ePipeline*       m_Owner;

    //int64 GetPendingMsgID_unlock(){ return m_PendingMsgID;};
	//int32 GetGetWaitToSendMsgNum_unlock(){ return Size()+m_UrgenceMsg.Size();};
	//int32 GetSendState_unlock(){return m_SendState;};
	//int32 GetSendBufferSize_unlock(){ return m_SendBuffer.size();};
	//int32 GetSendPos_unlock(){return m_SendPos;};
public: //��������������֤�̰߳�ȫ��ͬʱ��Ҫ����ݹ����

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
    
	void   CloseDialog(int64 LocalID); //ɾ���ID��ͷ�ĶԻ���ַ
 
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

	//ֹͣ��ǰ��Ϣ�Ĵ��ͣ���˼���е��ô˺��������Lock Linker
	void BreakSend();   

	void SwitchDebugMode(bool open);
	void SetDebugStep(int32 Step);


    //�û�������ʵ�־����IO���?�������̵߳���
	virtual bool  ThreadIOWorkProc(char* Buffer,uint32 BufSize) =0;
};

//��ƴ����Ŀ����Ҫ�ǹ���LinkerPipe������ڣ���LinkerPipeҪ��ɾ��ʱ����������߳�����ʹ��LinkerPipe��
//�����ͨ�����Ʊ�����ǰdelete
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
	const ePipeline& GetCompileData()const;//���ڹ۲�LinkerPipe��ǰ������װ�е����
};

} // namespace ABSTRACT

#endif // !define _LINKER_H__


