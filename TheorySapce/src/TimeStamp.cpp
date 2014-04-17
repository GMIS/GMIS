// TimeStamp.cpp: implementation of the CTimeStamp class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning (disable:4786)

#include "TimeStamp.h"

namespace ABSTRACT{


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

uint32 CTimeStamp::m_BirthDay = 0;
CTimeStamp::CTimeStamp()
{
	m_Mutex = NULL;
	m_LastTime = 0;
}

CTimeStamp::~CTimeStamp()
{

}

int64 CTimeStamp::TimeStamp(){
	uint32  Year;
	uint32  Month;
	uint32  Day;
	uint32  Hour;
	uint32  Minute;
	uint32  Second;
    uint32  Milli;
	uint32  Micro;
	uint32  Nano;

	m_Mutex->Acquire();

	GetSystemTime(Year,Month,Day,Hour,Minute,Second,Milli,Micro,Nano);

	int64 NewTime = TimeToInt64(Year,Month,Day,Hour,Minute,Second,Milli,Micro,Nano);

	assert(m_Mutex);

    int64 t=0;
	if (m_LastTime ==0)
	{
		m_LastTime = NewTime;
		m_LastTimeStamp = m_LastTime;
		t= NewTime;
	}
	else if (NewTime==m_LastTime)
	{
		t= ++m_LastTimeStamp;
	}else if(NewTime>m_LastTimeStamp){
		m_LastTime = NewTime;
		m_LastTimeStamp = m_LastTime+1;
		t= m_LastTimeStamp;
	}else // (NewTime<m_LastTime) //可能修改了时间导致当前时间戳比已经使用的还小
	{ 
		t= ++m_LastTimeStamp; //继续使用当前时间戳
	}

	m_Mutex->Release();
	return t;
}
	
void CTimeStamp::SetBirthDay(uint32 Year,uint32 Month, uint32 Day){
    m_BirthDay = Date2Days(Year,Month,Day);
};
	


//copy from boost::data_time(gregorian_calendar.ipp)
uint32 CTimeStamp::Date2Days(
				uint32  Year,
				uint32  Month,
				uint32  Day){

    unsigned short a = static_cast<unsigned short>((14-Month)/12);
    unsigned short y = static_cast<unsigned short>(Year + 4800 - a);
    unsigned short m = static_cast<unsigned short>(Month + 12*a - 3);
    unsigned long  d = Day + ((153*m + 2)/5) + 365*y + (y/4) - (y/100) + (y/400) - 32045;
    return d;
}
 	
//copy from boost::data_time(gregorian_calendar.ipp)	
void CTimeStamp::Days2Date(
		        uint32   Days,
				uint32&  Year,
				uint32&  Month,
				uint32&  Day){

    uint32 a = Days + 32044;
    uint32 b = (4*a + 3)/146097;
    uint32 c = a-((146097*b)/4);
    uint32 d = (4*c + 3)/1461;
    uint32 e = c - (1461*d)/4;
    uint32 m = (5*e + 2)/153;
    Day = static_cast<unsigned short>(e - ((153*m + 2)/5) + 1);
    Month = static_cast<unsigned short>(m + 3 - 12 * (m/10));
    Year = static_cast<unsigned short>(100*b + d - 4800 + (m/10));  
} 
	

int64 CTimeStamp::TimeToInt64(
		uint32&  Year,
		uint32&  Month,
		uint32&  Day,
		uint32&  Hour,
		uint32&  Minute,
		uint32&  Second,
        uint32&  Milli,
		uint32&  Micro,
		uint32&  Nanos
		)	
{

    int64 T = Date2Days(Year,Month,Day);
	T -= m_BirthDay;

	//计算得到秒
    T *= 86400L;
    T += (Hour * 3600L) + (Minute * 60L) + Second;

    // 得到百纳秒
    T *= 10000000L;
	T += (Milli*10000); //*1000000/100=*10000
	T += (Micro*10);   //*1000/100 = *10
	T += Nanos==0?0:Nanos/100;
    return T;	

}

void CTimeStamp::Int64ToTime(
	    int64    TimeStamp,
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

    assert(TimeStamp != 0);
    if(TimeStamp==0)return;

    int64 t = TimeStamp;
    uint32 nHNanos = (uint32)(t % 10000000L);
    t /= 10000000L;
    uint32 nSecs    = (long)(t % 86400L);
    t /= 86400L;
    uint32 Days = (uint32)(t);
    Days += m_BirthDay;

	Days2Date(Days,Year,Month,Day);


	if (nSecs == 0)
		Hour = Minute = Second = 0;
	else
	{
		Second =  (uint32)nSecs % 60L;
		uint32 nMinutes = nSecs / 60L;
		Minute = (uint32)nMinutes % 60;
		Hour   = (uint32)nMinutes / 60;
	}		
	Milli = nHNanos / 10000L;	
	nHNanos %= 10000L;
	Micro  = nHNanos/10L;
	nHNanos %= 10L;
	Nanos = nHNanos*100;
}

tstring CTimeStamp::GetYMD(int64 TimeStamp ){
	uint32  Year;
	uint32  Month;
	uint32  Day;
	uint32  Hour;
	uint32  Minute;
	uint32  Second;
	uint32  Milli;
	uint32  Micro;
	uint32  Nano;
    Int64ToTime(TimeStamp,Year,Month,Day,Hour,Minute,Second,Milli,Micro,Nano);

    TCHAR buffer[30];
	_stprintf(buffer,_T("%d-%02d-%02d"),Year,Month,Day);
	tstring s(buffer);
	return s;
}

tstring CTimeStamp::GetHMSM(int64 TimeStamp ){
	uint32  Year;
	uint32  Month;
	uint32  Day;
	uint32  Hour;
	uint32  Minute;
	uint32  Second;
	uint32  Milli;
	uint32  Micro;
	uint32  Nano;
    Int64ToTime(TimeStamp,Year,Month,Day,Hour,Minute,Second,Milli,Micro,Nano);

    TCHAR buffer[30];
	_stprintf(buffer,_T("%d:%02d:%02d:%.3d"),Hour,Minute,Second,Milli);
	//std::string s(buffer);
	return buffer;
}
tstring CTimeStamp::GetHMS(int64 TimeStamp ){
	uint32  Year;
	uint32  Month;
	uint32  Day;
	uint32  Hour;
	uint32  Minute;
	uint32  Second;
	uint32  Milli;
	uint32  Micro;
	uint32  Nano;
    Int64ToTime(TimeStamp,Year,Month,Day,Hour,Minute,Second,Milli,Micro,Nano);

    TCHAR buffer[30];
	_stprintf(buffer,_T("%d:%02d:%02d"),Hour,Minute,Second);
	tstring s(buffer);
	return s;
}

tstring CTimeStamp::GetYMDHM(int64 TimeStamp ){
	uint32  Year;
	uint32  Month;
	uint32  Day;
	uint32  Hour;
	uint32  Minute;
	uint32  Second;
	uint32  Milli;
	uint32  Micro;
	uint32  Nano;
    Int64ToTime(TimeStamp,Year,Month,Day,Hour,Minute,Second,Milli,Micro,Nano);

	TCHAR buffer[100];
	_stprintf(buffer,_T("%d-%02d-%02d %02d:%02d"),Year,Month,Day,
		Hour,Minute);
	tstring s(buffer);
	return s;
}

tstring CTimeStamp::GetYMDHMS(int64 TimeStamp ){
	uint32  Year;
	uint32  Month;
	uint32  Day;
	uint32  Hour;
	uint32  Minute;
	uint32  Second;
	uint32  Milli;
	uint32  Micro;
	uint32  Nano;
    Int64ToTime(TimeStamp,Year,Month,Day,Hour,Minute,Second,Milli,Micro,Nano);

	TCHAR buffer[100];
	_stprintf(buffer,_T("%d-%02d-%02d %d:%02d:%02d"),Year,Month,Day,
		Hour,Minute,Second);
	tstring s(buffer);
	return s;
}

tstring CTimeStamp::GetFullTime(int64 TimeStamp){
	
	uint32  Year;
	uint32  Month;
	uint32  Day;
	uint32  Hour;
	uint32  Minute;
	uint32  Second;
	uint32  Milli;
	uint32  Micro;
	uint32  Nano;
    Int64ToTime(TimeStamp,Year,Month,Day,Hour,Minute,Second,Milli,Micro,Nano);

	TCHAR buffer[100];
	_stprintf(buffer,_T("%d-%02d-%02d %d:%02d:%02d:%.3d"),Year,Month,Day,
		Hour,Minute,Second,Milli);
	tstring s(buffer);
	return s;
}

int64  CTimeStamp::AddTime(int64 TimeStamp, int32 Milli,int32 Second/*=0*/,int32 Minute/*=0*/,int32 Hour/*=0*/){
	uint32  rYear;
	uint32  rMonth;
	uint32  rDay;
	uint32  rHour;
	uint32  rMinute;
	uint32  rSecond;
	uint32  rMilli;
	uint32  rMicro;
	uint32  rNano;

    Int64ToTime(TimeStamp,rYear,rMonth,rDay,rHour,rMinute,rSecond,rMilli,rMicro,rNano);
	rMilli  += Milli;
	rSecond += Second;
	rMinute += Minute;
    rHour   += Hour;

    int64 t = TimeToInt64(rYear,rMonth,rDay,rHour,rMinute,rSecond,rMilli,rMicro,rNano);
	return t;
}

}