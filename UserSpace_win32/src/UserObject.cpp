// UserObject.cpp: implementation of the CUserObject class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "UserObject.h"


//CUserobject
//////////////////////////////////////////////////////////////////////////

CUserObject::CUserObject( CTimeStamp* Timer,CAbstractSpacePool* Pool)
:Object(Timer,Pool),m_hObjectDefaultThread(NULL),m_hObjectDefaultThreadID(0)
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
	uint32 ThreadID = 0;
	m_hObjectDefaultThread = (HANDLE)_beginthreadex(NULL, // Security
		0,							                     // Stack size - use default
		ObjectDefaultThreadFunc,     		
		(void*)this,	      
		0,					                			 // Init flag
		&ThreadID);					                     // Thread address
	
	assert( m_hObjectDefaultThread != NULL );        
	
	m_hObjectDefaultThreadID = ThreadID;
	return TRUE;
}
void CUserObject::Dead(){
	m_Alive = FALSE;
	WaitForSingleObject (m_hObjectDefaultThread, INFINITE);

}
//UserModel
//////////////////////////////////////////////////////////////////////////


//UserSystem
//////////////////////////////////////////////////////////////////////////
