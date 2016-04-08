// OptionPipe.h: interface for the COptionPipe class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONPIPE_H__D4BAD681_E4BC_4A64_9E97_24EFF29CB606__INCLUDED_)
#define AFX_OPTIONPIPE_H__D4BAD681_E4BC_4A64_9E97_24EFF29CB606__INCLUDED_

#include "TheorySpace.h"
#include "LockPipe.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class COptionPipe : protected ePipeline  
{
protected:
	CABMutex*            m_Mutex;       //确保线程安全
public:
	COptionPipe(CABMutex* m);
	virtual ~COptionPipe();
     
	void ToPipe(ePipeline& Pipe);
	void FromPipe(ePipeline& Pipe);

    BOOL  GetValue(int64 ID,int32& Value);
	BOOL  GetValue(int64 ID,tstring& Value);
	BOOL  SetValue(int64 ID, int32 Value);
	BOOL  SetValue(int64 ID, tstring Value);

};

#endif // !defined(AFX_OPTIONPIPE_H__D4BAD681_E4BC_4A64_9E97_24EFF29CB606__INCLUDED_)
