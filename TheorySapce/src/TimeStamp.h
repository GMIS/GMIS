/*
*  
* 这里的时间戳作用侧重于标识数据对象的唯一性，当然也能提供时间信息。
* 从长远看，提供一个独立于软件系统的硬件时间戳非常重要，目前只能软件模拟
*
* 我们用一个64位整数来表示时间戳，目前软件系统的时间精度只有毫秒级，这意味着在
* 一秒钟内最多只能标识1000个数据，这远远赶不上当前硬件性能，如果把精度提高
* 到百纳秒级，这样一秒可以标识1000×1000×10个唯一ID的数据或对象。
*
* 由于百纳秒远远超出了系统时间的精度，当我们需要产生一个时间戳时，如果前后两次系统
* 时间相等则只是在当前系统时间顺序加1得到时间戳，如果不相等则在新的系统时间上加1。
* 
* 为了避免时间戳过大，时间戳是基于系统生日的时间
* 
* 为了避免系统依赖，使用纯虚函数来获得系统时间，用户应该自己实现确定的版本。
* 
* 另外，这里只提供Mutex指针来避免多线程同步问题，实例化需要用户解决。
*
* author: ZhangHongBing(hongbing75@gmail.com)   
*/

#ifndef _TIMESTAMP_H__
#define _TIMESTAMP_H__


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AbstractSpace.h"
#include <stdlib.h>
#include "ABMutex.h"

namespace ABSTRACT{

class CTimeStamp  
{
protected:
	static uint32  m_BirthDay;  //default = 0,可以m_BirthDay = Date2Days(2010,1,1)
	
	CABMutex*   m_Mutex;
	int64		m_LastTime;        //基于豪秒
	int64		m_LastTimeStamp;   //基于百纳秒

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
	)=0; //用户应该自己根据OS实现此函数


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

	CTimeStamp();
	virtual ~CTimeStamp();
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

	//把int64转换成相应时间格式;
	tstring GetYMD(int64 TimeStamp );        // 得到年月日 
	tstring GetHMS(int64 TimeStamp);         // 得到是时分秒
   	tstring GetHMSM(int64 TimeStamp );       // 得到时分秒毫秒；
	tstring GetYMDHM(int64 TimeStamp );      // 得到年月日时分
	tstring GetYMDHMS(int64 TimeStamp );     // 得到年月日时分秒  
	tstring GetFullTime(int64 TimeStamp );   // 得到年月日时分秒毫秒  

	//返回指定时间戳加上指定时间间隔得到的新时间戳
	int64  AddTime(int64 TimeStamp, int32 Milli,int32 Second=0,int32 Minute=0,int32 Hour=0);


};

} //name space

#endif // _TIMESTAMP_H__