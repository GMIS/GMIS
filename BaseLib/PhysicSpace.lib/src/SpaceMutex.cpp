// SpaceMutex.cpp: implementation of the CSpaceMutex class.
//
//////////////////////////////////////////////////////////////////////

#include "SpaceMutex.h"

namespace PHYSIC{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSpaceMutex::CSpaceMutex()
:m_SeatNum(2)
{

}

CSpaceMutex::~CSpaceMutex()
{
	map<void*,SeatMutex*>::iterator it = m_UseSeatList.begin();
	while (it != m_UseSeatList.end())
	{
		SeatMutex* Seat = it->second;
		delete Seat;
		it++;
	}
	m_UseSeatList.clear();

	deque<SeatMutex*>::iterator ita = m_BlankSeatList.begin();
	while (ita != m_BlankSeatList.end())
	{
		SeatMutex* Seat = *ita;
		delete Seat;
		ita++;
	}
	m_BlankSeatList.clear();
}
SeatMutex* CSpaceMutex::CreateSeat(){

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

}//end namespace
