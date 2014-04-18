/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _USERSPACEPOOL_H__
#define _USERSPACEPOOL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AbstractSpacePool.h"
#include "UserMutex.h"
class CUserSpacePool : public CAbstractSpacePool  
{
public:
	CUserSpacePool();
	virtual ~CUserSpacePool();

};

#endif // _USERSPACEPOOL_H__
