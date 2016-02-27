// curlobject.cpp: implementation of the WinAPIObject class.
//
//////////////////////////////////////////////////////////////////////

#include "curlobject.h"


#if defined _COMPILE_CURL_OBJECT

#include <assert.h>
#include <shellapi.h>
#include <algorithm>

using namespace ABSTRACT;

//CURL commdline 
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////



const TCHAR* CCurlObject::UserManual = __TEXT(\
		"\
		1 cmd：open\n\
		Function：打开一个程序或链接\n\
		Input：字符串，程序名或链接地址\n\
		Output：null\n\
		"\
		);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCurlObject::CCurlObject(tstring Name,int64 ID)
	:m_Name(Name),Mass(ID){
	curl_global_init(CURL_GLOBAL_ALL);
};

CCurlObject::~CCurlObject(){
	
};


bool CCurlObject::Get(ePipeline* Pipe){
	
	eElectron e;
	Pipe->Pop(&e);

	if (e.EnergyType() == TYPE_STRING)
	{
		tstring s = *(tstring*)e.Value();
		AnsiString url = WStoUTF8(s);

		CURL* Curl = curl_easy_init();
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Accept: Agent-007");
		if (Curl)
		{
			curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, headers);// 改协议头
			curl_easy_setopt(Curl, CURLOPT_URL,url.c_str());
				CURLcode code = curl_easy_perform(Curl);   // 执行
			if (code == CURLE_OK) {
				char* ct;
				code = curl_easy_getinfo(Curl, CURLINFO_CONTENT_TYPE, &ct);
			}
			curl_slist_free_all(headers);
			curl_easy_cleanup(Curl);
		}	
	}
	return true;
}
bool CCurlObject::CmdLine(ePipeline* pipe){
	return true;
}

bool CCurlObject::Do(Energy* E)
{
	ePipeline* Pipe = (ePipeline*)E;

	Pipe->SetLabel(_T(""));

	if (Pipe->Size()==0)
	{
		return true;
	}

	
    ENERGY_TYPE Type = Pipe->GetDataType(0);
	if (Type != TYPE_STRING)
	{
		return false;
	}
	tstring Cmd = Pipe->PopString();
    transform(Cmd.begin(),Cmd.end(),Cmd.begin(), ::tolower);

    if (Cmd == _T("get"))
    {
		return Get(Pipe);

    }else if (Cmd == _T("command"))
    {
		return CmdLine(Pipe);
	}
	Pipe->SetLabel(_T("Illegal command"));
//	Pipe->SetID(RETURN_ERROR);
	return false;
}
#endif //_COMPILE_WIMAPI_OBJECT