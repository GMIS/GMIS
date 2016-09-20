/*
*author: ZhangHongBing(hongbing75@gmail.com)
*
*Some examples of Energy, which are the encapsulation of basic data types
*/

#ifndef _BASEENERGY_H_
#define _BASEENERGY_H_ 


#include "Energy.h"
#include "ABSpacePool.h"

namespace ABSTRACT{


// define eNULL
////////////////////////////////////////////////////////////////
/*
eNULL can be used to ensure the input and output data's testability of a Mass.

For example, if a Mass instance need to output three data, even if it is not many data,
it can use eNULL to replace,otherwise next Mass instance will fall into uncertainty 
when it need to get three data from ePipeline

*/

class  eNULL : public Energy
{
	friend class ePipeline;
public:
	eNULL(){};
	virtual ~eNULL(){}; 		
    
	eType EnergyType(){
		return TYPE_NULL;
	};
    void* Value()const{
		return NULL; 
	};
	Energy* Clone(){
		return  new eNULL;
	};


public:  
	//0@0@
	void ToString(AnsiString& s); //NOTE: to utf8
	bool  FromString(AnsiString& s,uint32& pos); //from utf8

};


///////////////////////////////////////////////////////////////

class  eINT : public Energy 
{
	friend class ePipeline;
	int64  m_Value;

    SUPPORT_ABSTRACT_SAPCE_POOL(eINT);
public:
	eINT():m_Value(0){

	};
	eINT(const int64 Int64):m_Value(Int64){

	};
	virtual ~eINT(){

	};

    operator int64(){
		return m_Value;
	};
	int64& operator()(){
		return m_Value;
	};

	eType EnergyType(){
		return TYPE_INT;
	};
	void* Value()const{
		return (void*)&m_Value;
	};
	Energy*  Clone(){ 
		return new eINT(m_Value);
	};

	void ToString(AnsiString& s);
	bool  FromString(AnsiString& s,uint32& pos);
	
};

// define eFLOAT64
////////////////////////////////////////////////////////////////////
class  eFLOAT : public Energy 
{
	friend class ePipeline;

	float64  m_Value; 
    SUPPORT_ABSTRACT_SAPCE_POOL(eFLOAT);
public:
	eFLOAT():m_Value(0){

	};
	eFLOAT(const float64 V):m_Value(V){

	};
	virtual ~eFLOAT(){

	};

    operator float64(){
		return m_Value;
	};
	float64& operator()(){
		return m_Value;
	};

	eType EnergyType(){
		return TYPE_FLOAT;
	};
    void* Value()const{
		return (void*)&m_Value;
	};
	Energy*  Clone(){ 
		return new eFLOAT(m_Value);
	};

	void ToString(AnsiString& s);
	bool  FromString(AnsiString& s,uint32& pos);	

};
// define eSTRING 
////////////////////////////////////////////////////////////////
class eSTRING:public Energy
{
	friend class ePipeline;

	tstring   m_Value;
	SUPPORT_ABSTRACT_SAPCE_POOL(eSTRING);
public:
	eSTRING():m_Value(){

	};
	eSTRING(const tstring &V):m_Value(V){

	};
	eSTRING(const wchar_t* s):m_Value(s){

	};
	virtual ~eSTRING(){

	};

    operator tstring(){
		return m_Value;
	};
	tstring& operator()(){
		return m_Value;
	};

	eType EnergyType(){
		return TYPE_STRING;
	};
	void* Value()const{
		return (void*)&m_Value;
	};
	Energy*  Clone(){
		return (new eSTRING(m_Value));
	};

	//Note: it will be convert into utf8,no matter whether the m_Value is unicode
	void ToString(AnsiString& s);
	
	//supposing the string s was utf8
	bool  FromString(AnsiString& s,uint32& pos);

};


//using std::string as carrier to handle binary data
////////////////////////////////////////////////////////////////

class  eBLOB : public Energy 
{
	friend class ePipeline;
	
	AnsiString  m_Value; 
    SUPPORT_ABSTRACT_SAPCE_POOL(eBLOB);
public:
	eBLOB():m_Value(){

	};
	eBLOB(const char* buf,int32 Len):m_Value(buf,Len){

	};
	virtual ~eBLOB(){

	};
	
    operator AnsiString(){
		return m_Value;
	};
	AnsiString& operator()(){
		return m_Value;
	};
	
	eType EnergyType(){
		return TYPE_BLOB;
	};
	void* Value()const{
		return (void*)&m_Value;
	};

	Energy*  Clone(){
		return new eBLOB(m_Value.c_str(),m_Value.size());
	};
	
	//Note: it will be convert into utf8,no matter whether the m_Value is unicode
	void ToString(AnsiString& s);
	
	//supposing the string s was utf8
	bool  FromString(AnsiString& s,uint32& pos);
};

} //end namespace ABSTRACT
 
#endif // _BASEENERGY_H_