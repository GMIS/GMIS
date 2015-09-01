#include "AbstractSpace.h"
#include "BaseEnergy.h"
#include "ABTime.h"
#include "ABSpacePool.h"
#include "ConvertUTF.h"

namespace ABSTRACT{
AnsiString  WStoUTF8(const std::wstring& s){
	AnsiString Value;

	if (s.size()==0)
	{
		return Value;
	}

	int32 len = 3 * s.size() + 1;
	Value.resize(len);

	UTF8* DestStart = (UTF8*)(&Value[0]);
	UTF8* DestEnd = DestStart + len;

#ifdef WIN32
	const UTF16* Start = (const UTF16*)s.c_str();
	const UTF16* End = Start + s.size();
	ConversionResult ret = ConvertUTF16toUTF8(&Start,End,&DestStart,DestEnd, strictConversion);
#else
	const UTF32* Start = (const UTF32*)s.c_str();
	const UTF32* End = Start + s.size();

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

std::wstring UTF8toWS(const AnsiString& s){
	std::wstring Value;
	if (s.size()==0)
	{
		return Value;		
	}
	Value.resize(s.size());
	//UTF16* buf = new UTF16[s.size()+1];

	const  UTF8* Start = (const UTF8*)s.c_str();
	const  UTF8* End = Start + s.size();

#ifdef WIN32
	UTF16* DestStart = (UTF16*)(&Value[0]);
	UTF16* DestEnd = DestStart + s.size();

	ConversionResult ret = ConvertUTF8toUTF16(&Start,End, &DestStart, DestEnd, strictConversion);
#else
	UTF32* DestStart = (UTF32*)(&Value[0]);
	UTF32* DestEnd = DestStart + s.size();

	ConversionResult ret = ConvertUTF8toUTF32(&Start,End, &DestStart, DestEnd, strictConversion);
#endif		
	if (ret != conversionOK)
	{
		if(ret == sourceIllegal) //
		{
			int n = mbstowcs(NULL,s.c_str(),0);
			Value.resize(n);
			setlocale(LC_ALL,"");
			mbstowcs((wchar_t*)Value.c_str(),s.c_str(),n);
			return Value;
		}else{
			//throw std::exception("UFT8 Convert Fail."); 这种工具函数扔出例外似乎只会对宿主程序构成骚扰
			//改为给出提示+可能的乱码
			Value = _T("UFT8TO16 Convert Fail:");

			int n = mbstowcs(NULL,s.c_str(),0);
			tstring s1(n,0);
			setlocale(LC_ALL,"");
			mbstowcs((wchar_t*)s1.c_str(),s.c_str(),n);

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

tstring Format1024(const wchar_t* Format, ...){
		const int32 size = 1024;
	    wchar_t Buffer[size] = {0} ;
		va_list ArgList ;
		va_start (ArgList, Format) ;

#ifdef _WIN32
		_vsnwprintf(Buffer,size-1, Format, ArgList) ;
#else
		vswprintf(Buffer,1024,Format,ArgList);
#endif
		va_end (ArgList) ;

		return tstring(Buffer);
	};

	CABTime* AbstractSpace::m_StaticTimer = NULL;
	CABSpacePool*  AbstractSpace::m_StaticSpacePool = NULL;

	void AbstractSpace::InitTimer(CABTime* Timer){
		m_StaticTimer = Timer;
	}

	void AbstractSpace::InitSpacePool(CABSpacePool* Pool){
		m_StaticSpacePool = Pool;
	}

	int64 AbstractSpace::CreateTimeStamp(){
		assert(m_StaticTimer != NULL); 
		return m_StaticTimer->TimeStamp();
	};
	
	CABTime*  AbstractSpace::GetTimer(){
		assert(m_StaticTimer != NULL); 
		return m_StaticTimer;

	}

	CABSpacePool* AbstractSpace::GetSpacePool(){
		assert(m_StaticSpacePool);
		return m_StaticSpacePool;
	};
	

}//namespace ABSTRACT



