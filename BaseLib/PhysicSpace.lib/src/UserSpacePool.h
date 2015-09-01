/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERSPACEPOOL_H__
#define _USERSPACEPOOL_H__


#include "ABSpacePool.h"
#include "UserMutex.h"

class CUserSpacePool : public CABSpacePool  
{
public:
	CUserSpacePool();
	virtual ~CUserSpacePool();

};

#endif // _USERSPACEPOOL_H__
