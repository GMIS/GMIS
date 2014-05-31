/*
*
* author: ZhangHongBing(hongbing75@gmail.com)  
*/

#ifndef _LOGICOPERATOR_H__
#define _LOGICOPERATOR_H__

#include "../../TheorySpace/src/AbstractSpace.h"
#include "../../TheorySpace/src/Pipeline.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

// �߼������޶�������Ϊ����

namespace BASESPACE{
	
	bool Logical_Error(eElectron& A);      //0 ��δ֪���͵�������߼�������׳�һ���쳣
	bool Logical_Int(eElectron&  A);     //1
	bool Logical_Float(eElectron&  A);   //2
	
	class  CLogical_AND : public Mass  
	{
		static bool (*LOGICAL_OPERATOR[])(eElectron&  A);
	public:
		CLogical_AND(int64 ID=0):Mass(ID){};
		virtual ~CLogical_AND(){};
		
		virtual tstring      GetName(){ return _T("CLogical_AND");};
		

		//�����ⲿ���ͼ�飬�ڲ��������ͼ�飬ֻ���������Ͳ�����������
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
