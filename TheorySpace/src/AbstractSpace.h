
/*
* author: ZhangHongBing(hongbing75@gmail.com)  
*  
*
* root base class with inspirations from energy-mass viewpoint, instance it to get everything
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
	//hardly use for the Energy class' instances,so weakened it as private 
    //virtual bool   Do(AbstractSpace* Other){ return true;};
public:
	Energy(){};
	virtual ~Energy(){};
    virtual SPACE_TYPE  SpaceType(){ return ENERGY_SPACE;};
	virtual ENERGY_TYPE EnergyType() = 0;

	virtual Energy*  Clone(){ return NULL;};
	virtual void*    Value()const = 0;
	
    virtual void  ToString(AnsiString& s) = 0;  
	
	//returns the number of written chars in s
	virtual uint32  FromString(AnsiString& s,uint32 pos=0) = 0; 

public:
    static const wchar_t*  GetTypeName(uint32 type);
    #define  TYPENAME(t) Energy::GetTypeName(t)

	virtual tstring      GetName(){ return GetTypeName(EnergyType());}

		
	/* Serialization:
	the ToString () and FromString ()  are in charge of the serialization of data.

	String used here to store data,the benefits include directly reading ,flexible using,no need to consider hardware differences, and adapting a variety of situations,but disadvantage is that the efficiency slightly lower than using streams of bytes.

	For Tostring () function,the converted text format are: type@Len@data

	Where: 
	Type is the abbreviation of energy type, as the type amount does not exceed 16, so you can actually use a 16 hexadecimal character to represent it
	Data for the data content, and also use a string to represent.
	Len for string data length

	For FromString (string& s) function, which  converted the text to a specific data according to the above format description.

	See also Energy class instances.
	*/

	/*Serialize data to string, format:type@len@data
	*/
	static void  PrintString(AnsiString& s,int32 type,uint32 datalen, const char* data);

	/*ditto, but the format: type@ID@NameLen@Name@len@data, mainly for ePipeline
	*/
	static void PrintString(AnsiString& s,int32 type,int64 ID,tstring Name,uint32 datalen, const char* data);
    

    /*Convert len chars of the string s to an integer, Note: must ensure that the format is correct an len<20
	*/
	static int64 StringToInt(const char* s, uint32 len);

	/*ditto, but without format checking,if there is any error the correct=false or true
	*/
	static int64 RawStringToInt(const char* s, uint32 len,bool& correct);
	uint64 RawStringToUInt(const char* s, uint32 len,bool& correct);


    /*must ensure the string s' format is correct,and the digits of decimal point must <= 6
	*/
	float64 StringToFloat(const char* s, uint32 len,int32 floatpoint);

	/*Starting from the pos of s to check each char,finds whether there is any integer occurred until encounters '@',returns the integer length or 0
	*/
	int32 FindInt(AnsiString& s,uint32 pos, char ch='@');

	/*ditto, finds the float and return its length, the decimal point position will be save in DecimalPos
	*/
	int32 FindFloat(AnsiString& s, uint32 pos, uint32& DecimalPos,char ch='@');
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
	//hardly use for the mass class' instances,so weakened it as private 
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

	/*The interaction between mass and energy,but from a programmer's point of view,
	 Do(Energy* E) like a unified form of arbitrary c functions whose parameters provided by E
	*/
	virtual bool Do(Energy* E)=0;  

	/*returns the energies abbreviation(the parameters list) that the mass can accept,
	 reference to the specific instance of this class and Pipeline.h
	*/
	virtual TypeAB   GetTypeAB()=0;  
};



}// namespace ABSTRACT

#endif // _ABSTRACTSPACE_H_

