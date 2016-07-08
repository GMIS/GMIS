/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

class  CWaitSecond_Static : public CElement  
{
public:
	float64    m_Second;  //µÈ´ýÊ±¼ä

	int64      m_StartTimeStamp;
	bool       m_bPause;
public:
	CWaitSecond_Static(int64 ID,float64 Second)
		:CElement(ID,_T("Wait")),m_Second(Second),m_bPause(false){
	};	

	virtual ~CWaitSecond_Static(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};

	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};	

class  CWaitSecond: public CWaitSecond_Static  
{
public:
	CWaitSecond(int64 ID)
		:CWaitSecond_Static(ID,0.0f){
	};	

	virtual ~CWaitSecond(){};

	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};	
