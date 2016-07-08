/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

class CFocusObject: public CElement
{
public:
	CFocusObject(int64 ID)
		:CElement(ID,_T("Focus object"))
	{

	};
	virtual ~CFocusObject(){

	};

	virtual TypeAB  GetTypeAB(){ return 0x30000000;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

//静态版本，即focus object名字直接由参数提供
class CFocusObject_Static: public CElement
{
	tstring  m_FocusName;
public:
	CFocusObject_Static(int64 ID,tstring FocusName)
		:CElement(ID,_T("Focus object")),m_FocusName(FocusName)
	{

	};
	virtual ~CFocusObject_Static(){

	};

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

//内部版本，即focus object instance ID名字直接由参数提供
class CFocusObject_Inter: public CElement
{
	int64   m_InstanceID;
public:
	CFocusObject_Inter(int64 ID,int64 InstanceID)
		:CElement(ID,_T("Focus object")),m_InstanceID(InstanceID)
	{

	};
	virtual ~CFocusObject_Inter(){

	};

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};