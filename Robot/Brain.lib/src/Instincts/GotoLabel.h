/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"


//GOTO LABLE��ת��֮ǰ����ϵͳ����һ���жϣ��������Ա���������ת��ѭ��������������֧û�л���ִ��
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