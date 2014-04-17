/*
* author: ZhangHongBing(hongbing75@gmail.com)   
*/


#ifndef _TYPEDEF_H
#define _TYPEDEF_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <assert.h>
#include <string>
#include <vector>
#include <tchar.h>
#include <iostream> 
#include <sstream> 
#include <map>
#include <list>

#ifdef _WIN32
#define  WIN32_LEAN_AND_MEAN
#include <Windows.h>
#pragma warning(disable : 4786)

#else
#include <sys/types.h>
typedef int  BOOL;
#define TRUE  1
#define FALSE 0
#endif



typedef std::string  AnsiString;

#ifdef   _UNICODE  
#define   tstring   std::wstring  
#define   tcout     std::wcout
#define   tstringstream  std::wstringstream
#else  
#define   tstring   std::string  
#define   tcout     std::cout
#define   tstringstream  std::stringstream
#endif   


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


/////////////////////////////////////////////////////

/* 定义基本数据类型识别ID

  为了能进行基本的数据类型检查，我们用每4个bit表达一种数据类型，这样
  就限定了数据类型的总数为16种，是否够用取决与你把数据类型分的多细，
  实际上对于Final C对数据类型的理解应该以社会意义为主，比如不需要再关
  心长整数和短整数，可以统一用64位整数和64浮点数来处理数学计算。但具体
  如何分配数据类型还需要考虑整个系统的数据应用情况，比如把三维点或三维
  模型数据也作为单独的数据类型，目前暂时采用C语言的分类，
  
  WARNING: 如果要修改此处定义必须同时修改其它部分, 参见eAddtion.h
*/

typedef enum  ENERGY_TYPE {
    TYPE_NULL     =    0,  //在识别能量缩写时能与NULL有所区别
    TYPE_INT      =    1,
    TYPE_FLOAT    =    2,
    TYPE_STRING   =    3,
    TYPE_PIPELINE =    4, 
	TYPE_BLOB     =    5,
    TYPE_USER     =    6,  //用户定义的特殊数据类型      
	TYPE_END      =    16  //结束定义
}eType;

#define BASETYPE(TypeID) (TypeID==TYPE_INT || TypeID==TYPE_FLOAT)
/*
  类型缩写，每4个bit表达一种Energy Type，这样一个32位无符号整数可以
  一次表示8个数据的类型及其存储顺序。它将起的作用将类似与C语言函数
  里的参数检查。请参看ePipeline.h
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


	/*得到指定位置[0-7]的数据类型*/
	eType operator [](uint32 pos){
		assert(pos<8);
		uint32 mask = 0xF0000000 >> (pos*4);
        return (eType)(mask & m_TypeAB);
	}

	/*寻找数据类型，返回其位置 没有找到则返回-1；*/
	int32 Find(eType t){
		uint32 mask  = 0xF0000000;
		for(int i=0; i<8; i++)
		{
			if( (m_TypeAB & mask >> (i*4) ) == (uint32)t )return i;
		}
		return -1;
	}
};

//链接的C运行时库DLL类型,目前只包含win平台，其他平台可以继续添加
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

#endif _TYPEDEF_H