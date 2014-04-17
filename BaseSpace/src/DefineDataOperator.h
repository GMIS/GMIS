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

#ifndef _DEFINEDATAOPERATOR_H_
#define _DEFINEDATAOPERATOR_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "AbstractSpace.h"
#include <stdio.h>
namespace BASESPACE{


//////////////////////////////////////////////////
class  CDefineInt: public Mass{
private:
    int64 m_Value;
public:
	CDefineInt(int64 V=0,int64 ID=0)
		:m_Value(V),Mass(ID){};
	virtual ~CDefineInt(){};

	virtual tstring      GetName(){
		TCHAR buf[100];
		_stprintf(buf,_T("CDefineInt64=%I64ld"),m_Value);
		return buf;
	};
	virtual Energy*  ToEnergy(){
		return new eINT(m_Value);
	};

	virtual bool     FromEnergy(Energy* E){
		if(E->EnergyType() != TYPE_INT)return false;
		eINT& d = *(eINT*)E;
		m_Value = d();
		return true;
	}
	

    virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
    virtual bool Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		Pipe->Push_Directly(new eINT(m_Value));   
      	return true;	
	};
};
	
//////////////////////////////////////////////////
class  CDefineFloat: public Mass{
private:
	float64 m_Value;
public:
	CDefineFloat(float64 V=0.0,int64 ID=0)
		:m_Value(V),Mass(ID){};
	virtual ~CDefineFloat(){};
	virtual tstring      GetName(){
		TCHAR buf[50];
		_stprintf(buf,_T("CDefineFloat64=%f"),m_Value);
		return buf;
	};

	virtual Energy*  ToEnergy(){
		return new eFLOAT(m_Value);
	};
	
	virtual bool     FromEnergy(Energy* E){
		if(E->EnergyType() != TYPE_FLOAT)return false;
		eFLOAT& d = *(eFLOAT*)E;
		m_Value = d();
		return true;
	}	

    virtual TypeAB  GetTypeAB(){ return 0x00000000;} 
    virtual bool Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		Pipe->Push_Directly(new eFLOAT(m_Value)); 
		return true;
	}
};		 

class  CDefineString: public Mass{
private:
	tstring m_Value;
public:
	CDefineString(tstring V=L'\0',int64 ID=0)
		:m_Value(V),Mass(ID){};
	virtual ~CDefineString(){};
	virtual tstring      GetName(){
		return _T("CDefineString=")+m_Value;
	};

	virtual Energy*  ToEnergy(){
		return new eSTRING(m_Value);
	};
	
	virtual bool     FromEnergy(Energy* E){
		if(E->EnergyType() != TYPE_STRING)return false;
		eSTRING& d = *(eSTRING*)E;
		m_Value = d();
		return true;
	}	

    TypeAB  GetTypeAB(){ return 0x00000000;} 
    virtual bool Do(Energy* E)
	{
		ePipeline* Pipe = (ePipeline*)E;
		Pipe->Push_Directly(new eSTRING(m_Value));  
		return true;
	};
};

}// namespace BASESPACE

#endif // _DEFINEDATAOPERATOR_H_