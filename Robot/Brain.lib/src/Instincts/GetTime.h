/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once
#include "..\Element.h"

class CGetTime: public Mass
{

public:
	CGetTime(int64 ID);
	virtual ~CGetTime();

	virtual tstring GetName();
	virtual TypeAB  GetTypeAB(); 
	virtual bool Do(Energy* E);
};