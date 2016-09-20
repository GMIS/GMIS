/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

class CGetTime: public Mass
{

public:
	CGetTime(int64 ID);
	virtual ~CGetTime();

	virtual tstring GetName();
	virtual TypeAB  GetTypeAB(); 
	virtual bool Do(Energy* E);
};


class CPrintPipeMass: public Mass
{

public:
	CPrintPipeMass(int64 ID);
	virtual ~CPrintPipeMass();

	virtual tstring GetName();
	virtual TypeAB  GetTypeAB();

	virtual bool Do(Energy* E);
};


class  CWaitTime_Static : public CElement  
{
public:
	float64    m_Second;  //µÈ´ýÊ±¼ä

	int64      m_StartTimeStamp;
	bool       m_bPause;
public:
	CWaitTime_Static(int64 ID,float64 Second)
		:CElement(ID,_T("Wait")),m_Second(Second),m_bPause(false){
	};	

	virtual ~CWaitTime_Static(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};

	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};	

class  CWaitTime: public CWaitTime_Static  
{
public:
	CWaitTime(int64 ID)
		:CWaitTime_Static(ID,0.0f){
	};	

	virtual ~CWaitTime(){};

	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};	
