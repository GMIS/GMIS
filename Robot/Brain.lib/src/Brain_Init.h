/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _BRAIN_INIT_H
#define _BRAIN_INIT_H

#include "Brain.h"
#include "Element.h"

class CBrainInit: public CElement
{

public:   
    int64            m_EventType; 
	int32            m_Index;

protected:
	//继续处理返回true,否则返回false
	bool CheckWord(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress);
    bool CheckInstinct(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress);

public:
	CBrainInit(int64 ID);
	virtual ~CBrainInit();
	
	virtual TypeAB  GetTypeAB(){ return 0;};
	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
};

#endif  //_BRAIN_INIT_H