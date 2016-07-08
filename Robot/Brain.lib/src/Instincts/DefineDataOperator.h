/*
* 
* author: ZhangHongBing(hongbing75@gmail.com)  
*/


#ifndef _DEFINEDATAOPERATOR_H_
#define _DEFINEDATAOPERATOR_H_


#include "PhysicSpace.h"
#include <stdio.h>
	
	
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


#endif // _DEFINEDATAOPERATOR_H_