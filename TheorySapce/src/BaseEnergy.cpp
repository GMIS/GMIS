#pragma warning(disable:4786)

#include "AbstractSpacePool.h"
#include "BaseEnergy.h"
#include "ConvertUTF.h"

#include <stdio.h>
namespace ABSTRACT{
	
	//0@0@
	void eNULL::ToString(AnsiString& s){ 
		s += TYPE_TO_CHAR(TYPE_NULL);
		s.append("@0@");
	};
	
	//对于NULL类型，其格式为：type@0@ 没有len@data部分
	uint32  eNULL::FromString(AnsiString& s,uint32 pos){
		const char* ch = &s[pos+1];
		if( *ch == '@' && *(++ch) =='0' && *(++ch) =='@'){
			return 4;
		}
		return 0;
	};
	

	void eINT::ToString(AnsiString& s){
		char buf[30];
		_i64toa(m_Value,buf,10);
		int len = strlen(buf);
		PrintString(s,TYPE_INT,len,buf);
	};

	uint32  eINT::FromString(AnsiString& s, uint32 pos){
        int32 start = pos+2;

		//找到并检查是否为整数；
		int32 slen = FindInt(s,start,'@');
		if(slen==0 || slen>2 )return 0;     //除了字符串slen都不会>2

		//得到整数位数(包含可能的负号）
		int32 t = StringToInt(&s[start],slen);

		//得到数据
		start += slen+1;
		bool correct;
		m_Value = RawStringToInt(&s[start],t,correct);
		if(!correct)return 0;
		return  start+t-pos;
	};

	void eFLOAT::ToString(AnsiString& s){
		char buf[50];
		uint32 n = sprintf(buf,"%.6f",m_Value);
		while(buf[--n] == '0'); //去掉多余的0,减少实际存储所需字节
		buf[n+1]='\0';
		PrintString(s,TYPE_FLOAT,n+1,buf);
	};
	uint32  eFLOAT::FromString(AnsiString& s,uint32 pos){
        int32 start = pos+2;

		//找到并检查是否为整数；   @4@23.34@
		int32 slen = FindInt(s,start,'@');
		if(slen==0 || slen>2 )return 0;     //除了字符串slen都不会>2

		//得到double位数(包含可能的负号）
		int64 t = StringToInt(&s[start],slen);

		//检查并找到浮点数的位置
		uint32 floatpos =0; //浮点位置，基于数字长度包括可能的负号
		start += slen +1;
		slen =  FindFloat(s, start,floatpos,'@');
        if(slen==0 || floatpos>20 || slen-floatpos>19)return 0;     

		//得到数据
		m_Value = StringToFloat(&s[start],t,floatpos);
		return  start+t-pos;
	};	

	AnsiString  eSTRING::UTF16TO8(const std::wstring& s){
		AnsiString Value;

		if (s.size()==0)
		{
			return Value;
		}

		int32 len = 3 * s.size() + 1;
		Value.resize(len);

		const UTF16* Start = (const UTF16*)s.c_str();
		const UTF16* End = Start + s.size();
		UTF8* DestStart = (UTF8*)(&Value[0]);
		UTF8* DestEnd = DestStart + len;
#ifdef WIN32  		
		ConversionResult ret = ConvertUTF16toUTF8(&Start,End,&DestStart,DestEnd, strictConversion);
#else
		ConversionResult ret = ConvertUTF32toUTF8(&Start,End,&DestStart,DestEnd, strictConversion);
#endif		
		if (ret != conversionOK)
		{
			//throw std::exception("UFT8 Convert Fail.");这种工具函数扔出例外似乎只会对宿主程序构成骚扰
			//改为给出提示+可能的乱码
			
			Value = "UFT16TO8 Convert Fail:";
			return Value += (const char*)s.c_str();
		}
		return Value.c_str();
	}

