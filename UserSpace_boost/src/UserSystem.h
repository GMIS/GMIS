// UserSystem.h: interface for the CUserSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERSYSTEM_H__BD5A1873_0F1E_41BB_860E_8C75B1D9A8AB__INCLUDED_)
#define AFX_USERSYSTEM_H__BD5A1873_0F1E_41BB_860E_8C75B1D9A8AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#define BOOST_ASIO_HAS_MOVE

#include <boost/thread.hpp>
#include <boost/asio.hpp>

using namespace ABSTRACT;

#include "System.h"

class CUserLinkerPipe;

class CUserSystem: public System{
public:
	class CUserModelIOWork: public Model::CModelIOWork{
	protected:
		boost::thread*    m_Thread;
	public:
		CUserModelIOWork(int64 ID, CUserSystem* Parent);
		virtual ~CUserModelIOWork();
		virtual bool Activation();
		virtual void Dead();
		virtual bool Do(Energy* E);
		virtual bool IsAlive();

	};
	
	class CUserCentralNerveWork: public Model::CCentralNerveWork{
	protected:
		boost::thread*    m_Thread;
	public:
		CUserCentralNerveWork(int64 ID, CUserSystem* Parent);
		virtual ~CUserCentralNerveWork();
		virtual bool Activation();
		virtual void Dead();
		virtual bool IsAlive();

	};

	class CUserSystemIOWork: public System::CSystemIOWork{
	protected:
		boost::thread*    m_Thread;
	public:
		CUserSystemIOWork(int64 ID, CUserSystem* Parent);
		virtual ~CUserSystemIOWork();
		virtual bool Activation();
		virtual void Dead();
		virtual bool Do(Energy* E);
		virtual bool IsAlive();

	};
	
	class CUserNerveWork: public System::CNerveWork{
	protected:
		boost::thread*    m_Thread;
	public:
		CUserNerveWork(int64 ID, CUserSystem* Parent);
		virtual ~CUserNerveWork();
		virtual bool Activation();
		virtual void Dead();
		virtual bool IsAlive();

	};
	

	class AcceptPair{
	public:
		boost::shared_ptr<boost::asio::ip::tcp::socket>  m_pSocket;
		boost::asio::ip::tcp::acceptor	                 m_Acceptor;
	public:	
		AcceptPair(boost::asio::io_service&  IOService);
	};
	
	class ConnectPair{
	public:
		int64                                            m_ID;
		boost::asio::deadline_timer                      m_Timer;
		int32                                            m_TimeCount;
		boost::shared_ptr<boost::asio::ip::tcp::socket>  m_pSocket;
    public:
		ConnectPair(int64 ID,boost::asio::io_service&  IOService,int32 TimeOut);
	};
	
	class CUserLockedSystemData : public System::CLockedSystemData{
	private:
		map<int32,boost::shared_ptr<AcceptPair>> m_AcceptorList;
	public:
		CUserLockedSystemData(CABMutex* mutex);
		virtual ~CUserLockedSystemData();
		void  AddAcceptor(CUserSystem* System,int32 Port,bool bIP6);
		void  DelAcceptor(int32 Port);
		virtual void Dead();
		
	};
	friend class AcceptPair;
	friend class CUserLockedSystemData;
	friend class CUserSystemIOWork;

private:
	virtual void AcceptHandler(const boost::system::error_code& error, boost::shared_ptr<AcceptPair> ap);
	virtual void ConnectHandler(const boost::system::error_code& error, boost::shared_ptr<ConnectPair> cp);
	virtual void ConnectTimeoutHandler(const boost::system::error_code& error, boost::shared_ptr<ConnectPair> cp);

protected:
	int32                       m_nCPU;
	boost::thread*			    m_Thread;
	boost::asio::io_service     m_IOService;

	void StartAccept(boost::shared_ptr<AcceptPair> ap);
	
	virtual void  NotifySysState(int64 NotifyID,ePipeline* Data = NULL);
	
	virtual void NerveProc(CMsg& Msg){
		//一般神经对信息的处理函数，用户需要自己实现
	};

	virtual System::CNerveWork* CreateNerveWorker(int64 ID,System* Parent,uint32 Reason);

public:
	CUserSystem(CSystemInitData* InitData);
	virtual ~CUserSystem();

	virtual bool Activation();
	virtual void Dead();

	//允许同时连接多个服务器，地址和端口相同则忽略
	bool Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock);

	//打开一个端口并listen,允许同时打开多重端口，端口重复则忽略
	bool OpenPort(int32 Port,tstring& error,bool bIP6);
    void ClosePort(int32 Port);

	
//用于Accept()
	CUserLinkerPipe* CreateClientLinkerPipe();

};


#endif // !defined(AFX_USERSYSTEM_H__BD5A1873_0F1E_41BB_860E_8C75B1D9A8AB__INCLUDED_)
