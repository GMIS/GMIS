/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

static CElement* FindFocusLogic(CElement* Elt,tstring FocusName);

class CFocusLogic: public CElement
{
public:
	CFocusLogic(int64 ID)
		:CElement(ID,_T("Focus Logic")){
	};;

	virtual ~CFocusLogic(){

	};

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool  TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class CFocusLogic_Static: public CElement
{
	tstring   m_FocusName;
public:
	CFocusLogic_Static(int64 ID,tstring FocusName)
		:CElement(ID,_T("Focus Logic")),m_FocusName(FocusName){
	};;

	virtual ~CFocusLogic_Static(){

	};

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool    TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};
class  CInsertLogicStatic : public CElement  
{
public:
	tstring     m_InsertLogicName;
public:
	CInsertLogicStatic(int64 ID,tstring LogicName)
		:CElement(ID,_T("Insert Logic")),m_InsertLogicName(LogicName){
	};	

	virtual ~CInsertLogicStatic(){

	};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};

	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};	

class  CRemoveLogicStatic : public CElement  
{
public:
	CRemoveLogicStatic(int64 ID)
		:CElement(ID,_T("Remove Logic")){
	};	

	virtual ~CRemoveLogicStatic(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};

	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};	

class CSetLogicAddress: public CElement
{
public:
	CSetLogicAddress(int64 ID)
		:CElement(ID,_T("Remove Logic")){
	};	

	virtual ~CSetLogicAddress(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};

	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

class CSetLogicBreakpoint: public CElement
{
public:
	CSetLogicBreakpoint(int64 ID)
		:CElement(ID,_T("Remove Logic")){
	};	

	virtual ~CSetLogicBreakpoint(){};

	virtual TypeAB  GetTypeAB(){ return 0x00000000;};

	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};