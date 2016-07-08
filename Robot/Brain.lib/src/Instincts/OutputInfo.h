/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

class COutputInfo: public CElement
{
public:
	COutputInfo(int64 ID);
	virtual ~COutputInfo();

	virtual TypeAB  GetTypeAB();

	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};


