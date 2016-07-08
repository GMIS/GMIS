/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

class CUseRobot: public CElement
{
public:
	CLogicDialog*   m_Dialog;
	int64           m_RobotID;
	ePipeline       m_TaskLogic;  //其中m_Label是此逻辑的明文

protected:
	CUseRobot():CElement(0,_T("")){}; //不允许空引用；
public:
	CUseRobot(int64 ID,CLogicDialog* Dialog, int64 RobotID,ePipeline& LogicPipe);
	virtual ~CUseRobot();

	virtual TypeAB GetTypeAB();
	virtual tstring GetName();

	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};


