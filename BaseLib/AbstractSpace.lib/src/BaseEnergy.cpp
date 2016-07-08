

#include "ABSpacePool.h"
#include "BaseEnergy.h"
#include <stdlib.h>
#include <stdio.h>
//#include "ConvertUTF.h"
#include <locale>  
#include <codecvt> 

std::wstring UTF8_To_UTF16(const std::string &source)  
{  

	try  
	{  
		static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt;  
		return cvt.from_bytes(source);  
	}  
	catch (std::range_error &e)  
	{  
		return std::wstring();  
	}

}  

std::string UTF16_To_UTF8(const std::wstring &source)  
{  
	try  
	{  
		static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cvt;  
		return cvt.to_bytes(source);  
	}  
	catch (std::range_error &)  
	{  
		return std::string();  
	}  
}  

namespace ABSTRACT{
	
	//0@0@
	void eNULL::ToString(AnsiString& s){ 
		s += TYPE_TO_CHAR(TYPE_NULL);
		s.append("@0@");
	};
	
	//For NUll Type,the form is: type@0@, without len@data part
	bool  eNULL::FromString(AnsiString& s,uint32& pos){
		const char* ch = &s[pos];
		if( *ch == '0' && *(++ch) == '@' && *(++ch) =='0' && *(++ch) =='@'){
			pos+=4;
			return true;
		}
		return false;
	};
	

	void eINT::ToString(AnsiString& s){
		char buf[30];
		int64toa(m_Value,buf);
		int len = strlen(buf);
		PrintString(s,TYPE_INT,len,buf);
	};

	bool  eINT::FromString(AnsiString& s,uint32& pos){
        int32 start = pos+2;

		//find and check for integer ;
		int32 slen = FindInt(s,start,'@');
		if(slen==0 || slen>2 )return false;     //slen always <=2 except string data

		//Get integer digits ( including possible negative sign )
		int32 t = (int32)StringToInt(&s[start],slen);

		//get data
		start += slen+1;
		bool correct;
		m_Value = RawStringToInt(&s[start],t,correct);
		if(!correct)return 0;
		pos = start+t;
		return true;
	};

	void eFLOAT::ToString(AnsiString& s){
		char buf[50];
		uint32 n = sprintf(buf,"%.6f",m_Value);
		while(buf[--n] == '0'); //Remove "0", to reduce the actual storage bytes
		buf[n+1]='\0';
		PrintString(s,TYPE_FLOAT,n+1,buf);
	};
	bool  eFLOAT::FromString(AnsiString& s,uint32& pos){
        int32 start = pos+2;

		//Find and check whether is integer,   @4@23.34@
		int32 slen = FindInt(s,start,'@');
		if(slen==0 || slen>2 )return false;     
		
		//Get double digits ( including possible negative sign )
		int64 t = StringToInt(&s[start],slen);

		//Check and find the float position
		uint32 floatpos =0; 
		start += slen +1;
		slen =  FindFloat(s, start,floatpos,'@');
        if(slen==0 || floatpos>20 || slen-floatpos>19)return 0;     

		m_Value = StringToFloat(&s[start],(uint32)t,floatpos);
		pos = start+t;
		return true;
	};	

	void eSTRING::ToString(AnsiString& s){ 
		AnsiString Value = WStoUTF8(m_Value);
		int len = Value.size();
		const char* pValue = Value.c_str();
        PrintString(s,TYPE_STRING,len,pValue);
	};

	bool  eSTRING::FromString(AnsiString& s,uint32& pos){   
		int32 start = pos+2;

		int32 slen = FindInt(s,start,'@');
		if(slen==0 || slen>20)return false;     

		//get the length of integer string
		int64 len = StringToInt(&s[start],slen);

		if (len==0)
		{
			m_Value== L"";
			pos+=4;
			return true;
		}

		//get string
		start += slen+1;
        m_Value = UTF8_To_UTF16(s.substr(start,len));

		pos = start+len;
		return true;
	};
/*
	bool  eSTRING::FromString(AnsiString& s,uint32& pos){   
		int32 start = pos+2;

		int32 slen = FindInt(s,start,'@');
		if(slen==0 || slen>20)return false;     

		//get the length of integer string
		int64 len = StringToInt(&s[start],slen);

		if (len==0)
		{
			m_Value== L"";
			pos+=4;
			return true;
		}
  
		//get integer
		start += slen+1;

		m_Value.resize((uint32)len);
		const  UTF8* Start = (const UTF8*)(s.c_str()+start);
		const  UTF8* End = Start + len;
#if defined(UNICODE_16_BIT)
		UTF16* DestStart = (UTF16*)(&m_Value[0]);
		UTF16* DestEnd = DestStart + len;

		ConversionResult ret = ConvertUTF8toUTF16(&Start,End, &DestStart, DestEnd, strictConversion);
#elif defined(UNICODE_32_BIT)
		UTF32* DestStart = (UTF32*)(&m_Value[0]);
		UTF32* DestEnd = DestStart + len;

		ConversionResult ret = ConvertUTF8toUTF32(&Start,End, &DestStart, DestEnd, strictConversion);
#else 
	    #error("I don't know how many bit of unicode char of your platform ")
#endif		
		if (ret != conversionOK)
		{
			return false;
		}

		tstring::size_type n = m_Value.find_first_of(L'\0');
		m_Value = m_Value.substr(0,n);

		pos = start+len;
		return true;
	};
*/
	void eBLOB::ToString(AnsiString& s){
		PrintString(s,TYPE_BLOB,m_Value.size(),m_Value.c_str());
	};

	bool  eBLOB::FromString(AnsiString& s, uint32& pos){
		int32 start = pos+2;
		
		int32 slen = FindInt(s,start,'@');
		if(slen==0 || slen>20){
			false;
		};     
		
		int64 len = StringToInt(&s[start],slen);
		
		if (len==0)
		{
			m_Value.resize(0);
			pos+=4;
			return true;
		}
		
		start += slen+1;
		m_Value.resize((uint32)len);
		memcpy(&m_Value[0],&s[start],(uint32)len);
		
		pos  = start+len;
		return true;

	};


}; //namespace ABSTRACT
