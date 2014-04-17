//                                                                               
//  Copyright 2002-2008  Zhang HongBing(hongbing75@gmail.com)                                   
//  All rights reserved.                                                         
//       
//  任何人无论有意或无意使用本文件/代码并不意味着得到任何权利的让渡。    
//                                                                           
//  本文件/代码仅允许个人在以下条件下任意使用:                               
//  - 仅用于非营利目的。                                                         
//  - 部分或完全基于本文件/代码的任何应用成果包括但不限于：编译所得软件、修改后  
//    文件/代码、讨论所得创意等等只能秘密享有，不得申请任何形式的专利或技术保护，
//    一旦非保密义务的第三方无论从何种途径获悉都有权按公共知识产权对待(不包括本文件/代码)。             
//  - 在基于本文件/代码的衍生文件中的显著位置保留本License
//                                                                               
//  THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT 
//  ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE 
//  DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE  
//  RISK OF USING THIS SOFTWARE.                                                 
//                                                                           
/////////////////////////////////////////////////////////////////////////////////

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
