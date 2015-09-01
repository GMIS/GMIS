/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERTIMESTAMP_H__
#define _USERTIMESTAMP_H__

#include "ABTime.h"

class CUserTimer :  public CABTime  
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
	CUserTimer();
	virtual ~CUserTimer();

};



#endif // _USERTIMESTAMP_H__