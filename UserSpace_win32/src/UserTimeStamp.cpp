// UserTimeStamp.cpp: implementation of the CUserTimeStamp class.
//
//////////////////////////////////////////////////////////////////////

#include "UserMutex.h"
#include "UserTimeStamp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserTimeStamp::CUserTimeStamp()
{
	m_Mutex = new CUserMutex;
  //  m_BirthDay = Date2Days(2010,1,1);
}

CUserTimeStamp::~CUserTimeStamp()
{
	if(m_Mutex){
		delete m_Mutex;
	}
}

void CUserTimeStamp::GetSystemTime( 
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
};