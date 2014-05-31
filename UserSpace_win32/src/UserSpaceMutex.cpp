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

    ::InitializeCriticalSection(&m_Mutex);
}

CUserSpaceMutex::~CUserSpaceMutex()
{

	::DeleteCriticalSection(&m_Mutex);
}


SeatMutex* CUserSpaceMutex::CreateSeat(){
	
	SeatMutex*  m = new SeatMutex;
	if(m==NULL)return NULL;
	m->Mutex = new CUserMutex;
	if (m->Mutex == NULL)
	{
		delete m;
		return NULL;
	}
	return m;
}
