// UserObject.h: interface for the CUserObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USEROBJECT_H__72C9065C_4EDD_46C8_B90C_E0C1245A106B__INCLUDED_)
#define AFX_USEROBJECT_H__72C9065C_4EDD_46C8_B90C_E0C1245A106B__INCLUDED_


#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "Object.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUserObject: public Object{

protected:
	boost::thread*    m_Thread;
public:
	CUserObject(CTimeStamp* Timer,CAbstractSpacePool* Pool);
	CUserObject();
	virtual ~CUserObject();

	virtual bool Activation();
	virtual void Dead();

};


#endif // !defined(AFX_USEROBJECT_H__72C9065C_4EDD_46C8_B90C_E0C1245A106B__INCLUDED_)
