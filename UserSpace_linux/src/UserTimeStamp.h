/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERTIMESTAMP_H__
#define _USERTIMESTAMP_H__

#include "../../TheorySpace/src/TimeStamp.h"

class CUserTimeStamp :  public CTimeStamp  
{
private:
	virtual void GetSystemTime( 
		uint32&  Year,
		uint32&  Month,
		uint32&  Day,
		uint32&  Hour,
		uint32&  Minute,
		uint32&  Second,
		uint32&  Milli,
		uint32&  Macro,
		uint32&  Nanos

	);

public:
	CUserTimeStamp();
	virtual ~CUserTimeStamp();

};



#endif // _USERTIMESTAMP_H__
