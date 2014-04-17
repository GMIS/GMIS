// UserTimeStamp.h: interface for the CUserTimeStamp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERTIMESTAMP_H__F6190E33_9F0D_401A_9016_3B8E06C1CD0A__INCLUDED_)
#define AFX_USERTIMESTAMP_H__F6190E33_9F0D_401A_9016_3B8E06C1CD0A__INCLUDED_

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
		uint32&  Micro,
		uint32&  Nanos
	);

public:
	CUserTimeStamp();
	virtual ~CUserTimeStamp();

};


#endif // !defined(AFX_USERTIMESTAMP_H__F6190E33_9F0D_401A_9016_3B8E06C1CD0A__INCLUDED_)