    std::wstring eSTRING::UTF8TO16(const AnsiString& s){
		std::wstring Value;
		if (s.size()==0)
		{
			return Value;		
		}
		Value.resize(s.size());
		//UTF16* buf = new UTF16[s.size()+1];
		
		const  UTF8* Start = (const UTF8*)s.c_str();
		const  UTF8* End = Start + s.size();
		UTF16* DestStart = (UTF16*)(&Value[0]);
		UTF16* DestEnd = DestStart + s.size();
		//UTF16* DestStart = buf;
		//UTF16* DestEnd = DestStart + s.size();

#ifdef WIN32  		
		ConversionResult ret = ConvertUTF8toUTF16(&Start,End, &DestStart, DestEnd, strictConversion);
#else
		ConversionResult ret = ConvertUTF8toUTF32(&Start,End, &DestStart, DestEnd, strictConversion);
#endif		
		if (ret != conversionOK)
		{
			if(ret == sourceIllegal) //假设为多字节再尝试一次
			{
				int n = mbstowcs(NULL,s.c_str(),0);
				Value.resize(n);
				setlocale(LC_ALL,"");
				mbstowcs((TCHAR*)Value.c_str(),s.c_str(),n);
				return Value;
			}else{
				//throw std::exception("UFT8 Convert Fail."); 这种工具函数扔出例外似乎只会对宿主程序构成骚扰
				//改为给出提示+可能的乱码
			    Value = _T("UFT8TO16 Convert Fail:");
				
				int n = mbstowcs(NULL,s.c_str(),0);
				tstring s1(n,0);
				setlocale(LC_ALL,"");
				mbstowcs((TCHAR*)s1.c_str(),s.c_str(),n);

				Value.insert(Value.end(),s1.begin(),s1.end());
				return Value;
			}


		}
	/*	else{
			Value = (wchar_t*)buf;
			delete buf;		
		}
	*/
		tstring::size_type n = Value.find_first_of(_T('\0'));
		Value = Value.substr(0,n);

		return Value;
	}

	void eSTRING::ToString(AnsiString& s){ 
#ifdef _UNICODE
		AnsiString Value = UTF16TO8(m_Value);
		int len = Value.size();
		const char* pValue = Value.c_str();
        PrintString(s,TYPE_STRING,len,pValue);
#else
		int len = m_Value.size();
		const char* pValue = m_Value.c_str();
		PrintString(s,TYPE_STRING,len,pValue);
#endif	
	};

	uint32  eSTRING::FromString(AnsiString& s,uint32 pos){   
		int32 start = pos+2;

		//找到并检查是否为整数；
		int32 slen = FindInt(s,start,'@');
		if(slen==0 || slen>20)return 0;     

		//得到字符串长度
		int32 len = StringToInt(&s[start],slen);

		if (len==0)
		{
			m_Value==_T("");
			return 4;
		}

		//得到数据
		start += slen+1;
#ifdef _UNICODE
		m_Value.resize(len);
		const  UTF8* Start = (const UTF8*)(s.c_str()+start);
		const  UTF8* End = Start + len;
		UTF16* DestStart = (UTF16*)(&m_Value[0]);
		UTF16* DestEnd = DestStart + len;
#ifdef WIN32  
		ConversionResult ret = ConvertUTF8toUTF16(&Start,End, &DestStart, DestEnd, strictConversion);
#else   
		ConversionResult ret = ConvertUTF8toUTF32(&Start,End, &DestStart, DestEnd, strictConversion);
#endif		
		if (ret != conversionOK)
		{
			//throw std::exception("UFT8 Convert Fail.");
			return 0;
		}

		tstring::size_type n = m_Value.find_first_of(_T('\0'));
		m_Value = m_Value.substr(0,n);

#else
		m_Value.resize(len, '\0');
		memcpy(&m_Value[0],&s[start],len);
#endif
		return  start+len-pos;

	};

	void eBLOB::ToString(AnsiString& s){
		PrintString(s,TYPE_BLOB,m_Value.size(),m_Value.c_str());
	};

	uint32  eBLOB::FromString(AnsiString& s, uint32 pos){
		int32 start = pos+2;
		
		//找到并检查是否为整数；
		int32 slen = FindInt(s,start,'@');
		if(slen==0 || slen>20)return 0;     
		
		//得到字符串长度
		int32 len = StringToInt(&s[start],slen);
		
		if (len==0)
		{
			m_Value.resize(0);
			return 4;
		}
		
		//得到数据
		start += slen+1;
		m_Value.resize(len);
		memcpy(&m_Value[0],&s[start],len);
		
		return  start+len-pos;

	};


}; //namespace ABSTRACT