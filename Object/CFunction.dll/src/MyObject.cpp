// MyObject.cpp: implementation of the MyObject class.
//
//////////////////////////////////////////////////////////////////////

#include "MyObject.h"
#include <assert.h>
#include <shellapi.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool MyObject::Do(Energy* E)
{
	//首先把输入的参数转换成一个数据管道
	assert(E->EnergyType() == TYPE_PIPELINE);
	ePipeline* Pipe = (ePipeline*)E;

	//如果数据管道里为空，则直接返回，执行失败
	if (Pipe->Size()==0)
	{
		return false;
	}


	
    ENERGY_TYPE Type = Pipe->GetDataType(0);
	
	//我们要求数据管道里的第一个数据必须是字符串
	if (Type != TYPE_STRING)
	{
		return false;
	}

	//取出字符串，这就是我们要求执行的C函数名
	tstring FunctionName = Pipe->PopString();
	
	if (FunctionName == _T("sin"))
	{
		//如果是sin，那么要求数据管道里还有一个数据是小数或者整数
		
		if (Pipe->Size()==0)
		{
			return false;
		}
		Type = Pipe->GetDataType(0);

		//取出这个数字，它就是sin函数的输入参数
		float64 t=0.0f;
		if (Type == TYPE_FLOAT  )
		{
			t = Pipe->PopFloat();
		
		}else if (Type == TYPE_INT)
		{
			t = Pipe->PopInt();
		}else{
			return false;

		}

		//调用c函数sin执行
		float64 n = sin(t);

		//然后把返回的结果压入到数据管道中
		Pipe->PushFloat(n);

		//最后表示执行成功
		return true; 
	}else if (FunctionName == _T("cos"))
	{
		//如果是sin，那么要求数据管道里还有一个数据是小数或者整数
		
		if (Pipe->Size()==0)
		{
			return false;
		}
		Type = Pipe->GetDataType(0);
		
		//取出这个数字，它就是sin函数的输入参数
		float64 t=0.0f;
		if (Type == TYPE_FLOAT  )
		{
			t = Pipe->PopFloat();
			
		}else if (Type == TYPE_INT)
		{
			t = Pipe->PopInt();
		}else{
			return false;
			
		}
		
		//调用c函数sin执行
		float64 n = cos(t);
		
		//然后把返回的结果压入到数据管道中
		Pipe->PushFloat(n);
		
		//最后表示执行成功
		return true; 
	}else if(FunctionName == _T("tan"))
	{
		//如果是sin，那么要求数据管道里还有一个数据是小数或者整数
		
		if (Pipe->Size()==0)
		{
			return false;
		}
		Type = Pipe->GetDataType(0);
		
		//取出这个数字，它就是sin函数的输入参数
		float64 t=0.0f;
		if (Type == TYPE_FLOAT  )
		{
			t = Pipe->PopFloat();
			
		}else if (Type == TYPE_INT)
		{
			t = Pipe->PopInt();
		}else{
			return false;
			
		}
		
		//调用c函数sin执行
		float64 n = tan(t);
		
		//然后把返回的结果压入到数据管道中
		Pipe->PushFloat(n);
		
		//最后表示执行成功
		return true; 
	} 
		
    
	return false;
}