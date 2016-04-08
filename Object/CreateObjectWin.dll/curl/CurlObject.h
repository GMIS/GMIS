// WinAPIObject.h: interface for the WinAPIObject class.
// 封装windows操作系统的一些常用API
//////////////////////////////////////////////////////////////////////

#ifndef  _CURL_OBJECT_H__
#define  _CURL_OBJECT_H__

#pragma warning(disable: 4786)

#include "..\ObjectList.h"

#if defined _COMPILE_CURL_OBJECT



#include <curl/curl.h>

class CCurlObject : public Mass
{
public:
	static const TCHAR* UserManual;/* = __TEXT(\
		"\
		1 cmd：open\n\
		Function：打开一个程序或链接\n\
		Input：字符串，程序名或链接地址\n\
		Output：null\n\
		"\
		);
		*/

	tstring m_Name;
	struct curl_slist*  m_Headers;
	CURL*               m_Curl;
	char*               m_buf;
	int32               m_bufsize;
public:
	CCurlObject(tstring Name,int64 ID=0);
	virtual ~CCurlObject();
	virtual tstring GetName(){ return m_Name;};
	virtual TypeAB GetTypeAB(){ return 0x00000000;}

	bool Header(ePipeline* Pipe);
	bool Set(ePipeline* Pipe);
	bool Run(ePipeline* Pipe);
	bool CmdLine(ePipeline* pipe);
	bool Reset(ePipeline* Pipe);
	virtual bool Do(Energy* E);
	
	/*如果Object的执行依赖其内部状态信息则应该实现下面两个函数来实现初始化
      virtual uint32  ToString(string& s,uint32 pos =0) 
	  virtual uint32  FromString(string& s,uint32 pos=0) 
    */
};

#endif //_COMPILE_CURL_OBJECT

#endif // 