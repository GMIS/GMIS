/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/



#ifndef _APPBAR_H__
#define _APPBAR_H__


#include "TheorySpace.h"
#include "WSAutoShowBar.h"
#include "TitleView.h"


class CAppBar : public CWSAutoShowBar  
{
public:
	CAppBar();
	virtual ~CAppBar();

    CTitleView* GetConvView(int64 ID);
	virtual LRESULT ParentRecttion(SpaceRectionMsg* Msg);
};

#endif // !defined(_APPBAR_H__)
