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
