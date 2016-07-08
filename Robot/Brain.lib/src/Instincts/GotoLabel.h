/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"


//GOTO LABLE在转向之前会向系统申请一次中断，这样可以避免在无限转向循环中其他并联分支没有机会执行
class CGotoLabel: public CElement
{
private:
	tstring      m_Name;
	tstring      m_GotoLabel;

public:
	CGotoLabel(const TCHAR* Name,int64 ID,tstring& Label)
		:CElement(ID,Name),m_GotoLabel(Label)
	{		
	};
	virtual ~CGotoLabel(){};
	virtual TypeAB GetTypeAB(){ return 0;};

	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};