/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _BRAINOBJECT_H__
#define _BRAINOBJECT_H__

#include "..\Element.h"


#ifdef _WIN32
#include <WINSOCK2.H>
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

//

//常用MASS
////////////////////////////////////

class CLogicThread;
class CLogicDialog;
class CObjectData;
class CArm;


//常用内部Element
////////////////////////////////////

class CInterInstinct: public CElement
{
protected:

	int64           m_InstinctID;
    ePipeline       m_Param;
	
	CInterInstinct();
public:
	CInterInstinct(int64 ID,tstring Name,int64 InstinctID,ePipeline& Param);
	~CInterInstinct();
	
	virtual bool  Do(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
	virtual TypeAB GetTypeAB(){ return 0;};
protected:
   	bool  DoThinkLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoRunTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoDebugTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoStopTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoPauseTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoStepTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoGotoTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoTestTask(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
    bool  DoLearnText(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoLearnWord(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
    bool  DoLearnLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoLearnObject(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoLearnAction(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoLearnLanguage(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoFind(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoFindLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoFindObject(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
	bool  DoSetGlobleLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	
	bool  DoFindSetStartTime(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoFindSetEndTime(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
	bool  DoSetFindPricision(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);
		
	bool  DoCloseDialog(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	
	bool  DoCreateAccount(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	
	bool  DoDeleteAccount(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	

	bool  DoSetLogicAddress(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	
	bool  DoGetLogicAddress(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	
	bool  DoSetLogicBreakpoint(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	
	bool  DoRemoveTempLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	
	bool  DoClearTempLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	

	bool  DoRemoveLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	
	bool  DoInsertLogic(CLogicDialog* Dialog,ePipeline& ExePipe,ePipeline& LocalAddress,CMsg& Msg);	


};


//////////////////////////////////////////////////////////////////////////


/*
class CUseArm: public CElement
{
	CArm&    m_Arm;
public:
	CUseArm(int64 ID,CArm& Arm);
	virtual ~CUseArm();

	virtual TypeAB  GetTypeAB(){ return 0x00000000;} 

	virtual bool TaskProc(CTaskDialog* Dialog,CMsg& Msg,int32 ChildIndex,ePipeline& ExePipe,ePipeline& LocalAddress);
};
*/



#endif // _BRAINOBJECT_H__
