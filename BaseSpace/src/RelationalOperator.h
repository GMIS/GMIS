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

	// ���ǰѺ���ָ�뼯�ɵ������У�ƥ�亯��Ҳ���Լ��ɣ������κ�����ֻҪ��ƥ�亯�������Ա�������
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
	
	
	// ���ǰѺ���ָ�뼯�ɵ������У�ƥ�亯��Ҳ���Լ��ɣ������κ�����ֻҪ��ƥ�亯�������Ա�������
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
	
		
	// ���ǰѺ���ָ�뼯�ɵ������У�ƥ�亯��Ҳ���Լ��ɣ������κ�����ֻҪ��ƥ�亯�������Ա�������
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
