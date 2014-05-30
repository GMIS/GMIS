/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERSPACEPOOL_H__
#define _USERSPACEPOOL_H__


#include "../../TheorySpace/src/AbstractSpacePool.h"

#include "UserMutex.h"

class CUserSpacePool : public CAbstractSpacePool  
{
public:
	CUserSpacePool();
	virtual ~CUserSpacePool();

};

#endif // _USERSPACEPOOL_H__
