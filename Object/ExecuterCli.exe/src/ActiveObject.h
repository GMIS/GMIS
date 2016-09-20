#ifndef _EXECUTER_H_
#define _EXECUTER_H_


#include "Model.h"
#include "PhysicSpace.h"

tstring GetCurrentDir();
tstring GetTempDir();


class CActiveObject : public PHYSIC::Model
{

protected:
	tstring               m_CryptText;
public:

	CActiveObject(int argc, _TCHAR* argv[]);
	virtual ~CActiveObject();


	void			UnitTest();
	void			SendRuntimeInfo(int64 EventID,ePipeline& Info);
	void			SendRuntimeLogic(int64 EventID,tstring LogicText);
	void			SendFeedback(int64 EventID,ePipeline& ExePipe);
	void			SendFeedbackError(int64 EventID,ePipeline& ExePipe,tstring Error);
	bool			WaitingForLogicResult(int64 SpaceEventID,ePipeline& ExePipe);

	void			GetSuperior(int64 ID,CLinker& Linker);	

	virtual bool	Activate();
	virtual tstring MsgID2Str(int64 MsgID);
	virtual void	OutputLog(uint32 Type,const wchar_t* text);
	virtual void	NotifyLinkerState(int64 SourceID,int64 NotifyID,STATE_OUTPUT_LEVEL Flag,ePipeline& Info);

protected:
	virtual void    CentralNerveMsgProc(CMsg& Msg);

	void			OnWhoAreYou(CMsg& Msg);
	void			OnTaskRuntimeInfo(CMsg& Msg);
	void			OnConnectTo(CMsg& Msg);
	void			OnConnectOK(CMsg& Msg);
	void			OnLinkerNotify(CMsg& Msg);
	void			OnObjectClose(CMsg& Msg);

protected:


	virtual void			OnObjectRun(CMsg& Msg)=0;
	virtual void			OnObjectGetDoc(CMsg& Msg)=0;

};

#endif   