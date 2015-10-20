// UserTimeStamp.cpp: implementation of the CUserTimeStamp class.
//
//////////////////////////////////////////////////////////////////////

#include "UserMutex.h"
#include "UserTimer.h"

#if defined(USING_POCO)
#include "Poco/DateTime.h"
#include "poco/LocalDateTime.H"
using namespace Poco;

#elif defined(USING_WIN32)
#include <WinBase.h>
#else 
#error "Lack of specified platform #define(eg.USING_POCO or USING_WIN32 or other UserSpace implementation relied on your OS,default using USING_POCO because of it is cross-platform"

#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserTimer::CUserTimer()
{
	m_Mutex = new CUserMutex;
  //  m_BirthDay = Date2Days(2010,1,1);
}

CUserTimer::~CUserTimer()
{
	if(m_Mutex){
		delete m_Mutex;
	}
}

void CUserTimer::GetSystemTime( 
		uint32&  Year,
		uint32&  Month,
		uint32&  Day,
		uint32&  Hour,
		uint32&  Minute,
		uint32&  Second,
		uint32&  Milli,
		uint32&  Macro,
		uint32&  Nanos

		){

#if defined(USING_POCO)

	Timestamp timestamp;
	DateTime tt(timestamp);
	LocalDateTime st(tt);

    Year = st.year();
	Month = st.month();
	Day   = st.day();
	Hour  = st.hour();
	Minute = st.minute();
	Second = st.second();
	Milli  = st.millisecond();
	Macro  = st.microsecond();
	Nanos  = 0;
#elif defined(USING_WIN32)
	SYSTEMTIME  st;
	GetLocalTime(&st);
	Year = st.wYear;
	Month = st.wMonth;
	Day   = st.wDay;
	Hour  = st.wHour;
	Minute = st.wMinute;
	Second = st.wSecond;
	Milli  = st.wMilliseconds;
	Macro  = 0;
	Nanos  = 0;
#endif
};
