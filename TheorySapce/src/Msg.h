/*
*author: ZhangHongBing(hongbing75@gmail.com)  
* 
*/

#ifndef _MSG_H__
#define _MSG_H__

#include "Typedef.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


namespace ABSTRACT{


//通用最基本预定义

#define  SYSTEM_SOURCE   0
#define  DEFAULT_DIALOG  0
#define  LOCAL_GUI       0
	
#define  SPACE_SOURCE        1
#define  SPACE_SOURCE_READY -1

#define  MSG_BASE           127282417797660000
#define  MSG_WHO_ARE_YOU    MSG_BASE+1
#define  MSG_BROADCAST_MSG  MSG_BASE+2
#define  MSG_LINKER_ERROR   MSG_BASE+3


/*  

=============
 MSG格式：
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

注意：简单的让Msg.Receiver()=Msg.Sender()会导致EventID错误
*/  

class ePipeline;

//返回收信地址，发信地址和空白的信由用户填写
ePipeline* CreateMsg(int64 MsgID,int64 EventID,ePipeline** ReceiverInfo,ePipeline** Letter,ePipeline** SenderInfo);

class CMsg{
protected:
	ePipeline*  m_MsgPtr;
	bool        m_bReaded; //已读过，只要使用GetLetter()则设置为true,缺省为false
public:
	//系统任何一个对话主动发信给外部（收信地址为对方缺省的系统对话）
	CMsg(int64 MsgID,int64 SenderID,int64 EventID); //EventID如果不使用则为0，但这里不设置缺省值=0，以免和下面构造函数混淆
	//系统内部一个对话给另一个对话发信
	CMsg(int64 ReceiverID,int64 MsgID,int64 SenderID,int64 EventID); 
	CMsg(ePipeline& Receiver,int64 MsgID,int64 EventID); //以系统名义给指定地址发信

	CMsg(ePipeline* Msg);	
	CMsg(CMsg& Msg);
	virtual ~CMsg();	
	
	bool IsValid();
	bool IsReaded();  //只要使用过GetLetter()则设置返回true

	void Reset(ePipeline* Msg = 0);	
	ePipeline* Release();
	
	CMsg& operator=(CMsg& Msg);

	//CMsg类似AutoPtr，只有这个函数是完全产生一个相同的
	CMsg& Clone(CMsg& Msg);

	//根据当前信息的地址和EventID生成一个空的回复信息
	CMsg CreateEmptyBackMsg(int64 MsgID,int64 SenderID);

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
