// UserTimeStamp.cpp: implementation of the CUserTimeStamp class.
//
//////////////////////////////////////////////////////////////////////

#include <boost/date_time.hpp>
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"
#include "UserMutex.h"
#include "UserTimeStamp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUserTimeStamp::CUserTimeStamp()
{
	m_Mutex = new CUserMutex;
//    m_BirthDay = Date2Days(2010,1,1);
//	int64 t = TimeStamp();
//	tstring s=GetFullTime(t);
}

CUserTimeStamp::~CUserTimeStamp()
{
	if(m_Mutex){
		delete m_Mutex;
		m_Mutex = NULL;
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
	uint32&  Micro,
	uint32&  Nanos
	){

	using namespace boost::gregorian;
    using namespace boost::posix_time;

	ptime pt0 = microsec_clock::universal_time();
    
	ptime pt = boost::date_time::c_local_adjustor<ptime>::utc_to_local(pt0);
	date d = pt.date();
    
	Year =  d.year();
	Month = d.month();
	Day   = d.day();
    
	time_duration td = pt.time_of_day();
	Hour   = td.hours();
	Minute = td.minutes();
	Second = td.seconds();

	Milli  = td.fractional_seconds()/1000000;
	Micro  = td.fractional_seconds()%1000000;
	Nanos  = 0;

};