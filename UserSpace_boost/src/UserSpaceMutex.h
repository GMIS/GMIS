// UserSpaceMutex.h: interface for the CUserSpaceMutex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USERSPACEMUTEX_H__5F40EADA_91D9_4CC4_8B49_E9354CADBE7A__INCLUDED_)
#define AFX_USERSPACEMUTEX_H__5F40EADA_91D9_4CC4_8B49_E9354CADBE7A__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "SpaceMutex.h"

using namespace ABSTRACT;

class CUserSpaceMutex: public CSpaceMutex
{
private:
	boost::mutex   m_Mutex;
public:
	CUserSpaceMutex();
	virtual ~CUserSpaceMutex();
	
	virtual void Acquire();
	virtual void Release();
	
	virtual SeatMutex* CreateSeat();
};

#endif // !defined(AFX_USERSPACEMUTEX_H__5F40EADA_91D9_4CC4_8B49_E9354CADBE7A__INCLUDED_)
