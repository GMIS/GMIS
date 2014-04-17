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

/* ���������������ʶ��ID

  Ϊ���ܽ��л������������ͼ�飬������ÿ4��bit���һ���������ͣ�����
  ���޶����������͵�����Ϊ16�֣��Ƿ���ȡ��������������ͷֵĶ�ϸ��
  ʵ���϶���Final C���������͵����Ӧ�����������Ϊ�������粻��Ҫ�ٹ�
  �ĳ������Ͷ�����������ͳһ��64λ������64��������������ѧ���㡣������
  ��η����������ͻ���Ҫ��������ϵͳ������Ӧ��������������ά�����ά
  ģ������Ҳ��Ϊ�������������ͣ�Ŀǰ��ʱ����C���Եķ��࣬
  
  WARNING: ���Ҫ�޸Ĵ˴��������ͬʱ�޸���������, �μ�eAddtion.h
*/

typedef enum  ENERGY_TYPE {
    TYPE_NULL     =    0,  //��ʶ��������дʱ����NULL��������
    TYPE_INT      =    1,
    TYPE_FLOAT    =    2,
    TYPE_STRING   =    3,
    TYPE_PIPELINE =    4, 
	TYPE_BLOB     =    5,
    TYPE_USER     =    6,  //�û������������������      
	TYPE_END      =    16  //��������
}eType;

#define BASETYPE(TypeID) (TypeID==TYPE_INT || TypeID==TYPE_FLOAT)
/*
  ������д��ÿ4��bit���һ��Energy Type������һ��32λ�޷�����������
  һ�α�ʾ8�����ݵ����ͼ���洢˳������������ý�������C���Ժ���
  ��Ĳ�����顣��ο�ePipeline.h
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


	/*�õ�ָ��λ��[0-7]����������*/
	eType operator [](uint32 pos){
		assert(pos<8);
		uint32 mask = 0xF0000000 >> (pos*4);
        return (eType)(mask & m_TypeAB);
	}

	/*Ѱ���������ͣ�������λ�� û���ҵ��򷵻�-1��*/
	int32 Find(eType t){
		uint32 mask  = 0xF0000000;
		for(int i=0; i<8; i++)
		{
			if( (m_TypeAB & mask >> (i*4) ) == (uint32)t )return i;
		}
		return -1;
	}
};

//���ӵ�C����ʱ��DLL����,Ŀǰֻ����winƽ̨������ƽ̨���Լ������
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