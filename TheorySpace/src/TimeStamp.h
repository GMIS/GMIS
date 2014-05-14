/*
*  
* �����ʱ������ò����ڱ�ʶ���ݶ����Ψһ�ԣ���ȻҲ���ṩʱ����Ϣ��
* �ӳ�Զ�����ṩһ�����������ϵͳ��Ӳ��ʱ����ǳ���Ҫ��Ŀǰֻ�����ģ��
*
* ������һ��64λ��������ʾʱ�����Ŀǰ���ϵͳ��ʱ�侫��ֻ�к��뼶������ζ����
* һ���������ֻ�ܱ�ʶ1000�����ݣ���ԶԶ�ϲ��ϵ�ǰӲ�����ܣ�����Ѿ������
* �������뼶������һ����Ա�ʶ1000��1000��10��ΨһID�����ݻ����
*
* ���ڰ�����ԶԶ������ϵͳʱ��ľ��ȣ���������Ҫ����һ��ʱ���ʱ�����ǰ������ϵͳ
* ʱ�������ֻ���ڵ�ǰϵͳʱ��˳���1�õ�ʱ������������������µ�ϵͳʱ���ϼ�1��
* 
* Ϊ�˱���ʱ�������ʱ����ǻ���ϵͳ���յ�ʱ��
* 
* Ϊ�˱���ϵͳ������ʹ�ô��麯�������ϵͳʱ�䣬�û�Ӧ���Լ�ʵ��ȷ���İ汾��
* 
* ���⣬����ֻ�ṩMutexָ����������߳�ͬ�����⣬ʵ������Ҫ�û������
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
	static uint32  m_BirthDay;  //default = 0,����m_BirthDay = Date2Days(2010,1,1)
	
	CABMutex*   m_Mutex;
	int64		m_LastTime;        //���ں���
	int64		m_LastTimeStamp;   //���ڰ�����

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
	)=0; //�û�Ӧ���Լ�����OSʵ�ִ˺���


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

	//��int64ת������Ӧʱ���ʽ;
	tstring GetYMD(int64 TimeStamp );        // �õ������� 
	tstring GetHMS(int64 TimeStamp);         // �õ���ʱ����
   	tstring GetHMSM(int64 TimeStamp );       // �õ�ʱ������룻
	tstring GetYMDHM(int64 TimeStamp );      // �õ�������ʱ��
	tstring GetYMDHMS(int64 TimeStamp );     // �õ�������ʱ����  
	tstring GetFullTime(int64 TimeStamp );   // �õ�������ʱ�������  

	//����ָ��ʱ�������ָ��ʱ�����õ�����ʱ���
	int64  AddTime(int64 TimeStamp, int32 Milli,int32 Second=0,int32 Minute=0,int32 Hour=0);


};

} //name space

#endif // _TIMESTAMP_H__