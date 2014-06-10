// UserModel.h: interface for the CUserModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERMODEL_H__B5A83673_30BF_4C6C_A531_7D333D681958__INCLUDED_)
#define AFX_USERMODEL_H__B5A83673_30BF_4C6C_A531_7D333D681958__INCLUDED_

#define BOOST_ASIO_HAS_MOVE

#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "Model.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CUserModel: public Model{
public:	
	class CUserModelIOWork: public Model::CModelIOWork{
	protected:
		boost::thread*    m_Thread;
	public:
		CUserModelIOWork(int64 ID, CUserModel* Parent);
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
		CUserCentralNerveWork(int64 ID, CUserModel* Parent);
		virtual ~CUserCentralNerveWork();

		virtual bool Activation();
		virtual void Dead();
		virtual bool IsAlive();
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

	friend class CUserModelIOWork;

private:
	virtual void ConnectHandler(const boost::system::error_code& error, boost::shared_ptr<ConnectPair> cp);
	virtual void ConnectTimeoutHandler(const boost::system::error_code& error, boost::shared_ptr<ConnectPair> cp);

protected:
	int32                     m_nCPU;
	boost::thread*            m_Thread;
	boost::asio::io_service   m_IOService;

	virtual void  NotifySysState(int64 NotifyID,ePipeline* Data = NULL);
	virtual Model::CCentralNerveWork* CreateCentralNerveWorker(int64 ID,Model* Parent,uint32 Reason);

public:
	CUserModel(CModelInitData* InitData);
	virtual ~CUserModel();

	virtual bool Activation();
	virtual void Dead();


	//允许同时连接多个服务器，地址和端口相同则忽略
	bool Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock);

};

#endif // !defined(AFX_USERMODEL_H__B5A83673_30BF_4C6C_A531_7D333D681958__INCLUDED_)
