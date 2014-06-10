// UserTimeStamp.cpp: implementation of the CUserTimeStamp class.
//
//////////////////////////////////////////////////////////////////////

#include "UserMutex.h"
#include "UserTimeStamp.h"
#include <sys/time.h>
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

	struct  timeval    tv;

	gettimeofday(&tv,NULL);

	struct tm * ptm = gmtime( (const time_t *)&tv.tv_sec );

    Year =  ptm->tm_year+ 1900;
	Month = ptm->tm_mon+ 1;
	Day   = ptm->tm_mday;
	Hour  = ptm->tm_hour+ 1;
	Minute = ptm->tm_min+ 1;
	Second = ptm->tm_sec+ 1;
	Milli  = tv.tv_usec/1000;
	Macro  = tv.tv_usec%1000;
	Nanos  = 0;
};
