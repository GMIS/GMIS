/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USEROBJECT_H__
#define _USEROBJECT_H__

#include "../../TheorySpace/src/Model.h"
#include "../../TheorySpace/src/System.h"


class CUserObject: public Object{
protected:
	pthread_t        m_hObjectDefaultThread;

public:
	CUserObject(CTimeStamp* Timer,CAbstractSpacePool* Pool);
	CUserObject();
	~CUserObject();
	virtual bool Activation();
	virtual void Dead();
};



#endif // _USEROBJECT_H__
