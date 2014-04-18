/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERTIMESTAMP_H__
#define _USERTIMESTAMP_H__

#include "TimeStamp.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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