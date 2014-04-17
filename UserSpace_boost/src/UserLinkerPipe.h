// SocketLinkerPipe.h: interface for the CUserLinkerPipe class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERLINKERPIPE_H__CBADBA55_64F7_4884_B2D2_3BA004C4CE71__INCLUDED_)
#define AFX_USERLINKERPIPE_H__CBADBA55_64F7_4884_B2D2_3BA004C4CE71__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define BOOST_ASIO_HAS_MOVE

#include <boost/asio.hpp>

#include "SpaceMutex.h"
#include "LinkerPipe.h"

using namespace boost::asio;

class CUserLinkerPipe : public CLinkerPipe  
{    
	SUPPORT_ABSTRACT_SAPCE_POOL(CUserLinkerPipe);
protected:
	bool					 m_bDeleteMutex;
	AnsiString               m_RevBuf;  //default=1024,ʵ�������Ӧ�ÿ����ʼֻ��256�������٣�Ȼ�������Ϣ��С����̬����
	boost::shared_ptr<ip::tcp::socket> m_Socket;
public:
	CUserLinkerPipe(Model* Parent,int64 SourceID,tstring Name); 
	CUserLinkerPipe(CSpaceMutex* Mutex,Model* Parent,int64 SourceID); //����CONNECT server

	virtual ~CUserLinkerPipe();

	void  AttachSocket(boost::shared_ptr<ip::tcp::socket>& Socket); //��ͬʱ��ʼ����һ���첽��
	boost::shared_ptr<ip::tcp::socket> GetSocket();

	virtual bool   IsValid();
	virtual void   Close();
	
	virtual int64  PushMsgToSend(CMsg& Msg,bool bUrgence=FALSE);
	
private: 
	virtual void FeedbackDirectly(ePipeline* Msg);

	virtual bool  ThreadIOWorkProc(char* Buffer,uint32 BufSize){ return TRUE;}; //��������

	void RevHandler(const boost::system::error_code& error, std::size_t bytes_transferred );
	void SendHandler(const boost::system::error_code& error, std::size_t bytes_transferred );
};

#endif // !defined(AFX_USERLINKERPIPE_H__CBADBA55_64F7_4884_B2D2_3BA004C4CE71__INCLUDED_)
