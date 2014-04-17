// UserModel.h: interface for the CUserModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERMODEL_H__826078EB_84E9_47FE_AAE6_F72BFB07BBCA__INCLUDED_)
#define AFX_USERMODEL_H__826078EB_84E9_47FE_AAE6_F72BFB07BBCA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Model.h"

class CUserModel: public Model{
public:
	class CUserModelIOWork: public Model::CModelIOWork{
	protected:
		HANDLE           m_hObjectDefaultThread;
		DWORD            m_hObjectDefaultThreadID;
	public:
		CUserModelIOWork(int64 ID, CUserModel* Parent);
		virtual ~CUserModelIOWork();
		virtual bool Activation();
		virtual void Dead();
	};
	
	class CUserCentralNerveWork: public Model::CCentralNerveWork{
	protected:
		HANDLE           m_hObjectDefaultThread;
		DWORD            m_hObjectDefaultThreadID;
	public:
		CUserCentralNerveWork(int64 ID, CUserModel* Parent);
		virtual ~CUserCentralNerveWork();
		
		virtual bool Activation();
		virtual void Dead();	
	};

protected:
	int32            m_nCPU;
	HANDLE           m_hObjectDefaultThread;
	DWORD            m_hObjectDefaultThreadID;

	virtual Model::CCentralNerveWork* CreateCentralNerveWorker(int64 ID,Model* Parent,uint32 Reason);
public:
	CUserModel(CModelInitData* InitData);
	virtual ~CUserModel();
	virtual bool Activation();
	virtual void Dead();

	//允许同时连接多个服务器，地址和端口相同则忽略
	bool Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock);

};

#endif // !defined(AFX_USERMODEL_H__826078EB_84E9_47FE_AAE6_F72BFB07BBCA__INCLUDED_)
