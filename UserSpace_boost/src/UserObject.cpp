// UserObject.cpp: implementation of the CUserObject class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "UserObject.h"



//CUserobject
//////////////////////////////////////////////////////////////////////////

CUserObject::CUserObject(CTimeStamp* Timer,CAbstractSpacePool* Pool)
:Object(Timer,Pool),m_Thread(NULL)
{

}
CUserObject::CUserObject(){
	
};

CUserObject::~CUserObject(){
	if (m_Thread)
	{
		delete m_Thread;
		m_Thread = NULL;
	}
}


bool CUserObject::Activation(){
	if(m_Alive)return TRUE;
	m_Alive  = TRUE;
	if(m_Thread){
		delete m_Thread;
		m_Thread = NULL;
	};
	m_Thread = new boost::thread(ObjectDefaultThreadFunc,this);
	return TRUE;
}

void CUserObject::Dead(){
	Object::Dead();
	m_Alive = FALSE;
	if(m_Thread){
		m_Thread->join();
	}
}

