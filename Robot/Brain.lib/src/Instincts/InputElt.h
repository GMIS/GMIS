/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"


class CInputElement: public CElement
{
public:   
	bool             m_bInputNum;  //����ı���������
protected:

	CInputElement():CElement(0,_T("")),m_bInputNum(false){}; //����������ã�
public:
	CInputElement(int64 ID,tstring Tip,bool bInputNum);
	virtual ~CInputElement();

	virtual TypeAB  GetTypeAB(){ return 0;};

	virtual MsgProcState EltMsgProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);

};