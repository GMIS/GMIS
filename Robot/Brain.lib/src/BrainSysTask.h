/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "Element.h"


//常用MASS
////////////////////////////////////

class CLogicThread;
class CLogicDialog;

//常用内部Element
////////////////////////////////////

enum SysTaskType{
	SysTask_BrainInit = 100,
	SysTask_GotoSpace,
	SysTask_RobotRequest
};


class CBrainInitElt: public CElement
{
	ePipeline m_Param;
protected:
	//继续处理返回true,否则返回false
	bool  CheckWord(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress);
	bool  CheckInstinct(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress);
public:
	CBrainInitElt(int64 ID,tstring Name,ePipeline& Param);
	virtual ~CBrainInitElt();

	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
};

class CBrainGotoSpaceElt: public CElement
{
	ePipeline m_Param;
public:
	CBrainGotoSpaceElt(int64 ID,tstring Name,ePipeline& Param);
	virtual ~CBrainGotoSpaceElt();

	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
};


class CBrainRobotRequestElt: public CElement
{
protected:
	enum {INIT,QUERY,EXECUTION,USER_STOP,CLOSE };

    int32        m_State;  //0=init, 1=query, 2=execute ,3=close
	int64        m_SpaceEventID;
	tstring      m_Caller;
	tstring      m_Right;
	tstring      m_LogicText;
protected:
	MsgProcState OnEltTaskResult(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	MsgProcState OnEltExecuteRequest(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	MsgProcState OnEltRequestRuntime(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	MsgProcState OnEltCloseRequest(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

	void StopRequest(CLogicDialog* Dialog,ePipeline& ExePipe,tstring tip,bool bClose);
public:
	CBrainRobotRequestElt(int64 ID,tstring Name,ePipeline& Param);
	virtual ~CBrainRobotRequestElt();
	
	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	
};
