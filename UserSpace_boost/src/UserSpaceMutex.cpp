// UserSpaceMutex.cpp: implementation of the CUserSpaceMutex class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "UserSpaceMutex.h"
#include "UserMutex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserSpaceMutex::CUserSpaceMutex()
{

}

CUserSpaceMutex::~CUserSpaceMutex()
{

}

void CUserSpaceMutex::Acquire(){
	m_Mutex.lock();
};


void CUserSpaceMutex::Release(){
	m_Mutex.unlock();
};

SeatMutex* CUserSpaceMutex::CreateSeat(){
	
	SeatMutex*  m = new SeatMutex;
	if(m==NULL)return NULL;
	m->Mutex = new CUserMutex;
	if (m->Mutex == NULL)
	{
		delete m;
		m  = NULL;
		return NULL;
	}
	return m;
}