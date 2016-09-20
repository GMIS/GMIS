/*
* author: ZhangHongBing(hongbing75@gmail.com)
*/

#ifndef _TYPEDEF_H
#define _TYPEDEF_H

#include "vld.h"

#include <assert.h>
#include <string>
#include <vector>
#include <iostream> 
#include <sstream> 
#include <map>
#include <list>
#include <stdio.h>



#ifdef _WIN32

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif 

#define  WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#pragma warning(disable : 4786)

#else
#include <sys/types.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>

typedef int  BOOL;
#define TRUE  1
#define FALSE 0
#define SOCKET_ERROR -1
#define _T(x) L##x

#endif



typedef std::string  AnsiString;
#define tstring      std::wstring 


namespace ABSTRACT{

///////////////////////////////////////////////////////
typedef signed char		       int8;
typedef unsigned char	       uint8;
typedef short			       int16;
typedef unsigned short	       uint16;
typedef int				       int32;
typedef unsigned int	       uint32;

#ifdef _MSC_EXTENSIONS
    typedef __int64			   int64;
    typedef unsigned __int64   uint64;
#else
    typedef long long		   int64;
    typedef unsigned long long uint64;
#endif

typedef float	           	   float32;
typedef double		           float64;

const float EPSINON = 0.00001f;


#ifdef _WIN32
#define int64toa(t,buf) sprintf_s(buf,"%I64ld",(int64)t)
#define uint64toa(t,buf) sprintf_s(buf,"%I64ud",(int64)t)
#define atoint64(buf)  _atoi64(buf)
#define SLEEP_MILLI(n)  Sleep(n)
#else
#define int64toa(t,buf) ::sprintf(buf,"%lld",(int64)t)
#define uint64toa(t,buf) ::sprintf(buf,"%llu",(int64)t)
#define atoint64(buf)  ::atoll(buf)
#define SLEEP_MILLI(n)  ::usleep(n*1000)
#endif


/////////////////////////////////////////////////////


/* Defines the base data type ID

  For base data type checking ,we use 4 bit to express a data type, so the total is 16
  
  WARNING: If you want to modify the definition here,you  must also modify other parts, see eAddtion.h
*/
typedef enum  ENERGY_TYPE {
    TYPE_NULL     =    1,  
    TYPE_INT      =    2,
    TYPE_FLOAT    =    3,
    TYPE_STRING   =    4,
    TYPE_PIPELINE =    5, 
	TYPE_BLOB     =    6,
    TYPE_USER     =    15  //Special user-defined data type    
}eType;

#define BASETYPE(TypeID) (TypeID==TYPE_INT || TypeID==TYPE_FLOAT)

/*
  Type abbreviations, each 4 bit express one Energy Type,
  so a 32-bit unsigned integer can express 8 data types and their order. 
  It will play a role similar to the c functions parameter checking. 
  Please refer to ePipeline.h
 
*/

#define  PARAM_TYPE1(a)					0|a<<28
#define  PARAM_TYPE2(a,b)				0|a<<28|b<<24
#define  PARAM_TYPE3(a,b,c)				0|a<<28|b<<24|c<<20
#define  PARAM_TYPE4(a,b,c,d)			0|a<<28|b<<24|c<<20|d<<16
#define  PARAM_TYPE5(a,b,c,d,e)			0|a<<28|b<<24|c<<20|d<<16|e<<12
#define  PARAM_TYPE6(a,b,c,d,e,f)		0|a<<28|b<<24|c<<20|d<<16|e<<12|f<<8
#define  PARAM_TYPE7(a,b,c,d,e,f,g)		0|a<<28|b<<24|c<<20|d<<16|e<<12|f<<8|g<<4
#define  PARAM_TYPE8(a,b,c,d,e,f,g,h)	0|a<<28|b<<24|c<<20|d<<16|e<<12|f<<8|g<<4|h

class  TypeAB {
public:
	uint32  m_TypeAB;
public:
	TypeAB(uint32 TypeAB=0):m_TypeAB(TypeAB){};
	TypeAB(ENERGY_TYPE Type):m_TypeAB(Type){};
	
	TypeAB& operator = (const TypeAB& t){
		m_TypeAB = t.m_TypeAB;
		return *this;
	};
    operator uint32(){ return m_TypeAB;};
	uint32& operator()(){return m_TypeAB;};

	bool operator == (const eType& t){
		return m_TypeAB == (uint32)t;
	}

	TypeAB& operator = (const eType& t){
		m_TypeAB = (uint32)t;
		return *this;
	};

	TypeAB& operator = (const uint32& t){
		m_TypeAB = t;
		return *this;
	};

	TypeAB& operator |= (const uint32& t){
		m_TypeAB |= t;
		return *this;
	};


	eType operator [](uint32 pos){
		assert(pos<8);
		uint32 mask = 0xF0000000 >> (pos*4);
        return (eType)(mask & m_TypeAB);
	}

	int32 Find(eType t){
		uint32 mask  = 0xF0000000;
		for(int i=0; i<8; i++)
		{
			if( (m_TypeAB & mask >> (i*4) ) == (uint32)t )return i;
		}
		return -1;
	}
};

tstring Format1024(const wchar_t* Format, ...);


} //namespace ABSTRACT

#endif //_TYPEDEF_H