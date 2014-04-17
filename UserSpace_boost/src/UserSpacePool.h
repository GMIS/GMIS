// UserSpacePool.h: interface for the CUserSpacePool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERSPACEPOOL_H__103D2440_2A9D_409A_92A4_5A4A99C0D7B6__INCLUDED_)
#define AFX_USERSPACEPOOL_H__103D2440_2A9D_409A_92A4_5A4A99C0D7B6__INCLUDED_

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

#endif // !defined(AFX_USERSPACEPOOL_H__103D2440_2A9D_409A_92A4_5A4A99C0D7B6__INCLUDED_)
