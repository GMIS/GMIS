/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

class CStartObject: public CElement
{
public:   
	ePipeline*       m_Obj;
protected:
	void Reset(){	
		m_Obj->m_ID = 0; //ʵ��ID
		assert(m_Obj->Size()==5);
	}
	CStartObject():CElement(0,_T("")){}; //����������ã�
public:
	CStartObject(int64 ID,ePipeline* Obj);
	virtual ~CStartObject();

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);

};

//////////////////////////////////////////////////////////////////////////

//focus object��ʵ������ִ�йܵ��ṩ
class CFocusObject: public CElement
{
public:
	CFocusObject(int64 ID)
		:CElement(ID,_T("Focus object"))
	{

	};
	virtual ~CFocusObject(){

	};

	virtual TypeAB  GetTypeAB(){ return PARAM_TYPE1(TYPE_STRING);};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

//��̬�汾����focus objectʵ����ֱ���ɲ����ṩ
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

//�ڲ��汾����focus object instance IDֱ���ɲ����ṩ
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


//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////

class CGetObjectDoc: public CElement
{
public:
	CGetObjectDoc(int64 ID);
	virtual ~CGetObjectDoc();

	virtual TypeAB  GetTypeAB(){ return 0;};

	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

//////////////////////////////////////////////////////////////////////////

class CUseObject: public CElement
{
	int64   m_SpaceEventID;
public:
	CUseObject(int64 ID);
	virtual ~CUseObject();

	virtual TypeAB  GetTypeAB(){ return 0;};

	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

//////////////////////////////////////////////////////////////////////////


class CCloseObject: public CElement
{

public:
	CCloseObject(int64 ID);

	virtual ~CCloseObject();

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);

};

