/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"


class  CDebugBreak: public Mass
{
	Mass*  m_SrcMass;
public:
	CDebugBreak(Mass* srcMass);
	virtual ~CDebugBreak();
	virtual MASS_TYPE  MassType();
	virtual TypeAB GetTypeAB();
	virtual tstring GetName();
	virtual bool  Do(Energy* E);
	Mass* Release();
};

