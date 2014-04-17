//                                                                               
//  Copyright 2002-2008  Zhang HongBing(hongbing75@gmail.com)                                   
//  All rights reserved.                                                         
//       
//  �κ����������������ʹ�ñ��ļ�/���벢����ζ�ŵõ��κ�Ȩ�����öɡ�    
//                                                                           
//  ���ļ�/����������������������������ʹ��:                               
//  - �����ڷ�Ӫ��Ŀ�ġ�                                                         
//  - ���ֻ���ȫ���ڱ��ļ�/������κ�Ӧ�óɹ������������ڣ���������������޸ĺ�  
//    �ļ�/���롢�������ô���ȵ�ֻ���������У����������κ���ʽ��ר������������
//    һ���Ǳ�������ĵ��������۴Ӻ���;����Ϥ����Ȩ������֪ʶ��Ȩ�Դ�(���������ļ�/����)��             
//  - �ڻ��ڱ��ļ�/����������ļ��е�����λ�ñ�����License
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

// �߼������޶�������Ϊ����

namespace BASESPACE{
	
	bool Logical_Error(eElectron& A);      //0 ��δ֪���͵��������߼�������׳�һ���쳣
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
