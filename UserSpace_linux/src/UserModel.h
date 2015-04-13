/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERMODEL_H__
#define _USERMODEL_H__

#include "../../TheorySpace/src/Model.h"
#include <pthread.h>

class CUserModel: public Model{
public:
	class CUserModelIOWork: public Model::CModelIOWork{
	protected:
		pthread_t           m_hObjectDefaultThread;
	public:
		CUserModelIOWork(int64 ID, CUserModel* Parent);
		virtual ~CUserModelIOWork();
		virtual bool Activation();
		virtual void Dead();
	};
	
	class CUserCentralNerveWork: public Model::CCentralNerveWork{
	protected:
		pthread_t   m_hObjectDefaultThread;

	public:
		CUserCentralNerveWork(int64 ID, CUserModel* Parent);
		virtual ~CUserCentralNerveWork();
		
		virtual bool Activation();
		virtual void Dead();	
	};

protected:
	int32            m_nCPU;
	pthread_t        m_hObjectDefaultThread;


	virtual Model::CCentralNerveWork* CreateCentralNerveWorker(int64 ID,Model* Parent,uint32 Reason);
public:
	CUserModel(CModelInitData* InitData);
	virtual ~CUserModel();
	virtual bool Activation();
	virtual void Dead();

	//Allows to connect several servers at same time, if address and port are the same then ignore
	bool Connect(int64 ID,AnsiString Address,int32 Port,int32 TimeOut,tstring& error,bool bBlock);

};

#endif // _USERMODEL_H__
