/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

class CCloseObject: public CElement
{
public:   
	tstring          m_ObjectName;
public:
	CCloseObject(int64 ID);

	virtual ~CCloseObject();

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);

};

