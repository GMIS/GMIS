/*
*author: ZhangHongBing(hongbing75@gmail.com)  
* 
*/

#ifndef _MSG_H__
#define _MSG_H__

#include "Typedef.h"

namespace ABSTRACT{


//ͨ�����Ԥ����

#define  SYSTEM_SOURCE   0
#define  DEFAULT_DIALOG  0
#define  LOCAL_GUI       0
	
#define  SPACE_SOURCE        1
#define  SPACE_SOURCE_READY -1

#define  MSG_BASE           127282417797660000ULL
#define  MSG_WHO_ARE_YOU    MSG_BASE+1ULL
#define  MSG_BROADCAST_MSG  MSG_BASE+2ULL
#define  MSG_LINKER_ERROR   MSG_BASE+3ULL


/*  

=============
 MSG��ʽ��
  ePipeline{   //m_ID = SourceID
    ePipeline  //ReceiverInfo  m_ID = EventTimeStamp
	ePipeline  //Letter        m_ID = MsgID
	ePipeline  //SenderInfo    m_ID = SendTimeStamp  
  }
 using:
    CMsg Msg(...);
    Msg.GetReceiver()<<ReceiverAddress;  
	Msg.GetSender()<<SenderAddress

    Msg.GetLetter().PushInt32(12);
    ...
    Msg.GetLetter().PushString("Hello");
    m_Ouput->PushToSend(Msg); 

ע�⣺�򵥵���Msg.Receiver()=Msg.Sender()�ᵼ��EventID����
*/  

class ePipeline;

//�������ŵ�ַ�����ŵ�ַ�Ϳհ׵������û���д
ePipeline* CreateMsg(int64 MsgID,int64 EventID,ePipeline** ReceiverInfo,ePipeline** Letter,ePipeline** SenderInfo);

class CMsg{
private:
	CMsg(const CMsg& Msg){
		assert(0);
	};
	CMsg& operator=(const CMsg& Msg){
		assert(0);
		return *this;
	}
protected:
	ePipeline*  m_MsgPtr;
	bool        m_bReaded; //�Ѷ���ֻҪʹ��GetLetter()������Ϊtrue,ȱʡΪfalse
public:
	CMsg();
	//ϵͳ�κ�һ���Ի��������Ÿ��ⲿ�����ŵ�ַΪ�Է�ȱʡ��ϵͳ�Ի���
	CMsg(int64 MsgID,int64 SenderID,int64 EventID); //EventID���ʹ����Ϊ0�������ﲻ����ȱʡֵ=0����������湹�캯�����
	//ϵͳ�ڲ�һ���Ի�����һ���Ի�����
	CMsg(int64 ReceiverID,int64 MsgID,int64 SenderID,int64 EventID); 
	CMsg(ePipeline& Receiver,int64 MsgID,int64 EventID); //��ϵͳ�����ָ����ַ����

	CMsg(ePipeline* Msg);	
	virtual ~CMsg();	
	
	bool IsValid();
	bool IsReaded();  //ֻҪʹ�ù�GetLetter()�����÷���true

	void Reset(ePipeline* Msg = 0);	
	ePipeline* Release();
	
	ePipeline& GetMsg();

	ePipeline& GetReceiver();
	ePipeline& GetLetter();
	ePipeline& GetSender();

	void  SetSourceID(int64 ID);
	int64 GetSourceID();
	int64 GetMsgID();

	int64 GetReceiverID();
	int64 GetSenderID();

	int64 GetEventID();
	int64 GetSendTimeStamp();
};

#define  GET_LETTER(Msg)       static_cast<ePipeline*>(Msg->GetData(1))

} //namespace

#endif // !defined(AFX_MSG_H__A04A1CB0_212A_482F_8D0D_9851512911F8__INCLUDED_)
