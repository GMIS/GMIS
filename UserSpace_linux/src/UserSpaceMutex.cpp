// UserSpaceMutex.cpp: implementation of the CUserSpaceMutex class.
//
//////////////////////////////////////////////////////////////////////

#include "UserSpaceMutex.h"
#include "UserMutex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserSpaceMutex::CUserSpaceMutex()
{


    int ret = pthread_mutex_init(&m_Mutex, NULL);
    assert(ret !=0 );
}

CUserSpaceMutex::~CUserSpaceMutex()
{

	pthread_mutex_destroy(&m_Mutex);
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
