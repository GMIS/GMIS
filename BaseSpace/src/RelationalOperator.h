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

#ifndef _RELATIONALOPERATOR_H__
#define _RELATIONALOPERATOR_H__

#include "AbstractSpace.h"
#include "Pipeline.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace BASESPACE{

	/////////////////////////////////////////////////////////////
	class  CEquivalent : public Mass 
	{
	public:
		CEquivalent(int64 ID=0):Mass(ID){};
		virtual ~CEquivalent(){};		

		virtual tstring      GetName(){ return _T("CEquivalent");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};	

	// 考虑把函数指针集成到运算中，匹配函数也可以集成，这样任何数据只要有匹配函数都可以被操作。
	/////////////////////////////////////////////////////////////
	class  CGreaterThan : public Mass 
	{
	public:
		CGreaterThan(int64 ID=0):Mass(ID){};
		virtual ~CGreaterThan(){};


		virtual tstring       GetName(){ return _T("CGreaterThan");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};

	class  CGreaterThanOrEqual : public Mass 
	{
	public:
		CGreaterThanOrEqual(int64 ID=0):Mass(ID){};
		virtual ~CGreaterThanOrEqual(){};

		virtual tstring      GetName(){ return _T("CGreaterThanOrEqual");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	
	
	// 考虑把函数指针集成到运算中，匹配函数也可以集成，这样任何数据只要有匹配函数都可以被操作。
	/////////////////////////////////////////////////////////////
	class  CLessThan : public Mass 
	{
	public:
		CLessThan(int64 ID=0):Mass(ID){};
		virtual ~CLessThan(){};

		virtual tstring      GetName(){ return _T("CLessThan");};
		
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	
		
	// 考虑把函数指针集成到运算中，匹配函数也可以集成，这样任何数据只要有匹配函数都可以被操作。
	/////////////////////////////////////////////////////////////
	class  CLessThanOrEqual : public Mass 
	{
	public:
		CLessThanOrEqual(int64 ID=0):Mass(ID){};
		virtual ~CLessThanOrEqual(){};


		virtual tstring  GetName(){ return _T("CLessThanOrEqual");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	
	class  CNotEquivalent : public Mass
	{
	public:
		CNotEquivalent(int64 ID=0):Mass(ID){};
		virtual ~CNotEquivalent(){};

		virtual tstring      GetName(){ return _T("CNotEquivalent");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
}//namespace BASESPACE

#endif // !defined(_RELATIONALOPERATOR_H__)
