// UserObject.cpp: implementation of the CUserObject class.
//
//////////////////////////////////////////////////////////////////////

#include "UserObject.h"


//CUserobject
//////////////////////////////////////////////////////////////////////////

CUserObject::CUserObject( CTimeStamp* Timer,CAbstractSpacePool* Pool)
:Object(Timer,Pool),m_hObjectDefaultThread(NULL)
{

}
CUserObject::CUserObject(){
	
};

CUserObject::~CUserObject(){
	if (m_Alive)
	{
		Dead();
	}
};

bool CUserObject::Activation(){
	if(m_Alive)return TRUE;

	m_Alive  = TRUE;
	int ret = pthread_create(&m_hObjectDefaultThread, NULL, ObjectDefaultThreadFunc, (void*)this);
	if(ret!=0)
	{
		return false;
	}
	return true;
}
void CUserObject::Dead(){
	m_Alive = FALSE;
	pthread_join(m_hObjectDefaultThread,NULL);

}
//UserModel
//////////////////////////////////////////////////////////////////////////


//UserSystem
//////////////////////////////////////////////////////////////////////////
