/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"


enum EXE_EXPECTATION{
	SUCCESS_NORMAL = 0,
	ERROR_NORMAL,
	ERROR_DATATYPE 
};

class CTestExpectation: public CElement
{
protected:
	bool m_bExpectation;
public:
	CTestExpectation(int64 ID,bool bExpectation,tstring Name=_T("Test Expectation"));
	virtual ~CTestExpectation();
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};


class CBrainTestElt: public CElement
{

public:
	CBrainTestElt(int64 ID,tstring Name=_T("Brain Test"));
	virtual ~CBrainTestElt();
	
	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool Do(CLogicDialog* Dialog,ePipeline& ExePipe, ePipeline& LocalAddress,CMsg& Msg);
	
};
