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
	CMainBrain(CSystemInitData* InitData);

	virtual ~CMainBrain();

#ifndef USING_GUI  //直接使用主线程
	virtual bool Do(Energy* E);
	
	//与Brain主要区别是取消生成中枢线程的，因为没有GUI,直接使用主进程
	virtual bool Activation();
#else
	virtual bool Activation();
#endif
	
	virtual void SendMsgToGUI(int64 GuiID,CMsg& GuiMsg); //向用户界面发送消息,这是Brain唯一与用户界面打交道的接口
};



#endif // !defined(_MAINBRAIN_H__)
