/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

class CGetSpaceCatalog: public CElement
{
	int64   m_InstanceID;
public:
	CGetSpaceCatalog(int64 ID);
	virtual ~CGetSpaceCatalog();

	virtual TypeAB  GetTypeAB(){ return 0;};

	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};



