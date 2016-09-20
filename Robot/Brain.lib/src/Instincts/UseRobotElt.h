/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"


//发送一个请求，得到一个
class CCallRobot: public CElement
{
protected:
	ePipeline*       m_Robot;
	bool             m_bResponded;
	void Reset(){	
		m_Robot->m_ID = 0; //实例ID
		assert(m_Robot->Size()==5);
	}
	CCallRobot():CElement(0,_T("")){}; //不允许空引用；
public:
	CCallRobot(int64 ID,ePipeline* Robot);
	virtual ~CCallRobot();

	virtual TypeAB GetTypeAB(){ 
		if(m_Robot == NULL){
			return PARAM_TYPE2(TYPE_STRING,TYPE_STRING);
		}else{
			return PARAM_TYPE1(TYPE_STRING);
		}
	}
	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);

};


//////////////////////////////////////////////////////////////////////////

class CChatRobot: public CCallRobot
{
public:
	CChatRobot(int64 ID,ePipeline* Robot);
	virtual ~CChatRobot();

	virtual TypeAB GetTypeAB(){ return 0;};

	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);

};

//////////////////////////////////////////////////////////////////////////
class CFocusRobotRequest: public CElement
{
public:
	CFocusRobotRequest(int64 ID)
		:CElement(ID,_T("focus robot"))
	{

	};
	virtual ~CFocusRobotRequest(){

	};

	virtual TypeAB  GetTypeAB(){ return PARAM_TYPE1(TYPE_STRING);};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

//静态版本，即focus request名字直接由参数提供
class CFocusRobotRequest_Static: public CElement
{
	tstring  m_FocusName;
public:
	CFocusRobotRequest_Static(int64 ID,tstring FocusName)
		:CElement(ID,_T("focus robot")),m_FocusName(FocusName)
	{

	};
	virtual ~CFocusRobotRequest_Static(){

	};

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};

//////////////////////////////////////////////////////////////////////////
class CNameRobotRequest: public CElement
{

public:
	CNameRobotRequest(int64 ID);  
	virtual ~CNameRobotRequest();

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};


class CNameRobotRequest_Static: public CElement
{
	tstring  m_Name;	
public:
	CNameRobotRequest_Static(int64 ID,tstring Name);  
	virtual ~CNameRobotRequest_Static();

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};


//////////////////////////////////////////////////////////////////////////
class CExecuteRobotRequest: public CElement
{
	int64   m_WorkingRequestID;
public:
	CExecuteRobotRequest(int64 ID);
	virtual ~CExecuteRobotRequest();

	virtual TypeAB  GetTypeAB(){ return 0;};

	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};


//////////////////////////////////////////////////////////////////////////

class CCloseRobotRequest: public CElement
{

public:
	CCloseRobotRequest(int64 ID);

	virtual ~CCloseRobotRequest();

	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);

};

