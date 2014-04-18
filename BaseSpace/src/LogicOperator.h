/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _LOGICOPERATOR_H__
#define _LOGICOPERATOR_H__

#include "AbstractSpace.h"
#include "Pipeline.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

// 逻辑操作限定操作数为整数

namespace BASESPACE{
	
	bool Logical_Error(eElectron& A);      //0 对未知类型的数据做逻辑运算会抛出一个异常
	bool Logical_Int(eElectron&  A);     //1
	bool Logical_Float(eElectron&  A);   //2
	
	class  CLogical_AND : public Mass  
	{
		static bool (*LOGICAL_OPERATOR[])(eElectron&  A);
	public:
		CLogical_AND(int64 ID=0):Mass(ID){};
		virtual ~CLogical_AND(){};
		
		virtual tstring      GetName(){ return _T("CLogical_AND");};
		

		//忽略外部类型检查，内部会做类型检查，只有数字类型才能做此运算
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);		
	};
	
	class  CLogical_NOT : public Mass 
	{
		static bool (*LOGICAL_OPERATOR[])(eElectron&  A);
	public:
		CLogical_NOT(int64 ID=0):Mass(ID){};
		virtual ~CLogical_NOT(){};
		
	
		virtual tstring      GetName(){ return _T("CLogical_NOT");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	
	class  CLogical_OR : public Mass  
	{
		static bool (*LOGICAL_OPERATOR[])(eElectron&  A);
	public:
		CLogical_OR(int64 ID=0):Mass(ID){};
		virtual ~CLogical_OR(){};
	
		virtual tstring      GetName(){ return _T("CLogical_OR");};		
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 	
		virtual bool Do(Energy* E);
	};
	
	
}; //namespace BASESPACE
	
#endif // !defined(_LOGICOPERATOR_H__)
