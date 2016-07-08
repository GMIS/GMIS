/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

// CPipeview��һ�������壬����ePipeline����������Է���һ��Ŀ¼
class  CPipeViewMass  : public CElement
{
protected:
	int32            m_TabNum;

	void PrintView(ePipeline& Result,ePipeline& Pipe);
public:
	CPipeViewMass(int64 ID)
		:CElement(ID,_T("PipeView")){
	};
	virtual ~CPipeViewMass(){};

	void GetPipeInfo(ePipeline* Pipe,tstring& text);
	virtual bool TaskProc(CLogicDialog* Dialog,int32 ChildIndex,CMsg& Msg,ePipeline& ExePipe,ePipeline& LocalAddress);
};


