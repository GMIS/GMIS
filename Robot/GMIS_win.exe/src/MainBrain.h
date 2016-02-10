/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _MAINBRAIN_H__
#define _MAINBRAIN_H__


#include "Brain.h"
#include "PhysicSpace.h"
#include "UserMutex.h"
#include "UserLinkerPipe.h"
#include <vector>

#define USING_GUI

class CTaskDialog;
class CBrainDialog;

class CMainBrain : public CBrain  
{
	friend CBrainDialog;

public:
	CMainBrain(CUserTimer* Timer,CUserSpacePool* Pool,tstring Name);

	virtual ~CMainBrain();

	virtual void  SendMsgToLocalGUI(CMsg& GuiMsg);
};



#endif // !defined(_MAINBRAIN_H__)
