/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#pragma once

#include "..\Element.h"

class  CConnectSocket : public CElement  
{
	enum STATE { PHYSICAL_CON, LOGIC_CON,END_CON}; 
public:
	STATE        m_ConState;
	int64        m_LinkerID;

	int64        m_StopTime;
	SOCKET       m_Socket;
public:
	CConnectSocket(int64 ID): CElement(ID,_T("Connect")),m_ConState(PHYSICAL_CON),m_StopTime(0),m_Socket(NULL){};
	~CConnectSocket(){};

	virtual bool Do(ePipeline* Pipe,ePipeline* LocalAddress);

protected:
	bool  CheckConnected(ePipeline* Pipe, ePipeline* LocalAddress);
	virtual void SysMsgProc(CLogicDialog* Dialog,CMsg& SysMsg,ePipeline* ExePipe,ePipeline* LocalAddress);
};




