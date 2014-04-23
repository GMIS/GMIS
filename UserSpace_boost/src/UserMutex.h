// UserMutex.h: interface for the CUserMutex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERMUTEX_H__76B5425A_CBB1_4E98_B229_FF4E86EA61CF__INCLUDED_)
#define AFX_MUTEX_H__76B5425A_CBB1_4E98_B229_FF4E86EA61CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "ABMutex.h"

using namespace ABSTRACT;



class CUserMutex : public CABMutex  
{
private:
	boost::mutex   m_Mutex;
public:
	CUserMutex();
	virtual ~CUserMutex();

    virtual void Acquire();


	virtual void Release();
};


#endif // !defined(AFX_USERMUTEX_H__76B5425A_CBB1_4E98_B229_FF4E86EA61CF__INCLUDED_)
