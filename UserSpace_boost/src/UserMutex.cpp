// UserMutex.cpp: implementation of the CUserMutex class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "UserMutex.h"
#include "format.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserMutex::CUserMutex()
{

}

CUserMutex::~CUserMutex()
{

}

void CUserMutex::Acquire(){
	m_Mutex.lock();
};


void CUserMutex::Release(){
	m_Mutex.unlock();
};