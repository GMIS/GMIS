/*
*author: ZhangHongBing(hongbing75@gmail.com)  
*  
*  һЩ������Energyʵ����Ҳ���Ƕ��������������
*  Some of the basic examples of Energy, which is the basic definition of data types 
*/


#ifndef _BaseEnergy_H_
#define _BaseEnergy_H_ 

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


#include "AbstractSpace.h"
#include "AbstractSpacePool.h"


namespace ABSTRACT{


// define eNULL
////////////////////////////////////////////////////////////////
/*
eNULL����ȷ��һ������������������ݵĿɲ��ԣ�
���磬���һ��Mass��ʵ���涨��Ҫ���3�����ݣ���ô��ʹ��û��3������������
Ҳ����ʹ��eNULL����䣬�������Massʵ��ȡ��ePipeline�������ʱ���������
*/

class  eNULL : public Energy
{

	friend class ePipeline;
public:
	eNULL(){};
	virtual ~eNULL(){}; 		
    
	eType EnergyType(){return TYPE_NULL;};
    void* Value()const{	return NULL; };
	Energy* Clone(){return NULL;};


public:  
	//0@0@
	void ToString(AnsiString& s); //NOTE: to utf8
	uint32  FromString(AnsiString& s,uint32 pos); //from utf8

};


///////////////////////////////////////////////////////////////

class  eINT : public Energy 
{
	friend class ePipeline;
	int64  m_Value;

    SUPPORT_ABSTRACT_SAPCE_POOL(eINT);
public:
	eINT():m_Value(0){};
	eINT(const int64 &Int64):m_Value(Int64){};
	virtual ~eINT(){};

    operator int64(){return m_Value;};
	int64& operator()(){return m_Value;};

	eType EnergyType(){return TYPE_INT;};
	void* Value()const{ return (void*)&m_Value;};
	Energy*  Clone(){ return new eINT(m_Value);};

	void ToString(AnsiString& s);
	uint32  FromString(AnsiString& s, uint32 pos);
	
};

// define eFLOAT64
////////////////////////////////////////////////////////////////////
class  eFLOAT : public Energy 
{
	friend class ePipeline;

	float64  m_Value; 
    SUPPORT_ABSTRACT_SAPCE_POOL(eFLOAT);
public:
	eFLOAT():m_Value(0){};
	eFLOAT(const float64 &V):m_Value(V){};
	virtual ~eFLOAT(){};

    operator float64(){return m_Value;};
	float64& operator()(){return m_Value;};

	eType EnergyType(){return TYPE_FLOAT;};
    void* Value()const{return (void*)&m_Value;};
	Energy*  Clone(){ return new eFLOAT(m_Value);}

	void ToString(AnsiString& s);
	uint32  FromString(AnsiString& s,uint32 pos);	

};

// define eSTRING 
////////////////////////////////////////////////////////////////

class  eSTRING : public Energy 
{
	friend class ePipeline;

	tstring  m_Value; 
    SUPPORT_ABSTRACT_SAPCE_POOL(eSTRING);
public:
	eSTRING():m_Value(){};
	eSTRING(const tstring &V):m_Value(V){};
	eSTRING(const TCHAR* s):m_Value(s){};
	virtual ~eSTRING(){};

    operator tstring(){return m_Value;};
	tstring& operator()(){return m_Value;};

	eType EnergyType(){return TYPE_STRING;};
	void* Value()const{return (void*)&m_Value;};
	Energy*  Clone(){return (new eSTRING(m_Value));};

	//ע�⣺Ϊ��ͳһ����m_Value�Ƿ�unicode����ת����utf8��ʽ
	void ToString(AnsiString& s);
	//ͬ��s����Ϊutf8��ʽ
	uint32  FromString(AnsiString& s,uint32 pos=0);

	static AnsiString  UTF16TO8(const std::wstring& s);

     //���s����utf8��Ҳ�᳢����default codepageʹ��mbstowcs
	static std::wstring UTF8TO16(const AnsiString& s);

};


// ��std::stringΪ���崦�����������
////////////////////////////////////////////////////////////////

class  eBLOB : public Energy 
{
	friend class ePipeline;
	
	AnsiString  m_Value; 
    SUPPORT_ABSTRACT_SAPCE_POOL(eBLOB);
public:
	eBLOB():m_Value(){};
	eBLOB(const char* buf,int32 Len):m_Value(buf,Len){};
	virtual ~eBLOB(){};
	
    operator AnsiString(){return m_Value;};
	AnsiString& operator()(){return m_Value;};
	
	eType EnergyType(){return TYPE_BLOB;};
	void* Value()const{return (void*)&m_Value;};

	Energy*  Clone(){return new eBLOB(m_Value.c_str(),m_Value.size());};
	
	//ע�⣺Ϊ��ͳһ����m_Value�Ƿ�unicode����ת����utf8��ʽ
	void ToString(AnsiString& s);
	//ͬ��s����Ϊutf8��ʽ
	uint32  FromString(AnsiString& s,uint32 pos);
};

} //end namespace ABSTRACT
 
#endif // _BaseEnergy_H_