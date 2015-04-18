/*
* author: ZhangHongBing(hongbing75@gmail.com)
*/

#ifndef _TYPEDEF_H
#define _TYPEDEF_H


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
    TYPE_NULL     =    0,  
    TYPE_INT      =    1,
    TYPE_FLOAT    =    2,
    TYPE_STRING   =    3,
    TYPE_PIPELINE =    4, 
	TYPE_BLOB     =    5,
    TYPE_USER     =    6,  //Special user-defined data type    
	TYPE_END      =    16 
}eType;

#define BASETYPE(TypeID) (TypeID==TYPE_INT || TypeID==TYPE_FLOAT)

/*
  Type abbreviations, each 4 bit express one Energy Type,
  so a 32-bit unsigned integer can express 8 data types and their order. 
  It will play a role similar to the c functions parameter checking. 
  Please refer to ePipeline.h
 
*/

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

enum DLL_TYPE{
	DLL_INVALID = 0,
	DLL_VC6  = 60,
	DLL_VC6D = 61,
	DLL_VC7  = 70,
	DLL_VC7D = 71,
	DLL_VC8  = 80,
	DLL_VC8D = 81,
	DLL_VC9  = 90,
	DLL_VC9D = 91,
	DLL_VC10 = 100,
	DLL_VC10D= 101
};
} //namespace ABSTRACT

#endif //_TYPEDEF_H