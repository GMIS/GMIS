
/*
* author: ZhangHongBing(hongbing75@gmail.com)  
*  
* 根基类，采用能量-质量的观点构建，由此实例成万物
*/


#ifndef _ABSTRACTSPACE_H_
#define _ABSTRACTSPACE_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "Typedef.h"

  
using namespace ABSTRACT;
using namespace std;


namespace ABSTRACT { 


enum  SPACE_TYPE{     
		ENERGY_SPACE,
		MASS_SPACE, 
};

class CTimeStamp;
class CAbstractSpacePool;

class  AbstractSpace  
{

private:
	static CTimeStamp*          m_StaticTimer;
    static CAbstractSpacePool*  m_StaticSpacePool;

protected:
	AbstractSpace(){

	};
	AbstractSpace(CTimeStamp* Timer,CAbstractSpacePool* Pool)
	{
		m_StaticTimer = Timer;
		m_StaticSpacePool=Pool;
	}
public:	
	virtual ~AbstractSpace(){
		
	};

	static void InitTimer(CTimeStamp* Timer);
	static void InitSpacePool(CAbstractSpacePool* Pool);

	static int64 CreateTimeStamp();
	static CTimeStamp*  GetTimer();
    static CAbstractSpacePool* GetSpacePool();

    virtual SPACE_TYPE  SpaceType()=0;  
    virtual tstring GetName() = 0;

	//The interaction between space
	//virtual bool Do(AbstractSpace* Other) = 0;
};


//////////////////////////////////////////////////////////////////////
// Energy Class
//////////////////////////////////////////////////////////////////////



class  Energy : public AbstractSpace  
{
	static const int64     INTTABLE[21][10];
    static const float64   FLOATTABLE[][10];

	static const TCHAR*     BaseEnergyName[];
	/*= {
                     "TYPE_NULL",        // = 0,   	                 
		    		 "TYPE_INT",         // = 1,
		    		 "TYPE_FLOAT",       // = 2,
					 "TYPE_STRING",      // = 3,
					 "TYPE_PIPELINE",    // = 4,
					 "TYPE_BLOB",        // = 5,
					 "TYPE_USER"         // = 6,		
	};*/

public:
    static const char*     BaseEnergyChar;  // = "0123456"; used for data-string conversion 

    #define  TYPE_TO_CHAR(type) Energy::BaseEnergyChar[type]
    #define  CHAR_TO_TYPE(ch)   ch-'0'   
    #define  IsDataType(ch)    (ch>47 && ch<54)

private: 
	//对于能量空间此函数几乎不用，弱化为私有  
    //virtual bool   Do(AbstractSpace* Other){ return true;};
public:
	Energy(){};
	virtual ~Energy(){};
    virtual SPACE_TYPE  SpaceType(){ return ENERGY_SPACE;};
	virtual ENERGY_TYPE EnergyType() = 0;

	virtual void*       Value()const = 0;
	virtual Energy*     Clone() = 0;

	
	//返回写入s中的字符数
    virtual void  ToString(AnsiString& s) = 0;  
	
	//返回s中本次处理的字符数 
	virtual uint32  FromString(AnsiString& s,uint32 pos=0) = 0; 

public:
    static const TCHAR*  GetTypeName(uint32 type);
    #define  TYPENAME(t) Energy::GetTypeName(t)

	virtual tstring      GetName(){ return GetTypeName(EnergyType());}

	/*序列化:

    Energy的ToString()和FromString()实际执行的是数据的序列化。

	这里采用字符串方式，好处是可以直接看懂，灵活，不需要考虑硬件差别，能适应各种情况，
	缺点是效率比字节流稍低。

	对于Tostring()函数,转换成文字后的格式通常为: type@len@data
	其中: type为能量类型的缩写, 由于基本能量类型不超过16个,所以可以用一个16进制字符表示
	      data为数据内容,同样使用字符串来表示.
		  len为data字符串长度	 

	对于FromString(string& s)函数,它根据上述格式的描述把data部分转换成具体的数据。
	
	 参见Energy实例。
    */

	/*在字符串s加入数据类型为type，长度为len，数据为data结合成一种数据格式：type@len@data
	*/
	static void  PrintString(AnsiString& s,int32 type,uint32 datalen, const char* data);

	/*同上，只不过数据格式改为：type@ID@NameLen@Name@len@data 用于ePipeline
	*/
	static void PrintString(AnsiString& s,int32 type,int64 ID,tstring Name,uint32 datalen, const char* data);
    

    /*把在字符串s的len个字符转换为整数 注意：必须确保格式正确，并且len<=20;
	*/
	static int64 StringToInt(const char* s, uint32 len);

	/*同上，已知字符串长度，但没有经过格式检查，len必须小于20
	  返回整数，如果错误correct = false 否则 true
	*/
	static int64 RawStringToInt(const char* s, uint32 len,bool& correct);
	uint64 RawStringToUInt(const char* s, uint32 len,bool& correct);


    /*必须已经确认s格式正确，并且检查小数点后数字len<=6;
	*/
	float64 StringToFloat(const char* s, uint32 len,int32 floatpoint);

	/*从s的pos位置开始查找每一个字符，检查其为整数，以字符@结束，最后返回数字长度,0为错误
	*/
	int32 FindInt(AnsiString& s,uint32 pos, char ch='@');

	/*同上，找到并检查浮点数，返回数字长度，并把浮点位置保存在floatpos里
	*/
	int32 FindFloat(AnsiString& s, uint32 pos, uint32& floatpos,char ch='@');
};


//////////////////////////////////////////////////////////////////////
// Mass Class
//////////////////////////////////////////////////////////////////////
typedef enum  MASS_TYPE {
    MASS_VOID        =    0,  
	MASS_ELEMENT     =    1,  
	MASS_OBJECT      =    2,
	MASS_MODEL       =    3,
	MASS_SYSTEM      =    4,
	MASS_USER        =    5
};

class  Mass : public AbstractSpace  
{
protected: 
	Mass(CTimeStamp* Timer,CAbstractSpacePool* Pool):
	AbstractSpace(Timer,Pool)
	{
	}
/*同样弱化为受保护的，原因是质量体通常不与质量体直接相互作用
	  而是间接通过能量来进行，因此用BOOL Do(Energy* E) = 0 替代
	*/
    //virtual bool  Do(AbstractSpace* Other){ return true;};
public:
	int64    m_ID;
public:
	Mass():m_ID(0){};
	Mass(int64 ID):m_ID(ID){};
	virtual ~Mass(){};

    virtual SPACE_TYPE SpaceType(){ return MASS_SPACE;};
	virtual MASS_TYPE  MassType(){ return MASS_VOID;}; 

    virtual Energy*  ToEnergy(); 
	virtual bool     FromEnergy(Energy* E); 

	/*质量与能量的相互作用
	 从程序员的观点来看，Do类似于一个统一形式的任意C函数，参数由E提供。
	*/
	virtual bool Do(Energy* E)=0;  

	/*mass类所能接受的能量类型缩写（即参数表），参见此类的具体实例和Pipeline.h
	*/
	virtual TypeAB   GetTypeAB()=0;  
};



}// namespace ABSTRACT

#endif // _ABSTRACTSPACE_H_

