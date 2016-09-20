#ifndef _ENERGY_H_
#define _ENERGY_H_

#include "AbstractSpace.h"


namespace ABSTRACT{


class  Energy : public AbstractSpace  
{
	static const int64     INTTABLE[21][10];
    static const float64   FLOATTABLE[][10];

	static const wchar_t*     BaseEnergyName[];

public:
    static const char*     BaseEnergyChar;  // = "0123456789abcdef"; 

    #define  TYPE_TO_CHAR(type) Energy::BaseEnergyChar[type]
    #define  CHAR_TO_TYPE(ch)  (ch >47 && ch <58)?ch-'0': ch- 'a'+10 
    #define  IsDataType(ch)    ((ch >47 && ch <58)|| (ch>96 && ch <103))

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
	//pos point to the next data position
	virtual bool  FromString(AnsiString& s,uint32& pos) = 0; 

public:
    static const wchar_t*  GetTypeName(uint32 type);
    #define  TYPENAME(t) Energy::GetTypeName(t)

	virtual tstring  GetName(){ return GetTypeName(EnergyType());}

		
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

}//end namespace

#endif

