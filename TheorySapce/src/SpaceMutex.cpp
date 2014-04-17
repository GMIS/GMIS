// SpaceMutex.cpp: implementation of the CSpaceMutex class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable: 4786)

#include "SpaceMutex.h"

namespace ABSTRACT{

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

}