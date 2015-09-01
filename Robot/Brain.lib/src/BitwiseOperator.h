/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _BITWISEOPERATOR_H__
#define _BITWISEOPERATOR_H__


#include "PhysicSpace.h"
#include "Pipeline.h"

	
	class  CBitwise_AND : public Mass  
	{
		
	public:
		CBitwise_AND(int64 ID=0):Mass(ID){};
		virtual ~CBitwise_AND(){};

		virtual tstring      GetName(){ return _T("CBitwise_AND");};
		

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
	
#endif // !defined(_BITWISEOPERATOR_H__)
