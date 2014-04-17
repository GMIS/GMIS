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
		
	
		/* ����int32��uint32�������������㣬���������������κ��������ݣ�
		   ���ڲ�����������Ƿ�Ϊ32λ���������򷵻�false
		   ���²������ơ�
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
