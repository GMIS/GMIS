/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"


class CGetDate: public Mass
{

public:
	CGetDate(int64 ID);
	virtual ~CGetDate();

	virtual tstring GetName();
	virtual TypeAB  GetTypeAB();

	virtual bool Do(Energy* E);
};
