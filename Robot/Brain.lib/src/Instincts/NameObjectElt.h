/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"


class CNameObject: public CElement
{

public:
	CNameObject(int64 ID);  
	virtual ~CNameObject();

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class CNameObject_Static: public CElement
{
	tstring  m_Name;	
public:
	CNameObject_Static(int64 ID,tstring Name);  
	virtual ~CNameObject_Static();

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};