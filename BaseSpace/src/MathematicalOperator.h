/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


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
