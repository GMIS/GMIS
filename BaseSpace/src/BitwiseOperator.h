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

#ifndef _BITWISEOPERATOR_H__
#define _BITWISEOPERATOR_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "AbstractSpace.h"
#include "Pipeline.h"

namespace BASESPACE{
	
	class  CBitwise_AND : public Mass  
	{
		
	public:
		CBitwise_AND(int64 ID=0):Mass(ID){};
		virtual ~CBitwise_AND(){};

		virtual tstring      GetName(){ return _T("CBitwise_AND");};
		
	
		/* 由于int32和uint32都可以做此运算，因此这里允许接受任何类型数据，
		   但内部会检查操作数是否为32位整数，否则返回false
		   以下操作类似。
		*/

		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	
	class  CBitwise_NOT : public Mass  
	{
		
	public:
		CBitwise_NOT(int64 ID=0):Mass(ID){};
		virtual ~CBitwise_NOT(){};		

		virtual tstring      GetName(){ return _T("CBitwise_NOT");};
		virtual TypeAB  GetTypeAB(){ return 0x20000000;} 
		virtual bool Do(Energy* E);
	};
	
	class  CBitwise_OR : public Mass  
	{
	public:
		CBitwise_OR(int64 ID=0):Mass(ID){};
		virtual ~CBitwise_OR(){};		

		virtual tstring      GetName(){ return _T("CBitwise_OR");};
		
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	
	class  CBitwise_XOR : public Mass 
	{
		
	public:
		CBitwise_XOR(int64 ID=0):Mass(ID){};
		virtual ~CBitwise_XOR(){};		
	
		virtual tstring      GetName(){ return _T("CBitwise_XOR");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	
	class  CLeft_Shift : public Mass  
	{
		
	public:
		CLeft_Shift(int64 ID=0):Mass(ID){};
		virtual ~CLeft_Shift(){};		

		virtual tstring      GetName(){ return _T("CLeft_Shift");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	
	class  CRight_Shift : public Mass  
	{
	public:
		CRight_Shift(int64 ID=0):Mass(ID){};
		virtual ~CRight_Shift(){};		
		
		virtual tstring      GetName(){return _T("CRight_Shift");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	
}; //namespace BASESPACE

#endif // !defined(_BITWISEOPERATOR_H__)
