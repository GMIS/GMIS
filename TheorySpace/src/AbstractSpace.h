
/*
* author: ZhangHongBing(hongbing75@gmail.com)  
*  
* ����࣬��������-�����Ĺ۵㹹�����ɴ�ʵ�������
*/


#ifndef _ABSTRACTSPACE_H_
#define _ABSTRACTSPACE_H_

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

	static const wchar_t*     BaseEnergyName[];
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
	//���������ռ�˺�������ã�����Ϊ˽��  
    //virtual bool   Do(AbstractSpace* Other){ return true;};
public:
	Energy(){};
	virtual ~Energy(){};
    virtual SPACE_TYPE  SpaceType(){ return ENERGY_SPACE;};
	virtual ENERGY_TYPE EnergyType() = 0;

	virtual Energy*  Clone(){ return NULL;};
	virtual void*    Value()const = 0;
	
	//����д��s�е��ַ���
    virtual void  ToString(AnsiString& s) = 0;  
	
	//����s�б��δ�����ַ��� 
	virtual uint32  FromString(AnsiString& s,uint32 pos=0) = 0; 

public:
    static const wchar_t*  GetTypeName(uint32 type);
    #define  TYPENAME(t) Energy::GetTypeName(t)

	virtual tstring      GetName(){ return GetTypeName(EnergyType());}

	/*���л�:

    Energy��ToString()��FromString()ʵ��ִ�е�����ݵ����л���

	��������ַ�ʽ���ô��ǿ���ֱ�ӿ�����������Ҫ����Ӳ���������Ӧ���������
	ȱ����Ч�ʱ��ֽ����Ե͡�

	����Tostring()����,ת�������ֺ�ĸ�ʽͨ��Ϊ: type@len@data
	����: typeΪ�������͵���д, ���ڻ��������Ͳ�����16��,���Կ�����һ��16�����ַ��ʾ
	      dataΪ�������,ͬ��ʹ���ַ�����ʾ.
		  lenΪdata�ַ���	 

	����FromString(string& s)����,����������ʽ��������data����ת���ɾ������ݡ�
	
	 �μ�Energyʵ��
    */

	/*���ַ�s�����������Ϊtype������Ϊlen�����Ϊdata��ϳ�һ����ݸ�ʽ��type@len@data
	*/
	static void  PrintString(AnsiString& s,int32 type,uint32 datalen, const char* data);

	/*ͬ�ϣ�ֻ������ݸ�ʽ��Ϊ��type@ID@NameLen@Name@len@data ����ePipeline
	*/
	static void PrintString(AnsiString& s,int32 type,int64 ID,tstring Name,uint32 datalen, const char* data);
    

    /*�����ַ�s��len���ַ�ת��Ϊ���� ע�⣺����ȷ����ʽ��ȷ������len<=20;
	*/
	static int64 StringToInt(const char* s, uint32 len);

	/*ͬ�ϣ���֪�ַ��ȣ���û�о����ʽ��飬len����С��20
	  ��������������correct = false ���� true
	*/
	static int64 RawStringToInt(const char* s, uint32 len,bool& correct);
	uint64 RawStringToUInt(const char* s, uint32 len,bool& correct);


    /*�����Ѿ�ȷ��s��ʽ��ȷ�����Ҽ��С��������len<=6;
	*/
	float64 StringToFloat(const char* s, uint32 len,int32 floatpoint);

	/*��s��posλ�ÿ�ʼ����ÿһ���ַ�����Ϊ�������ַ�@������󷵻����ֳ���,0Ϊ����
	*/
	int32 FindInt(AnsiString& s,uint32 pos, char ch='@');

	/*ͬ�ϣ��ҵ�����鸡��������ֳ��ȣ����Ѹ���λ�ñ�����floatpos��
	*/
	int32 FindFloat(AnsiString& s, uint32 pos, uint32& floatpos,char ch='@');
};


//////////////////////////////////////////////////////////////////////
// Mass Class
//////////////////////////////////////////////////////////////////////
enum  MASS_TYPE {
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
/*ͬ������Ϊ�ܱ����ģ�ԭ����������ͨ������������ֱ���໥����
	  ���Ǽ��ͨ�����������У������BOOL Do(Energy* E) = 0 ���
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

	/*�������������໥����
	 �ӳ���Ա�Ĺ۵�������Do������һ��ͳһ��ʽ������C���������E�ṩ��
	*/
	virtual bool Do(Energy* E)=0;  

	/*mass�����ܽ��ܵ�����������д��������?���μ����ľ���ʵ���Pipeline.h
	*/
	virtual TypeAB   GetTypeAB()=0;  
};



}// namespace ABSTRACT

#endif // _ABSTRACTSPACE_H_

