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

#ifndef _MATHEMATICALOPERATOR_H__
#define _MATHEMATICALOPERATOR_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "AbstractSpace.h"
#include "Pipeline.h"

namespace BASESPACE{
	
	
	class  CAddition : public Mass 
	{		
	public:
		CAddition(int64 ID=0):Mass(ID){};
		virtual ~CAddition(){};

		virtual tstring      GetName(){ return _T("CAddition");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		
		virtual bool Do(Energy* E);
	};
	
		
	////////////////////////////////////////////////////////////////////////
	class  CSubtraction : public Mass  
	{
		
	public:
		CSubtraction(int64 ID=0):Mass(ID){};
		virtual ~CSubtraction(){};

		virtual tstring      GetName(){ return _T("CSubtraction");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
		
	class  CMultiplication : public Mass  
	{
	public:
		CMultiplication(int64 ID=0):Mass(ID){};
		virtual ~CMultiplication(){};

		virtual tstring      GetName(){ return _T("CMultiplication");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	
	void Div_Int(eElectron&  A, eElectron&  B);    //1
	void Div_Float(eElectron&  A, eElectron&  B);  //2
	
	class  CDivision : public Mass
	{
	public:
		CDivision(int64 ID=0):Mass(ID){};
		virtual ~CDivision(){};

		virtual tstring      GetName(){ return _T("CDivision");};
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		
		virtual bool Do(Energy* E);
	};
	
	class  CModulus: public Mass  
	{
	public:
		CModulus(int64 ID=0):Mass(ID){};
		virtual ~CModulus(){};


		virtual tstring      GetName(){ return _T("CModulus");};
		
		virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
		virtual bool Do(Energy* E);
	};
	

}//namespace BASESPACE

#endif // !defined(_MATHEMATICALOPERATOR_H__)
