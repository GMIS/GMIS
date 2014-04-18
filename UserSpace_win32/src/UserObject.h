/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USEROBJECT_H__
#define _USEROBJECT_H__

#include "Model.h"
#include "System.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CUserObject: public Object{
protected:
	HANDLE           m_hObjectDefaultThread;
	DWORD            m_hObjectDefaultThreadID;
public:
	CUserObject(CTimeStamp* Timer,CAbstractSpacePool* Pool);
	CUserObject();
	~CUserObject();
	virtual bool Activation();
	virtual void Dead();
};



#endif // _USEROBJECT_H__
