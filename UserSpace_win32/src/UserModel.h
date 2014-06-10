/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERMODEL_H__
#define _USERMODEL_H__

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

#endif // _USERMODEL_H__
