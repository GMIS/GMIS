/*
*  
The timestamp in addition to providing time information, but also as a unique ID to identify the data or objects.

In the long run,  that providing an hardware timestamp which is independent of the software system  is very important, At present it is only available with software simulation.

We use a 64-bit integer representing a timestamp, the software system only provided millisecond precision, which means that we can identify no more than 1000 data in one second, the efficiency is very low, if the accuracy is improved to the hundred nanosecond,  we can identify 1000x1000x10 data or objects in a second.

But hundred nanosecond goes far beyond the precision of the system time, therefore,when we need to create a timestamp, if new timestamp equal to the old, we just add 1 to get the latest timestamp.

In order to avoid bigger timestamp, the timestamp is based on the system birthday.

To avoid system dependence, we use a pure virtual functions to get system time, so users should implement their own versions based specified hardware platform.

In addition, here only provide a Mutex pointer to avoid handling multithreading synchronization problems in advance, it requires the user to  instantiate it

* author: ZhangHongBing(hongbing75@gmail.com)   
*/


#ifndef _ABTIME_H__
#define _ABTIME_H__

#include "AbstractSpace.h"
#include <stdlib.h>
#include "ABMutex.h"

namespace ABSTRACT{

class CABTime  
{
protected:
	static uint32  m_BirthDay;  //default = 0,m_BirthDay = Date2Days(2010,1,1)
	
	CABMutex*   m_Mutex;
	int64		m_LastTime;        //based on  milliseconds
	int64		m_LastTimeStamp;   //based on  hundred nanosecond

protected:

	virtual void GetSystemTime( 
		uint32&  Year,
		uint32&  Month,
		uint32&  Day,
		uint32&  Hour,
		uint32&  Minute,
		uint32&  Second,
		uint32&  Milli,
		uint32&  Micro,
		uint32&  Nanos
	)=0; //User should implement this function under specified OS 


	static uint32 Date2Days(
				uint32  Year,
				uint32  Month,
				uint32  Day);
 	
	static void Days2Date(
		        uint32   Days,
				uint32&  Year,
				uint32&  Month,
				uint32&  Day); 

	
	int64 TimeToInt64( 
		uint32&  Year,
		uint32&  Month,
		uint32&  Day,
		uint32&  Hour,
		uint32&  Minute,
		uint32&  Second,
		uint32&  Milli,
		uint32&  Micro,
		uint32&  Nanos
		);

	CABTime();
	virtual ~CABTime();
public:

	int64  TimeStamp();   
	
	static void SetBirthDay(uint32 Year,uint32 Month, uint32 Day);

	void Int64ToTime( 
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
	);

	//Return new timestamp that equal to a old timestamp add the specified time interval
	int64  AddTime(int64 TimeStamp, int32 Milli,int32 Second=0,int32 Minute=0,int32 Hour=0);

	//Convert a int64 into string time format;
	tstring GetYMD(int64 TimeStamp );        // Get year month day
	tstring GetHMS(int64 TimeStamp);         // Get hour minute second
	tstring GetHMSM(int64 TimeStamp );       // Get hour minute second millisecond
	tstring GetYMDHM(int64 TimeStamp );      // Get year month day hour minute 
	tstring GetYMDHMS(int64 TimeStamp );     // Get year month day hour minute second
	tstring GetFullTime(int64 TimeStamp );   // Get year month day hour minute second millisecond
	
};

} //name space

#endif // _ABTIME_H__